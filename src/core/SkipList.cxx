#include "core/SkipList.h"

#include "core/LockType.h"
#include "core/RowInfo.h"
#include "core/SkipListIterator.h"
#include "core/SkipListNode.h"
#include "core/Transaction.h"
#include "engine/TableIterator.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>

namespace squiddb { namespace core {

template<typename K>
SkipList<K>::SkipList(utils::Logger& logger, const bool primaryIndex, const std::uint8_t maxNodeHeight)
	: m_logger(logger), m_primaryIndex(primaryIndex), m_maxNodeHeight(maxNodeHeight) {
	m_head = nullptr;
	m_initialized = false;
	m_size = 0;
	m_sequenceLock = 0;
}

template<typename K>
SkipList<K>::~SkipList() {
	// SKipList owns all nodes, keys, row infos, values
	SkipListNode<K>* node = m_head;

	while (node != nullptr) {
		SkipListNode<K>* nodeToDelete = node;
		node = node->getNext(0);

		RowInfo* rowInfo = nodeToDelete->getRowInfo();
		nodeToDelete->setRowInfo(nullptr);

		while (rowInfo != nullptr) {
			RowInfo* rowInfoToDelete = rowInfo;
			rowInfo = rowInfo->getNext();

			std::byte* dataToDelete = rowInfoToDelete->getData();
			rowInfoToDelete->setData(nullptr, 0 /* size */);

			if (dataToDelete != nullptr) {
				std::free(dataToDelete);
			}

			delete rowInfoToDelete;
		}

		delete nodeToDelete;	
	}

	m_head = nullptr;
	m_initialized = false;
}

template<typename K>
void SkipList<K>::initialize() {
	if (m_initialized == true) {
		throw std::runtime_error("SkipList<K>::initialize already initialized");
	}

	if (m_maxNodeHeight == 0) {
		throw std::runtime_error("SkipList<K>::initialize maxNodeHeight 0");
	}

	std::string message = "SkipList<K>::initialize with max node height: " + std::to_string(m_maxNodeHeight);
	m_logger.log(utils::Logger::LogLevel::Info, message);

	// head is dummy object
	m_head = new SkipListNode<K>(K{}, m_maxNodeHeight);

	m_initialized = true;
}

template<typename K>
bool SkipList<K>::insert(const K key, std::byte* data, const std::uint16_t size, std::uint8_t nodeHeight, Transaction* transaction) {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::insert not initialized");
	}

	if (nodeHeight > m_maxNodeHeight) {
		throw std::runtime_error("SkipList<K>::insert nodeHeight > maxNodeHeight");
	}

	std::string message = "SkipList<K>::insert key: " + std::to_string(key) + " size: " + std::to_string(size);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	RETRY:
		std::size_t sequenceLock = m_sequenceLock.load(std::memory_order_acquire);
		if (sequenceLock % 2 != 0) {
			goto RETRY;
		}

		bool duplicateKey = false;
		TraverseContext<K>* traverseContext = traversePrevNodes(key, duplicateKey, transaction, LockType::Write);

		if (m_sequenceLock.compare_exchange_weak(sequenceLock, sequenceLock + 1, std::memory_order_release, std::memory_order_relaxed) == false) {
			traverseContext->unlockAll();
			delete traverseContext;
			goto RETRY;
		}

	if ((m_primaryIndex == true) && (duplicateKey == true)) {
		std::string message = "SkipList<K>::insert duplicate key (primary) key: " + std::to_string(key);
		m_logger.log(utils::Logger::LogLevel::Debug, message);

		traverseContext->unlockAll();
		m_sequenceLock.fetch_add(1, std::memory_order_release);

		delete traverseContext;
		return false;
	}

	RowInfo* rowInfo = nullptr;
	SkipListNode<K>* insertNode = nullptr;
	SkipListNode<K>* nextLevel0 = traverseContext->getPrevNode(0 /* level */)->getNext(0 /* level */);

