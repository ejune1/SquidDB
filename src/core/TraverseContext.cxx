#include "core/TraverseContext.h"

#include "core/SkipListNode.h"

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

template<typename K>
TraverseContext<K>::TraverseContext(const std::uint8_t size) : m_size(size) {
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


}} // namespace
