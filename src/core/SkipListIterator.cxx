#include "core/SkipListIterator.h"

#include "core/SkipListNode.h"

#include <iterator>

namespace squiddb { namespace core {

template<typename K>
SkipListIterator<K>::SkipListIterator() {
	current = nullptr;
}

template<typename K>
SkipListIterator<K>::SkipListIterator(SkipListNode<K>* startNode) {
	current = startNode;
}

template<typename K>
typename SkipListIterator<K>::reference SkipListIterator<K>::operator*() const {
	return current->getKey();
}

template<typename K>
SkipListIterator<K>& SkipListIterator<K>::operator++() {
	if (current != nullptr) {
		current = current->getNext(0 /* level */);
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
	return (current == other.current);
}

template<typename K>
bool SkipListIterator<K>::operator!=(const SkipListIterator<K>& other) const {
	return (current != other.current);
}

template<typename K>
bool SkipListIterator<K>::valid() const {
	return (current != nullptr);
}

template<typename K>
void SkipListIterator<K>::next() {
	current = current->getNext(0 /* level */);
}

template<typename K>
const void* SkipListIterator<K>::getKey() const {
	// TODO fix this in opaque keys and return the internal data
	return static_cast<const void*>(current->getKeyRef());
}

template<typename K>
const void* SkipListIterator<K>::getData() const {
	return current->getData();
}

// explicit instantiation - we know what kinds of keys we will get
template class SkipListIterator<int>;

}} // namespace