	// key exists but has been deleted - tack on new info
	if ((nextLevel0 != nullptr) && (key == nextLevel0->getKey())) {
		assert(transaction != nullptr);
		insertNode = nextLevel0;
		insertNode->writeLock();

		rowInfo = transaction->isolateRowInfo(insertNode->getRowInfo());
		assert((rowInfo != nullptr) && (rowInfo->getDeleting() == true));

		RowInfo* newRowInfo = new RowInfo(RowInfo::Status::Uncommitted, data, size);
		newRowInfo->setTransactionId(transaction->getTransactionId());
		newRowInfo->setCreating(true);
		newRowInfo->setDirty(true);

		newRowInfo->setNext(rowInfo->getNext());
		rowInfo->setNext(newRowInfo);

		rowInfo = newRowInfo;

	} else {
		if (nodeHeight == 0) {
			nodeHeight = SkipList<K>::generateNodeHeight(m_maxNodeHeight);
		}

		insertNode = new SkipListNode<K>(key, nodeHeight);
		insertNode->writeLock();

		rowInfo = new RowInfo(RowInfo::Status::Committed, data, size);

		if (transaction != nullptr) {
			rowInfo->setStatus(RowInfo::Status::Uncommitted);
			rowInfo->setTransactionId(transaction->getTransactionId());
			rowInfo->setCreating(true);
			rowInfo->setDirty(true);
		}
		insertNode->setRowInfo(rowInfo);

		// increment width for all levels above the height of this node
		for (uint8_t level = m_maxNodeHeight - 1; level >= nodeHeight; level--) {
			SkipListNode<K>* prev = traverseContext->getPrevNode(level);
			assert(prev != nullptr);

			prev->setWidth(level, prev->getWidth(level) + 1);
		}

		std::uint8_t level = nodeHeight;
		while (level-- > 0) {
			SkipListNode<K>* prev = traverseContext->getPrevNode(level);
			SkipListNode<K>* next = prev->getNext(level);

			// I dont want to lock next here just for the asserts
			assert((prev == m_head) || (key > prev->getKey()));
			assert((next == nullptr) || (key <= next->getKey()));

			size_t prevNodeRank = traverseContext->getPrevRank(level);
			size_t insertNodeRank = traverseContext->getRank();
			size_t distance = insertNodeRank - prevNodeRank;

			size_t prevWidth = distance + 1;
			size_t insertWidth = (prev->getWidth(level) == 0) ? 0 : (prev->getWidth(level) - distance);

			prev->setNext(level, insertNode);
			prev->setWidth(level, prevWidth);

			insertNode->setNext(level, next);
			insertNode->setWidth(level, insertWidth);
		}
	}

	insertNode->writeUnlock();
	traverseContext->unlockAll();

	m_sequenceLock.fetch_add(1, std::memory_order_release);

	delete traverseContext;

	if (transaction != nullptr) {
		// TODO opaque keys
		transaction->addAffectedRow(reinterpret_cast<const std::byte*>(insertNode->getKeyRef()), sizeof(K), rowInfo);
	}

