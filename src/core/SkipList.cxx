#include "core/SkipList.h"

#include "core/SkipListNode.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

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
bool SkipList<K>::insert(const K key, void* data, std::uint16_t size) {
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

	return true;
}

template<typename K>
bool SkipList<K>::remove(const K key) {

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

		return true;
	}

	return false;
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipList<int>;

}} // namespace
