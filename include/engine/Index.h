#ifndef INDEX_H
#define INDEX_H

#include "engine/TableIterator.h"

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace engine {

class Index {
	public:
		virtual ~Index() = default;

		virtual bool insertRow(const void* key, void* row, const std::uint16_t size) = 0;
		virtual bool deleteRow(const void* key) = 0;
		virtual bool updateRow(const void* key, void* row, const std::uint16_t size) = 0;

		virtual TableIterator* scan() const = 0;
		virtual TableIterator* rangeScan(const void* startKey, const void* endKey) const = 0;
};

}} // namespace

#endif