	m_size.fetch_add(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::remove(const K key, Transaction* transaction, bool background) {
	// TODO transaction needs background delete
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::remove not initialized");
	}

	std::string message = "SkipList<K>::remove key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	RETRY:
		std::size_t sequenceLock = m_sequenceLock.load(std::memory_order_acquire);
		if (sequenceLock % 2 != 0) {
			goto RETRY;
		}

		bool duplicateKey = false;
		TraverseContext<K>* traverseContext = traversePrevNodes(key, duplicateKey, transaction, LockType::Write);

		if (m_sequenceLock.compare_exchange_weak(sequenceLock, sequenceLock + 1, std::memory_order_release, std::memory_order_relaxed) == false) {
			traverseContext->unlockAll();
			delete traverseContext;
			goto RETRY;
		}

	if (duplicateKey == false) {
		message = "SkipList<K>::remove key not found key: " + std::to_string(key);
		m_logger.log(utils::Logger::LogLevel::Debug, message);

		traverseContext->unlockAll();
		m_sequenceLock.fetch_add(1, std::memory_order_release);

		delete traverseContext;
		return false;
	}

	// TODO next (the remove node) needs to be locked during the traverse (before the retry)
	if (background == true) {
		if (transaction == nullptr) {
			throw std::runtime_error("SkipList<K>::remove background true but transaction is nullptr");
		}

		SkipListNode<K>* nextLevel0 = traverseContext->getPrevNode(0 /* level */)->getNext(0 /* level */);
		nextLevel0->writeLock();

		assert(nextLevel0 != nullptr);
		assert(key == nextLevel0->getKey());

		RowInfo* rowInfo = transaction->isolateRowInfo(nextLevel0->getRowInfo());
		assert(rowInfo->getDeleting() == false);

		RowInfo* newRowInfo = new RowInfo(RowInfo::Status::Uncommitted);
		newRowInfo->setTransactionId(transaction->getTransactionId());
		newRowInfo->setDeleting(true);
		newRowInfo->setDirty(true);

		newRowInfo->setNext(rowInfo->getNext());
		rowInfo->setNext(newRowInfo);

		transaction->addAffectedRow(reinterpret_cast<const std::byte*>(nextLevel0->getKeyRef()), sizeof(K), newRowInfo);
		nextLevel0->writeUnlock();

	} else {
		SkipListNode<K>* removeNode = nullptr;

		std::uint8_t level = m_maxNodeHeight;
		while (level-- > 0) {
			SkipListNode<K>* prev = traverseContext->getPrevNode(level);
			SkipListNode<K>* next = prev->getNext(level);
			if (next != nullptr) {
				next->writeLock();
			}

			assert((prev == m_head) || (key > prev->getKey()));
			assert((next == nullptr) || (key <= next->getKey()));

			if ((next != nullptr) && (key == next->getKey())) {
				assert((removeNode == nullptr) || (removeNode == next));
				removeNode = next;

				size_t prevWidth = (prev->getWidth(level) + removeNode->getWidth(level)) - 1;

				prev->setNext(level, next->getNext(level));
				prev->setWidth(level, prevWidth);

				next->setNext(level, nullptr);
				removeNode->writeUnlock();
			} else {
				//decrement width for all levels above remove node's height
				prev->setWidth(level, prev->getWidth(level) - 1);

				if (next != nullptr) {
					next->writeUnlock();
				}
			}
		}
		assert(removeNode != nullptr);
		removeNode->writeLock();

		RowInfo* rowInfo = removeNode->getRowInfo();
		removeNode->setRowInfo(nullptr);

		while (rowInfo != nullptr) {
			RowInfo* rowInfoToDelete = rowInfo;
			rowInfo = rowInfo->getNext();

			std::byte* dataToDelete = rowInfoToDelete->getData();
			rowInfoToDelete->setData(nullptr, 0 /* size */);

			if (dataToDelete != nullptr) {
				std::free(dataToDelete);
			}
			delete rowInfoToDelete;
		}

		removeNode->writeUnlock();
		delete removeNode;
	}

	traverseContext->unlockAll();
	m_sequenceLock.fetch_add(1, std::memory_order_release);

	delete traverseContext;

	m_size.fetch_sub(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::update(const K key, std::byte* data, const std::uint16_t size, Transaction* transaction) {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::update not initialized");
	}

	std::string message = "SkipList<K>::update key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	RETRY:
		std::size_t sequenceLock = m_sequenceLock.load(std::memory_order_acquire);
		if (sequenceLock % 2 != 0) {
			goto RETRY;
		}

		SkipListNode<K>* foundNode = findNode(key);

		if (foundNode != nullptr) {
			foundNode->writeLock();
		}

		if (m_sequenceLock.compare_exchange_weak(sequenceLock, sequenceLock + 1, std::memory_order_release, std::memory_order_relaxed) == false) {
			if (foundNode != nullptr) {
				foundNode->writeUnlock();
			}
			goto RETRY;
		}

	if (foundNode != nullptr) {
		if (transaction != nullptr) {
			RowInfo* rowInfo = transaction->isolateRowInfo(foundNode->getRowInfo());

			RowInfo* newRowInfo = new RowInfo(RowInfo::Status::Uncommitted, data, size);
			newRowInfo->setTransactionId(transaction->getTransactionId());
			newRowInfo->setUpdating(true);
			newRowInfo->setDirty(true);

			newRowInfo->setNext(rowInfo->getNext());
			rowInfo->setNext(newRowInfo);
			
			transaction->addAffectedRow(reinterpret_cast<const std::byte*>(foundNode->getKeyRef()), sizeof(K), newRowInfo);

		} else {
			std::byte* dataToDelete = foundNode->getRowInfo()->getData();
			foundNode->getRowInfo()->setData(data, size);

			if (dataToDelete != nullptr) {
				std::free(dataToDelete);
			}
		}

		foundNode->writeUnlock();
		m_sequenceLock.fetch_add(1, std::memory_order_release);

		return true;
	}

	m_sequenceLock.fetch_add(1, std::memory_order_release);

	message = "SkipList<K>::update key not found key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Debug, message);

	// TODO who frees the passed in data that when key not found

	return false;
}

template<typename K>
std::byte* SkipList<K>::find(const K key) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::find not initialized");
	}

