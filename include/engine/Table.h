#ifndef TABLE_H
#define TABLE_H

#include "engine/Index.h"
#include "engine/TableIterator.h"
#include "storage/Schema.h"

#include <cstdint>

namespace squiddb { namespace engine {

class Table {
	public:
		Table(std::string name);
		~Table();

		void addColumn(const std::string name, const size_t size, const std::uint8_t columnType);
		void addIndex(const std::string name, const size_t size, const std::uint8_t columnType, const bool primary);

		// force read keys from row
		bool insertRow(void* row);

		bool deleteRow(const void* key);

		bool updateRow(const void* key, void* row);

		TableIterator scan() const;
		TableIterator scan(std::string index) const;
		TableIterator rangeScan(const void* startKey, const void* endKey) const;

	private:
		std::string m_name;
		storage::Schema m_schema;
		
		Index m_primary;
		Index* m_secondary;
		std::uint8_t m_secondarySize;
};

}} // namespace

#endif
