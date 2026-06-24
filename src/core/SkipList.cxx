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
bool SkipList<K>::insert(const K key, void* data, const std::uint16_t size) {
	std::uint8_t nodeHeight = SkipList<K>::generateNodeHeight(m_maxNodeHeight);

	std::string message = "SkipL:ist<K>::insert key: " + std::to_string(key) + " size: " + std::to_string(size);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	for (std::uint8_t level = 0; level < nodeHeight; level++) {
		if (m_head->getNext(level) == nullptr) {
			SkipListNode<K>* insertNode = new SkipListNode<K>(key, data, size, nodeHeight);
			m_head->setNext(level, insertNode);
			continue;
		}

		SkipListNode<K>* trail = m_head;
		SkipListNode<K>* node = m_head->getNext(level);

		while ((node != nullptr) && (key > node->getKey())) {
			trail = node;
			node = node->getNext(level);
		}

		// duplicate key - primary only - will be found at level 0
		if ((m_primaryIndex == true) && (node != nullptr) && (key == node->getKey())) {
			std::string message = "SkipL:ist<K>::insert duplicate key (primary) key: " + std::to_string(key);
			m_logger.log(utils::Logger::LogLevel::Debug, message);

			return false;
		}

		SkipListNode<K>* insertNode = new SkipListNode<K>(key, data, size, nodeHeight);

		trail->setNext(level, insertNode);
		insertNode->setNext(level, node);
	}
	
	m_size.fetch_add(1, std::memory_order_relaxed);
	return true;
}

template<typename K>
bool SkipList<K>::remove(const K key) {
	std::string message = "SkipL:ist<K>::remove key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	SkipListNode<K>* foundNode = findNode(key);

	if (foundNode != nullptr) {
		std::uint8_t nodeHeight = foundNode->getHeight();

		for (uint8_t level = nodeHeight - 1; level != 0; level++) {
			SkipListNode<K>* node = m_head;
			
			while (key != node->getNext(level)->getKey()) {
				node = node->getNext(level);
			}

			assert(node == foundNode);
			node->setNext(level, node->getNext(level)->getNext(level));
		}

		void* dataToDelete = foundNode->getData();
		delete foundNode;
		std::free(dataToDelete);

		m_size.fetch_sub(1, std::memory_order_relaxed);
		return true;
	}

	message = "SkipL:ist<K>::remove key not found key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Debug, message);

	return false;
}

template<typename K>
bool SkipList<K>::update(const K key, void* data, const std::uint16_t size) {
	std::string message = "SkipL:ist<K>::update key: " + std::to_string(key);
	m_logger.log(utils::Logger::LogLevel::Trace, message);

	SkipListNode<K>* foundNode = findNode(key);

	if (foundNode != nullptr) {
		void* dataToDelete = foundNode->getData();
		foundNode->setData(data, size);
		std::free(dataToDelete);

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
SkipListNode<K>* SkipList<K>::findNode(const K key) const {
	SkipListNode<K>* foundNode = nullptr;

	for (std::uint8_t level = m_maxNodeHeight - 1; level != 0; level--) {
		SkipListNode<K>* node = m_head->getNext(level);

		while ((node != nullptr) && (key > node->getKey())) {
			node = node->getNext(level);
		}

		if ((node != nullptr) && (key == node->getKey())) {
			foundNode = node;
			break;
		}
	}

	return foundNode;
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipList<int>;

}} // namespace
