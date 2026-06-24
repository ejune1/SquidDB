#include "core/SkipListNode.h"

#include <cstdint>

namespace squiddb { namespace core {

template<typename K>
SkipListNode<K>::SkipListNode(K key, std::uint8_t height) {
	m_key = key;
	m_height = height;
	m_data = nullptr;
	m_size = 0;

	m_next = new SkipListNode<K>*[m_height];
	for (std::uint8_t x = 0; x < m_height; x++) {
		m_next[x] = nullptr;
	}
}

template<typename K>
SkipListNode<K>::~SkipListNode() {
	delete [] m_next;
	m_next = nullptr;
}

template<typename K>
K SkipListNode<K>::getKey() const {
	return m_key;
}

template<typename K>
SkipListNode<K>* SkipListNode<K>::getNext(const std::uint8_t level) const {
	return m_next[level];
}

template<typename K>
std::uint8_t SkipListNode<K>::getHeight() const {
	return m_height;
}

template<typename K>
void* SkipListNode<K>::getData() const {
	return m_data;
}

template<typename K>
void SkipListNode<K>::setData(void* data) {
	m_data = data;
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipListNode<int>;

}} // namespace
