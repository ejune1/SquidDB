#ifndef SCHEMA_H
#define SCHEMA_H

#include "storage/Column.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace squiddb { namespace storage {

class Schema {
	public:
		Schema();
		~Schema() = default;

		void addColumn(const Column column);
		void read(const std::string filePath);

		std::int32_t readInt32(const std::string columnName, const std::byte* row);
		std::int64_t readInt64(const std::string columnName, const std::byte* row);
		// TODO reads for other data types

	private:
		std::vector<Column> m_column;
		std::unordered_map<std::string, size_t> m_offset;
};

}} // namespace

#endif
