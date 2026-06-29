#include "core/SkipListNode.h"

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

template<typename K>
SkipListNode<K>::SkipListNode(const K key, void* data, std::uint16_t size, const std::uint8_t height)
	: m_key(key), m_height(height) {
	m_data = data;
	m_size = size;

	assert(m_height > 0);

	m_next = new SkipListNode<K>*[m_height];
	for (std::uint8_t x = 0; x < m_height; x++) {
		m_next[x] = nullptr;
	}

	m_width = new size_t[m_height];
	for (std::uint8_t x = 0; x < m_height; x++) {
		m_width[x] = 0;
	}
}

template<typename K>
SkipListNode<K>::~SkipListNode() {
	delete [] m_next;
	m_next = nullptr;

	delete [] m_width;
	m_width = nullptr;
}

template<typename K>
K SkipListNode<K>::getKey() const {
	return m_key;
}

template<typename K>
SkipListNode<K>* SkipListNode<K>::getNext(const std::uint8_t level) const {
	assert(level < m_height);
	return m_next[level];
}

template<typename K>
void SkipListNode<K>::setNext(const std::uint8_t level, SkipListNode<K>* next) {
	assert(level < m_height);
	m_next[level] = next;
}

template<typename K>
size_t SkipListNode<K>::getWidth(const std::uint8_t level) const {
	assert(level < m_height);
	return m_width[level];
}

template<typename K>
void SkipListNode<K>::setWidth(const std::uint8_t level, const size_t width) {
	assert(level < m_height);
	m_width[level] = width;
}

template<typename K>
std::uint8_t SkipListNode<K>::getHeight() const {
	return m_height;
}

template<typename K>
void* SkipListNode<K>::getData() {
	return m_data;
}

template<typename K>
void SkipListNode<K>::setData(void* data, std::uint16_t size) {
	m_data = data;
	m_size = size;
}

template<typename K>
std::uint16_t SkipListNode<K>::getSize() const {
	return m_size;
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipListNode<int>;

}} // namespace
