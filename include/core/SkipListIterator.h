#ifndef SKIPLISTITERATOR_H
#define SKIPLISTITERATOR_H

#include "core/SkipListNode.h"

#include <iterator>

namespace squiddb { namespace core {

template<typename K>
class SkipListIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type        = K;
		using difference_type   = std::ptrdiff_t;
		using pointer           = const K*;
		using reference         = K;

		SkipListIterator();
		SkipListIterator(SkipListNode<K>* startNode);

		reference operator*() const;
		SkipListIterator<K>& operator++();
		SkipListIterator<K> operator++(int);

		bool operator==(const SkipListIterator<K>& other) const;
		bool operator!=(const SkipListIterator<K>& other) const;	
	
	private:
		SkipListNode<K>* current;
};

}} // namespace

#endif
