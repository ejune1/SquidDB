#include "core/SkipListIterator.h"

#include "core/SkipListNode.h"
#include "core/Transaction.h"

#include <cstdint>
#include <iterator>
#include <optional>

namespace squiddb { namespace core {

template<typename K>
SkipListIterator<K>::SkipListIterator() {
	m_current = nullptr;
	m_endKey = std::nullopt;
	m_transaction = nullptr;
}

template<typename K>
SkipListIterator<K>::SkipListIterator(SkipListNode<K>* startNode, std::optional<int> endKey, Transaction* transaction) {
	m_current = startNode;
	m_endKey = endKey;
	m_transaction = transaction;

	transactionalAdvance();

	if ((m_endKey.has_value() == true) && (m_current != nullptr) && (m_current->getKey() > m_endKey)) {
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
		transactionalAdvance();

		if ((m_endKey.has_value() == true) && (m_current != nullptr) && (m_current->getKey() > m_endKey)) {
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
	transactionalAdvance();

	if ((m_endKey.has_value() == true) && (m_current != nullptr) && (m_current->getKey() > m_endKey)) {
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
	RowInfo* rowInfo = m_current->getRowInfo();

	if (m_transaction != nullptr) {
		rowInfo = m_transaction->isolateRowInfo(rowInfo);
	}

	return rowInfo->getData();
}

template<typename K>
void SkipListIterator<K>::transactionalAdvance() {
	if ((m_transaction != nullptr) && (m_current != nullptr)) {
		RowInfo* rowInfo = m_current->getRowInfo();
		rowInfo = m_transaction->isolateRowInfo(rowInfo);

		while ((rowInfo == nullptr) || (rowInfo->getDeleting() == true)) {
			m_current = m_current->getNext(0 /* level */);

			if (m_current == nullptr) {
				return;
			}

			rowInfo = m_current->getRowInfo();
			rowInfo = m_transaction->isolateRowInfo(rowInfo);
		}
	}
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipListIterator<std::int32_t>;

}} // namespace
