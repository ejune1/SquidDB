#include "core/SkipList.h"

#include "core/SkipListNode.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace squiddb { namespace core {

template<typename K>
SkipList<K>::SkipList(const utils::Configuration& configuration, utils::Logger& logger, const bool primaryIndex)
	: m_configuration(configuration), m_logger(logger), m_primaryIndex(primaryIndex) {
	m_maxNodeHeight = 0;
	m_head = nullptr;
	m_size = 0;
}

template<typename K>
SkipList<K>::~SkipList() {
	// SKipList owns all nodes, keys, values
	SkipListNode<K>* node = m_head;

	while (node != nullptr) {
		SkipListNode<K>* nodeToDelete = node;
		node = node->getNext(0);

		void* dataToDelete = nodeToDelete->getData();
		delete nodeToDelete;
		
		if (dataToDelete != nullptr) {
			std::free(dataToDelete);
		}
	}

	m_head = nullptr;
}

template<typename K>
void SkipList<K>::initialize() {
	m_maxNodeHeight = m_configuration.getMaxNodeHeight();

	std::string message = "SkipList<K>::initialize with max node height: " + std::to_string(m_maxNodeHeight);
	m_logger.log(utils::Logger::LogLevel::Info, message);

	// head is dummy object
	m_head = new SkipListNode<K>(K{}, nullptr, 0, m_maxNodeHeight);
}

template<typename K>
bool SkipList<K>::insert(const K key, void* data, const std::uint16_t size, std::uint8_t nodeHeight) {
	std::string message = "SkipL:ist<K>::insert key: " + std::to_string(key) + " size: " + std::to_string(size);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	if (nodeHeight == 0) {
		nodeHeight = SkipList<K>::generateNodeHeight(m_maxNodeHeight);
	}
	SkipListNode<K>* insertNode = new SkipListNode<K>(key, data, size, nodeHeight);

	bool duplicateKey = false;
	SkipListNode<K>** prevNodes = traversePrevNodes(key, duplicateKey);

	if ((m_primaryIndex == true) && (duplicateKey == true)) {
		std::string message = "SkipL:ist<K>::insert duplicate key (primary) key: " + std::to_string(key);
		m_logger.log(utils::Logger::LogLevel::Debug, message);

		delete [] prevNodes;
		return false;
	}

	std::uint8_t level = nodeHeight;
	while (level-- > 0) {
		SkipListNode<K>* prev = prevNodes[level];
		SkipListNode<K>* next = prev->getNext(level);

		assert((prev == m_head) || (key > prev->getKey()));
		assert((next == nullptr) || (key <= next->getKey()));

		prev->setNext(level, insertNode);
		insertNode->setNext(level, next);
	}

	delete [] prevNodes;

	m_size.fetch_add(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::remove(const K key) {
	std::string message = "SkipL:ist<K>::remove key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	bool duplicateKey = false;
	SkipListNode<K>** prevNodes = traversePrevNodes(key, duplicateKey);

	if (duplicateKey == false) {
		message = "SkipList<K>::remove key not found key: " + std::to_string(key);
		m_logger.log(utils::Logger::LogLevel::Debug, message);

		delete [] prevNodes;
		return false;
	}

	SkipListNode<K>* removeNode = nullptr;

	std::uint8_t level = m_maxNodeHeight;
	while (level-- > 0) {
		SkipListNode<K>* prev = prevNodes[level];
		SkipListNode<K>* next = prev->getNext(level);

		assert((prev == m_head) || (key > prev->getKey()));
		assert((next == nullptr) || (key <= next->getKey()));

		if ((next != nullptr) && (key == next->getKey())) {
			assert((removeNode == nullptr) || (removeNode == next));

			removeNode = next;
			prev->setNext(level, next->getNext(level));
			next->setNext(level, nullptr);
		}
	}
	assert(removeNode != nullptr);

	void* dataToDelete = removeNode->getData();
	delete removeNode;

	if (dataToDelete != nullptr) {
		std::free(dataToDelete);
	}

	delete [] prevNodes;

	m_size.fetch_sub(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::update(const K key, void* data, const std::uint16_t size) {
	std::string message = "SkipL:ist<K>::update key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	SkipListNode<K>* foundNode = findNode(key);

	if (foundNode != nullptr) {
		void* dataToDelete = foundNode->getData();
		foundNode->setData(data, size);

		if (dataToDelete != nullptr) {
			std::free(dataToDelete);
		}

		return true;
	}

	message = "SkipL:ist<K>::update key not found key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Debug, message);

	// TODO who frees the passed in data that when key not found

	return false;
}

template<typename K>
void* SkipList<K>::find(const K key) const {
	std::string message = "SkipL:ist<K>::find key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	SkipListNode<K>* foundNode = findNode(key);

	if (foundNode != nullptr) {
		void* data = foundNode->getData();
		return data;
	}

	return nullptr;
}

template<typename K>
bool SkipList<K>::contains(const K key) const {
	std::string message = "SkipL:ist<K>::contains key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	SkipListNode<K>* foundNode = findNode(key);
	
	return (foundNode != nullptr); 
}

template<typename K>
size_t SkipList<K>::size(const bool calculate) const {
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
	// TODO implement pointer widths
	return 0;
}

template<typename K>
size_t SkipList<K>::memoryUsageMB() const {
	// TODO
	return 0;
}

template<typename K>
SkipListNode<K>* SkipList<K>::findNode(const K key) const {
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
SkipListNode<K>** SkipList<K>::traversePrevNodes(const K key, bool& duplicateKey) const {
	SkipListNode<K>** prevNodes = new SkipListNode<K>*[m_maxNodeHeight];

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
			duplicateKey = true;
		}

		prevNodes[level] = trail;
		node = trail;
	}

	return prevNodes;
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipList<int>;

}} // namespace
