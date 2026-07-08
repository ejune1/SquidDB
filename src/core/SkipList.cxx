#include "core/SkipList.h"

#include "core/RowInfo.h"
#include "core/SkipListIterator.h"
#include "core/SkipListNode.h"
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
bool SkipList<K>::insert(const K key, std::byte* data, const std::uint16_t size, std::uint8_t nodeHeight) {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::insert not initialized");
	}

	if (nodeHeight > m_maxNodeHeight) {
		throw std::runtime_error("SkipList<K>::insert nodeHeight > maxNodeHeight");
	}

	std::string message = "SkipList<K>::insert key: " + std::to_string(key) + " size: " + std::to_string(size);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	if (nodeHeight == 0) {
		nodeHeight = SkipList<K>::generateNodeHeight(m_maxNodeHeight);
	}

	bool duplicateKey = false;
	TraverseContext<K>* traverseContext = traversePrevNodes(key, duplicateKey);

	if ((m_primaryIndex == true) && (duplicateKey == true)) {
		std::string message = "SkipList<K>::insert duplicate key (primary) key: " + std::to_string(key);
		m_logger.log(utils::Logger::LogLevel::Debug, message);

		delete traverseContext;
		return false;
	}

	SkipListNode<K>* insertNode = new SkipListNode<K>(key, nodeHeight);
	RowInfo* rowInfo = new RowInfo(RowInfo::Status::None, data, size);
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

	delete traverseContext;

	m_size.fetch_add(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::remove(const K key) {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::remove not initialized");
	}

	std::string message = "SkipList<K>::remove key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	bool duplicateKey = false;
	TraverseContext<K>* traverseContext = traversePrevNodes(key, duplicateKey);

	if (duplicateKey == false) {
		message = "SkipList<K>::remove key not found key: " + std::to_string(key);
		m_logger.log(utils::Logger::LogLevel::Debug, message);

		delete traverseContext;
		return false;
	}

	SkipListNode<K>* removeNode = nullptr;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		SkipListNode<K>* prev = traverseContext->getPrevNode(level);
		SkipListNode<K>* next = prev->getNext(level);

		assert((prev == m_head) || (key > prev->getKey()));
		assert((next == nullptr) || (key <= next->getKey()));

		if ((next != nullptr) && (key == next->getKey())) {
			assert((removeNode == nullptr) || (removeNode == next));

			removeNode = next;
			size_t prevWidth = (prev->getWidth(level) + removeNode->getWidth(level)) - 1;

			prev->setNext(level, next->getNext(level));
			prev->setWidth(level, prevWidth);

			next->setNext(level, nullptr);
		} else {
			//decrement width for all levels above remove node's height
			prev->setWidth(level, prev->getWidth(level) - 1);
		}
	}
	assert(removeNode != nullptr);

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

	delete removeNode;
	delete traverseContext;

	m_size.fetch_sub(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::update(const K key, std::byte* data, const std::uint16_t size) {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::update not initialized");
	}

	std::string message = "SkipList<K>::update key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	SkipListNode<K>* foundNode = findNode(key);

	if (foundNode != nullptr) {
		// TODO isolate
		std::byte* dataToDelete = foundNode->getRowInfo()->getData();
		foundNode->getRowInfo()->setData(data, size);

		if (dataToDelete != nullptr) {
			std::free(dataToDelete);
		}

		return true;
	}

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

	SkipListNode<K>* foundNode = findNode(key);

	if (foundNode != nullptr) {
		// TODO isolate
		std::byte* data = foundNode->getRowInfo()->getData();
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

	SkipListNode<K>* foundNode = findNode(key);
	
	return (foundNode != nullptr); 
}

template<typename K>
SkipListIterator<K> SkipList<K>::begin() const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::begin not initialized");
	}

	return SkipListIterator<K>(m_head->getNext(0 /* level */), std::nullopt);
}