	std::string message = "SkipList<K>::find key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	RETRY:
		std::size_t sequenceLock = m_sequenceLock.load(std::memory_order_acquire);
		if (sequenceLock % 2 != 0) {
			goto RETRY;
		}

		SkipListNode<K>* foundNode = findNode(key);

		if (foundNode != nullptr) {
			foundNode->readLock();
		}

		if (sequenceLock != m_sequenceLock.load(std::memory_order_acquire)) {
			if (foundNode != nullptr) {
				foundNode->readUnlock();
			}
			goto RETRY;
		}

	if (foundNode != nullptr) {
		// TODO isolate
		std::byte* data = foundNode->getRowInfo()->getData();
	
		foundNode->readUnlock();

		return data;
	}

	return nullptr;
}

template<typename K>
bool SkipList<K>::contains(const K key) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::contains not initialized");
	}

	std::string message = "SkipList<K>::contains key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	RETRY:
		std::size_t sequenceLock = m_sequenceLock.load(std::memory_order_acquire);
		if (sequenceLock % 2 != 0) {
			goto RETRY;
		}

		// TODO isolate
		SkipListNode<K>* foundNode = findNode(key);

		if (sequenceLock != m_sequenceLock.load(std::memory_order_acquire)) {
			goto RETRY;
		}
	
	return (foundNode != nullptr); 
}

template<typename K>
SkipListIterator<K> SkipList<K>::begin(Transaction* transaction) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::begin not initialized");
	}

	std::size_t sequence = m_sequenceLock.load(std::memory_order_acquire);

	m_head->readLock();
	SkipListNode<K>* next = m_head->getNext(0 /* level */);
	m_head->readUnlock();

	return SkipListIterator<K>(this, next, std::nullopt, transaction, sequence);
}

template<typename K>
SkipListIterator<K> SkipList<K>::seek(const K key, const std::optional<K> endKey, Transaction* transaction) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::seek not initialized");
	}

	std::size_t sequence = m_sequenceLock.load(std::memory_order_acquire);

	SkipListNode<K>* node = lowerBound(key);

	return SkipListIterator<K>(this, node, endKey, transaction, sequence);
}

template<typename K>
SkipListIterator<K> SkipList<K>::end() const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::end not initialized");
	}

	return SkipListIterator<K>(nullptr, nullptr, std::nullopt, nullptr /* transaction */, 0 /* sequence */);
}

template<typename K>
bool SkipList<K>::empty() const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::empty not initialized");
	}

	return (m_size.load(std::memory_order_relaxed) == 0);
}

template<typename K>
size_t SkipList<K>::size(const bool calculate) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::size not initialized");
	}

	if (calculate == true) {
		size_t size = 0;

		m_head->readLock();
		SkipListNode<K>* node = m_head->getNext(0);
		m_head->readUnlock();

		while (node != nullptr) {
			size++;

			node->readLock();
			SkipListNode<K>* next = node->getNext(0);
			node->readUnlock();

			node = next;
		}

		return size;
	}

	return m_size.load(std::memory_order_relaxed);
}

template<typename K>
size_t SkipList<K>::estimateRangeCardinality(const K lowKey, const K highKey) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::estimateRangeCardinality not initialized");
	}

	if (lowKey > highKey) {
		std::string message = "SkipList<K>::estimateRangeCardinality lowKey > highKey";
		m_logger.log(utils::Logger::LogLevel::Warn, message);
		return 0;
	}

	// TODO transactions
	bool duplicateKey = false;
	TraverseContext<K>* traverseContextLow = traversePrevNodes(lowKey, duplicateKey);
	traverseContextLow->unlockAll();

	TraverseContext<K>* traverseContextHigh = traversePrevNodes(highKey, duplicateKey);
	traverseContextHigh->unlockAll();
	
	size_t lowRank = traverseContextLow->getPrevRank(0 /* level index */);
	size_t highRank = traverseContextHigh->getPrevRank(0 /* level index */);
	assert(lowRank <= highRank);

	delete traverseContextLow;
	delete traverseContextHigh;

	return highRank - lowRank;
}

