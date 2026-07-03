#ifndef TABLEITERATOR_H
#define TABLEITERATOR_H

#include <iterator>

namespace squiddb { namespace engine {

class TableIterator {
	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type        = void*;
		using difference_type   = std::ptrdiff_t;
		using pointer           = const void*;
		using reference         = void*;

		TableIterator();
		virtual ~TableIterator();

		virtual reference operator*() const;
		virtual TableIterator& operator++();
		virtual TableIterator operator++(int);

		virtual bool operator==(const TableIterator& other) const;
		virtual bool operator!=(const TableIterator& other) const;	
};

}} // namespace

#endif
