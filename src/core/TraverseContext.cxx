#include "core/TraverseContext.h"

#include "core/SkipListNode.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

template<typename K>
TraverseContext<K>::TraverseContext(const std::uint8_t size, const LockType lockType) : m_size(size), m_lockType(lockType) {
	m_prevNode = new SkipListNode<K>*[m_size];
	for (std::uint8_t x = 0; x < m_size; x++) {
		m_prevNode[x] = nullptr;
	}

	m_prevRank = new size_t[m_size];
	for (std::uint8_t x = 0; x < m_size; x++) {
		m_prevRank[x] = 0;
	}

	m_rank = 0;
}

template<typename K>
TraverseContext<K>::~TraverseContext() {
	delete [] m_prevNode;
	m_prevNode = nullptr;

	delete [] m_prevRank;
	m_prevRank = nullptr;
}

template<typename K>
std::uint8_t TraverseContext<K>::getSize() const {
	return m_size;
}

template<typename K>
bool TraverseContext<K>::containsNode(SkipListNode<K>* node) const {
	return (m_lockNodes.count(node) > 0);
}

template<typename K>
SkipListNode<K>* TraverseContext<K>::getPrevNode(const std::uint8_t index) const {
	assert(index < m_size);
	return m_prevNode[index];
}

template<typename K>
void TraverseContext<K>::setPrevNode(const std::uint8_t index, SkipListNode<K>* prevNode) {
	assert(index < m_size);
	m_prevNode[index] = prevNode;

	if (m_lockType != LockType::None) {
		if (m_lockNodes.count(prevNode) == 0) {
			if (m_lockType == LockType::Read) {
				prevNode->readLock();
			} else if (m_lockType == LockType::Write) {
				prevNode->writeLock();
			}

			m_lockNodes.insert(prevNode);
		}
	}
}

template<typename K>
size_t TraverseContext<K>::getPrevRank(const std::uint8_t index) const {
	assert(index < m_size);
	return m_prevRank[index];
}

template<typename K>
void TraverseContext<K>::setPrevRank(const std::uint8_t index, const size_t prevRank) {
	assert(index < m_size);
	m_prevRank[index] = prevRank;
}

template<typename K>
size_t TraverseContext<K>::getRank() const {
	return m_rank;
}

template<typename K>
void TraverseContext<K>::setRank(const size_t rank) {
	m_rank = rank;
}

template<typename K>
void TraverseContext<K>::unlockAll() {
	if (m_lockType != LockType::None) {
		for (SkipListNode<K>* node : m_lockNodes) {
			if (m_lockType == LockType::Read) {
				node->readUnlock();
			} else if (m_lockType == LockType::Write) {
				node->writeUnlock();
			}
		}
	}
}

// explicit instantiation - we know what kinds of keys we will get
template class TraverseContext<std::int32_t>;

}} // namespace
