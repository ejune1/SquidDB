#include "core/SkipListIterator.h"

#include "core/SkipListNode.h"

#include <cstdint>
#include <iterator>
#include <optional>

namespace squiddb { namespace core {

template<typename K>
SkipListIterator<K>::SkipListIterator() {
	m_current = nullptr;
	m_endKey = std::nullopt;
}

template<typename K>
SkipListIterator<K>::SkipListIterator(SkipListNode<K>* startNode, std::optional<int> endKey) {
	m_current = startNode;
	m_endKey = endKey;

	if ((m_endKey.has_value() == true) && (m_current->getKey() > m_endKey)) {
		m_current = nullptr;
	}
}

template<typename K>
typename SkipListIterator<K>::reference SkipListIterator<K>::operator*() const {
	return m_current->getKey();
}

template<typename K>
SkipListIterator<K>& SkipListIterator<K>::operator++() {
	if (m_current != nullptr) {
		m_current = m_current->getNext(0 /* level */);

		if ((m_endKey.has_value() == true) && (m_current->getKey() > m_endKey)) {
			m_current = nullptr;
		}
	}

	return *this;
}

template<typename K>
SkipListIterator<K> SkipListIterator<K>::operator++(int) {
	SkipListIterator<K> tmp = *this;
	++(*this);
	return tmp;
}

template<typename K>
bool SkipListIterator<K>::operator==(const SkipListIterator<K>& other) const {
	return (m_current == other.m_current) && (m_endKey == other.m_endKey);
}

template<typename K>
bool SkipListIterator<K>::operator!=(const SkipListIterator<K>& other) const {
	return (m_current != other.m_current) || (m_endKey != other.m_endKey);
}

template<typename K>
bool SkipListIterator<K>::valid() const {
	return (m_current != nullptr);
}

template<typename K>
void SkipListIterator<K>::next() {
	m_current = m_current->getNext(0 /* level */);

	if ((m_endKey.has_value() == true) && (m_current->getKey() > m_endKey)) {
		m_current = nullptr;
	}
}

template<typename K>
const void* SkipListIterator<K>::getKey() const {
	// TODO fix this in opaque keys and return the internal data
	return static_cast<const void*>(m_current->getKeyRef());
}

template<typename K>
const void* SkipListIterator<K>::getData() const {
	// TODO isolate
	return m_current->getRowInfo()->getData();
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipListIterator<std::int32_t>;

}} // namespace