template<typename K>
size_t SkipList<K>::memoryUsageMB() const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::memoryUsageMB not initialized");
	}

	size_t totalBytes = 0;

	SkipListNode<K>* node = m_head;

	while (node != nullptr) {
		node->readLock();

		std::uint8_t nodeHeight = node->getHeight();
		// the node itself
		totalBytes += sizeof(SkipListNode<K>);
		// width array
		totalBytes += nodeHeight * sizeof(size_t);
		// next pointer array
		totalBytes += nodeHeight * sizeof(SkipListNode<K>*);

		RowInfo* rowInfo = node->getRowInfo();
		while (rowInfo != nullptr) {
			totalBytes += sizeof(RowInfo);

			if (rowInfo->getData() != nullptr) {
				totalBytes += rowInfo->getSize();
			}

			rowInfo = rowInfo->getNext();
		}

		SkipListNode<K>* next = node->getNext(0 /* level */);
		node->readUnlock();
		node = next;
	}

	return totalBytes / (1024 * 1024);
}

template<typename K>
bool SkipList<K>::insertRow(const void* key, void* row, const std::uint16_t size, Transaction* transaction) {
	const K insertKey = *static_cast<const K*>(key);
	std::byte* insertData = static_cast<std::byte*>(row);

	return insert(insertKey, insertData, size, 0 /* nodeHeight */, transaction);
}

template<typename K>
bool SkipList<K>::deleteRow(const void* key, Transaction* transaction) {
	const K deleteKey = *static_cast<const K*>(key);

	return remove(deleteKey, transaction, true /* background */);
}

template<typename K>
bool SkipList<K>::updateRow(const void* key, void* row, const std::uint16_t size, Transaction* transaction) {
	const K updateKey = *static_cast<const K*>(key);
	std::byte* updateData = static_cast<std::byte*>(row);

	return update(updateKey, updateData, size, transaction);
}

template<typename K>
engine::TableIterator* SkipList<K>::scan(Transaction* transaction) const {
	std::size_t sequence = m_sequenceLock.load(std::memory_order_acquire);

	m_head->readLock();
	SkipListNode<K>* next = m_head->getNext(0 /* level */);
	m_head->readUnlock();

	SkipListIterator<K>* tableIterator = new SkipListIterator<K>(this, next, std::nullopt, transaction, sequence);
	return tableIterator;
}

template<typename K>
engine::TableIterator* SkipList<K>::rangeScan(const void* startKey, const void* endKey, Transaction* transaction) const {
	const K rangeStartKey = *static_cast<const K*>(startKey);
	const K rangeEndKey = *static_cast<const K*>(endKey);
	
	SkipListIterator<K> skipListIterator = seek(rangeStartKey, rangeEndKey, transaction);
	SkipListIterator<K>* tableIterator = new SkipListIterator<K>(skipListIterator);
	return tableIterator;
}

template<typename K>
SkipListNode<K>* SkipList<K>::lowerBound(const K key) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::lowerBound not initialized");
	}

	SkipListNode<K>* trail = m_head;
	trail->readLock();

	SkipListNode<K>* node = m_head;
	SkipListNode<K>* lowerBound = nullptr;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		node = node->getNext(level);
		if (node != nullptr) {
			node->readLock();
		}

		while ((node != nullptr) && (key > node->getKey())) {
			trail->readUnlock();
			trail = node;

			node = node->getNext(level);
			if (node != nullptr) {
				node->readLock();
			}
		}
		assert(trail != nullptr);

		if ((node != nullptr) && (key == node->getKey())) {
			lowerBound = node;
			break;
		}
		assert((node == nullptr) || (key < node->getKey()));

		if (level == 0) {
			lowerBound = node;
			break;
		}

		if (node != nullptr) {
			node->readUnlock();
		}
		node = trail;
	}

	trail->readUnlock();
	if (node != nullptr) {
		node->readUnlock();
	}

	return lowerBound;
}

