#include "storage/Schema.h"

#include "storage/Column.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace squiddb { namespace storage {

Schema::Schema() { }

void Schema::read(const std::string /* filePath */) {
	// TODO
}

void Schema::write(const std::string /* filePath */) {
	// TODO
}

void Schema::addColumn(const Column column) {
	size_t offset = 0;
	
	for (const Column& col : m_column) {
		offset += col.getSize();
	}

	m_offset[column.getName()] = offset;
	m_column.push_back(column);
}

std::int32_t Schema::readInt32(const std::string columnName, const std::byte* row) {
	size_t offset = m_offset.at(columnName);

	std::int32_t value = 0;
	memcpy(&value, row + offset, sizeof(std::int32_t));
	
	return value;
}

std::int64_t Schema::readInt64(const std::string columnName, const std::byte* row) {
	size_t offset = m_offset.at(columnName);

	std::int64_t value = 0;
	memcpy(&value, row + offset, sizeof(std::int64_t));

	return value;
}

}} // namespace
