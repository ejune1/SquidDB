#include "storage/Column.h"

#include <string>

namespace squiddb { namespace storage {

Column::Column(const std::string name, const size_t size, const Column::ColumnType columnType, bool primaryKey, bool secondaryKey) {
	m_name = name;
	m_size = size;
	m_type = columnType;
	m_primaryKey = primaryKey;
	m_secondaryKey = secondaryKey;
}

std::string Column::getName() const {
	return m_name;
}

size_t Column::getSize() const {
	return m_size;
}

Column::ColumnType Column::getColumnType() const {
	return m_type;
}

bool Column::getPrimaryKey() const {
	return m_primaryKey;
}

bool Column::getSecondaryKey() const {
	return m_secondaryKey;
}

}} // namespace