template<typename K>
SkipListNode<K>* SkipList<K>::findNode(const K key) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::findNode not initialized");
	}

	SkipListNode<K>* trail = m_head;
	trail->readLock();

	SkipListNode<K>* node = m_head;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		node = node->getNext(level);
		if (node != nullptr) {
			node->readLock();
		}

		while ((node != nullptr) && (key > node->getKey())) {
			trail->readUnlock();			
			trail = node;

			node = node->getNext(level);
			if (node != nullptr) {
				node->readLock();
			}
		}
		assert(trail != nullptr);

		if ((node != nullptr) && (key == node->getKey())) {
			trail->readUnlock();
			node->readUnlock();
			return node;
		}
		assert((node == nullptr) || (key < node->getKey()));

		if (node != nullptr) {
			node->readUnlock();
		}
		node = trail;
	}

	trail->readUnlock();
	return nullptr;
}

template<typename K>
TraverseContext<K>* SkipList<K>::traversePrevNodes(const K key, bool& duplicateKey, Transaction* transaction, LockType lockType) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::traversePrevNodes not initialized");
	}

	TraverseContext<K>* traverseContext = new TraverseContext<K>(m_maxNodeHeight, lockType);

	SkipListNode<K>* trail = m_head;
	trail->readLock();

	SkipListNode<K>* node = m_head;

	size_t accumulatedRank = 0;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		node = node->getNext(level);

		if ((node != nullptr) && (traverseContext->containsNode(node) == false)) {
			node->readLock();
		}

		while ((node != nullptr) && (key > node->getKey())) {
			accumulatedRank += trail->getWidth(level);

			if (traverseContext->containsNode(trail) == false) {
				trail->readUnlock();
			}

			trail = node;
			node = node->getNext(level);

			if ((node != nullptr) && (traverseContext->containsNode(node) == false)) {
				node->readLock();
			}
		}
		assert(trail != nullptr);

		if ((node != nullptr) && (key == node->getKey())) {
			if (transaction == nullptr) {
				duplicateKey = true;
			} else {
				RowInfo* rowInfo = transaction->isolateRowInfo(node->getRowInfo());
				if ((rowInfo != nullptr) && (rowInfo->getDeleting() == false)) {
					duplicateKey = true;
				}
			}
		}

		assert(trail != node);
		if (traverseContext->containsNode(trail) == false) {
			trail->readUnlock();
		}
		if ((node != nullptr) && (traverseContext->containsNode(node) == false)) {
			node->readUnlock();
		}

		traverseContext->setPrevNode(level, trail);
		traverseContext->setPrevRank(level, accumulatedRank);

		node = trail;
	}

	traverseContext->setRank(accumulatedRank);
	return traverseContext;
}

template<typename K>
bool SkipList<K>::validate() const {
	// first, validate widths
	std::uint8_t level = m_maxNodeHeight;
	bool valid = true;

	while (level-- > 0) {
		SkipListNode<K>* trail = m_head;
		SkipListNode<K>* node = m_head->getNext(level);
		SkipListNode<K>* level0 = m_head;

		if (level >= 1) {
			while (true) {
				size_t width = 0;

				while (level0 != node) {
					level0 = level0->getNext(0 /* level */);
					if (level0 != nullptr) {
						width++;
					}
				}
			
				valid = (valid == false) ? false : (trail->getWidth(level) == width);
				assert(trail->getWidth(level) == width);

				if (node == nullptr) {
					break;
				}

				trail = node;
				node = node->getNext(level);
			}
		} else if (level == 0) {
			level0 = m_head;

			while (level0 != nullptr) {
				if (level0->getNext(0 /* level */) == nullptr) {
					valid = (valid == false) ? false : (level0->getWidth(0 /* level */) == 0);
					assert(level0->getWidth(0 /* level */) == 0);	
				} else {
					valid = (valid == false) ? false : (level0->getWidth(0 /* level */) == 1);
					assert(level0->getWidth(0 /* level */) == 1);	
				}

				level0 = level0->getNext(0 /* level */);
			}
		}
	}
	
	// check that all keys are in order
	SkipListNode<K>* node = m_head->getNext(0 /* level */);
	while (node != nullptr) {
		K key = node->getKey();

		std::uint8_t level = node->getHeight();
		while (level-- > 0) {
			SkipListNode<K>* next = node->getNext(level);
			if (next != nullptr) {
				valid = (valid == false) ? false : (key <= next->getKey());	
				assert(key <= next->getKey());
			}
		}

		node = node->getNext(0 /* level */);
	}

	return valid;
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipList<std::int32_t>;

}} // namespace