template<typename K>
SkipListIterator<K> SkipList<K>::seek(const K key, const std::optional<K> endKey) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::seek not initialized");
	}

	SkipListNode<K>* trail = m_head;
	SkipListNode<K>* node = m_head;
	SkipListNode<K>* lowerBound = nullptr;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		node = node->getNext(level);

		while ((node != nullptr) && (key > node->getKey())) {
			trail = node;
			node = node->getNext(level);
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

		node = trail;
	}

	return SkipListIterator<K>(lowerBound, endKey);
}

template<typename K>
SkipListIterator<K> SkipList<K>::end() const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::end not initialized");
	}

	return SkipListIterator<K>(nullptr, std::nullopt);
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
		SkipListNode<K>* node = m_head->getNext(0);

		while (node != nullptr) {
			size++;
			node = node->getNext(0);
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

	bool duplicateKey = false;
	TraverseContext<K>* traverseContextLow = traversePrevNodes(lowKey, duplicateKey);
	TraverseContext<K>* traverseContextHigh = traversePrevNodes(highKey, duplicateKey);
	
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

		node = node->getNext(0 /* level */);
	}

	return totalBytes / (1024 * 1024);
}

template<typename K>
bool SkipList<K>::insertRow(const void* key, void* row, const std::uint16_t size) {
	const K insertKey = *static_cast<const K*>(key);
	std::byte* insertData = static_cast<std::byte*>(row);

	return insert(insertKey, insertData, size);
}

template<typename K>
bool SkipList<K>::deleteRow(const void* key) {
	const K deleteKey = *static_cast<const K*>(key);

	return remove(deleteKey);
}

template<typename K>
bool SkipList<K>::updateRow(const void* key, void* row, const std::uint16_t size) {
	const K updateKey = *static_cast<const K*>(key);
	std::byte* updateData = static_cast<std::byte*>(row);

	return update(updateKey, updateData, size);
}

template<typename K>
engine::TableIterator* SkipList<K>::scan() const {
	SkipListIterator<K>* tableIterator = new SkipListIterator<K>(begin());
	return tableIterator;
}

template<typename K>
engine::TableIterator* SkipList<K>::rangeScan(const void* startKey, const void* endKey) const {
	const K rangeStartKey = *static_cast<const K*>(startKey);
	const K rangeEndKey = *static_cast<const K*>(endKey);
	
	SkipListIterator<K>* tableIterator = new SkipListIterator<K>(seek(rangeStartKey, rangeEndKey));
	return tableIterator;
}

template<typename K>
SkipListNode<K>* SkipList<K>::findNode(const K key) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::findNode not initialized");
	}

	SkipListNode<K>* trail = m_head;
	SkipListNode<K>* node = m_head;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		node = node->getNext(level);

		while ((node != nullptr) && (key > node->getKey())) {
			trail = node;
			node = node->getNext(level);
		}
		assert(trail != nullptr);

		if ((node != nullptr) && (key == node->getKey())) {
			return node;
		}
		assert((node == nullptr) || (key < node->getKey()));

		node = trail;
	}

	return nullptr;
}

template<typename K>
TraverseContext<K>* SkipList<K>::traversePrevNodes(const K key, bool& duplicateKey) const {
	if (m_initialized == false) {
		throw std::runtime_error("SkipList<K>::traversePrevNodes not initialized");
	}

	TraverseContext<K>* traverseContext = new TraverseContext<K>(m_maxNodeHeight);

	SkipListNode<K>* trail = m_head;
	SkipListNode<K>* node = m_head;

	size_t accumulatedRank = 0;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		node = node->getNext(level);

		while ((node != nullptr) && (key > node->getKey())) {
			accumulatedRank += trail->getWidth(level);

			trail = node;
			node = node->getNext(level);
		}
		assert(trail != nullptr);

		if ((node != nullptr) && (key == node->getKey())) {
			duplicateKey = true;
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
