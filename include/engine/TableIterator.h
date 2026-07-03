#ifndef TABLEITERATOR_H
#define TABLEITERATOR_H

#include <iterator>

namespace squiddb { namespace engine {

class TableIterator {
	public:
		virtual ~TableIterator() = default;

		virtual bool valid() const = 0;
		virtual void next() = 0;

		virtual const void* getKey() const = 0;
		virtual const void* getData() const = 0;
};

}} // namespace

#endif
