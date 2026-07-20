#ifndef INDEX_H
#define INDEX_H

#include "core/Transaction.h"
#include "engine/TableIterator.h"

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace engine {

class Index {
	public:
		virtual ~Index() = default;

		virtual bool insertRow(const void* key, void* row, const std::uint16_t size, core::Transaction* transaction) = 0;
		virtual bool deleteRow(const void* key, core::Transaction* transaction) = 0;
		virtual bool updateRow(const void* key, void* row, const std::uint16_t size, core::Transaction* transaction) = 0;

		virtual TableIterator* scan(core::Transaction* transaction) const = 0;
		virtual TableIterator* rangeScan(const void* startKey, const void* endKey, core::Transaction* transaction) const = 0;
		
		virtual size_t memoryUsageMB() const = 0;
};

}} // namespace

#endif
