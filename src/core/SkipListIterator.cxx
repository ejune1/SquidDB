#include "core/SkipListIterator.h"

#include "core/SkipList.h"
#include "core/SkipListNode.h"
#include "core/Transaction.h"

#include <atomic>
#include <cstdint>
#include <iterator>
#include <optional>

namespace squiddb { namespace core {

template<typename K>
SkipListIterator<K>::SkipListIterator() {
	m_skipList = nullptr;
	m_current = nullptr;
	m_endKey = std::nullopt;
	m_transaction = nullptr;
	m_sequence = 0;
}

template<typename K>
SkipListIterator<K>::SkipListIterator(
	const SkipList<K>* skipList,
	const SkipListNode<K>* startNode, 
	std::optional<int> endKey, 
	Transaction* transaction, 
	std::size_t sequence
	) {
	m_skipList = skipList;
	m_current = startNode;
	m_endKey = endKey;
	m_transaction = transaction;
	m_sequence = sequence;

	if (m_current != nullptr) {
		m_current->readLock();
	}
	advance(false /* next */);
}

template<typename K>
SkipListIterator<K>::~SkipListIterator() {
	if (m_current != nullptr) {
		m_current->readUnlock();
	}
}

template<typename K>
typename SkipListIterator<K>::reference SkipListIterator<K>::operator*() const {
	return m_current->getKey();
}

template<typename K>
SkipListIterator<K>& SkipListIterator<K>::operator++() {
	advance(true /* next */);
	return *this;
}

template<typename K>
SkipListIterator<K> SkipListIterator<K>::operator++(int) {
	SkipListIterator<K> tmp = *this;
	++(*this);

	if (tmp.m_current != nullptr) {
		tmp.m_current->readLock();
	}
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
	advance(true /* next */);
}

template<typename K>
const void* SkipListIterator<K>::getKey() const {
	// TODO fix this in opaque keys and return the internal data
	if (m_current != nullptr) {
		return static_cast<const void*>(m_current->getKeyRef());
	}

	return nullptr;
}

template<typename K>
const void* SkipListIterator<K>::getData() const {
	if (m_current != nullptr) {
		RowInfo* rowInfo = m_current->getRowInfo();

		if (m_transaction != nullptr) {
			rowInfo = m_transaction->isolateRowInfo(rowInfo);
		}

		return rowInfo->getData();
	}

	return nullptr;
}

template<typename K>
void SkipListIterator<K>::advance(bool next) {
	RETRY:
	if ((m_current != nullptr) && (next == true)) {
		const SkipListNode<K>* last = m_current;
		m_current = m_current->getNext(0 /* level */);

		last->readUnlock();
		if (m_current != nullptr) {
			m_current->readLock();
		}
	}

	if ((m_current != nullptr) && (m_transaction != nullptr)) {
		RowInfo* rowInfo = m_current->getRowInfo();
		rowInfo = m_transaction->isolateRowInfo(rowInfo);

		while ((rowInfo == nullptr) || (rowInfo->getDeleting() == true)) {
			const SkipListNode<K>* last = m_current;
			m_current = m_current->getNext(0 /* level */);

			last->readUnlock();
			if (m_current == nullptr) {
				return;
			}
			m_current->readLock();

			rowInfo = m_current->getRowInfo();
			rowInfo = m_transaction->isolateRowInfo(rowInfo);
		}
	}

	if ((m_current != nullptr) && (m_endKey.has_value() == true) && (m_current->getKey() > m_endKey)) {
		m_current->readUnlock();
		m_current = nullptr;
	}

	if (m_current != nullptr) {
		std::size_t currentSequence = m_skipList->m_sequenceLock.load(std::memory_order_acquire);
		if (m_sequence < currentSequence) {
			m_current->readUnlock();

			m_current = m_skipList->lowerBound(m_current->getKey());
			m_current->readLock();
			m_sequence = currentSequence;

			goto RETRY;
		}
	}
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipListIterator<std::int32_t>;

}} // namespace
