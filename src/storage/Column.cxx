#include "storage/Column.h"

#include "utils/StringUtils.h"

#include <string>

namespace squiddb { namespace storage {

Column::Column(const std::string name, const size_t size, const Column::ColumnType columnType, Column::KeyType keyType) {
	m_name = name;
	m_size = size;
	m_type = columnType;
	m_keyType = keyType;
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

Column::KeyType Column::getKeyType() const {
	return m_keyType;
}

std::string Column::columnTypeString(const Column::ColumnType columnType) {
        switch (columnType) {
		case ColumnType::Int32:   return "INT32";
		case ColumnType::Int64:   return "INT64";
		case ColumnType::Float:   return "FLOAT";
		case ColumnType::String:  return "STRING";
		case ColumnType::Unknown: return "UNKNOWN";
        }
        return "UNKNOWN";
}

Column::ColumnType Column::parseColumnType(const std::string columnTypeString) {
	std::string columnTypeStringUpper = utils::StringUtils::toUpper(columnTypeString);

	if (columnTypeStringUpper == "INT32") {
		return ColumnType::Int32;
	}

	if (columnTypeStringUpper == "INT64") {
		return ColumnType::Int64;
	}

	if (columnTypeStringUpper == "FLOAT") {
		return ColumnType::Float;
	}

	if (columnTypeStringUpper == "STRING") {
		return ColumnType::String;
	}

	return ColumnType::Unknown;
}

std::string Column::keyTypeString(const Column::KeyType keyType) {
        switch (keyType) {
		case KeyType::None:      return "NONE";
		case KeyType::Primary:   return "PRIMARY";
		case KeyType::Secondary: return "SECONDARY";
		case KeyType::Unknown:   return "UNKNOWN";
        }
        return "UNKNOWN";
}

Column::KeyType Column::parseKeyType(const std::string keyTypeString) {
	std::string keyTypeStringUpper = utils::StringUtils::toUpper(keyTypeString);

	if (keyTypeStringUpper == "NONE") {
		return KeyType::None;
	}

	if (keyTypeStringUpper == "PRIMARY") {
		return KeyType::Primary;
	}

	if (keyTypeStringUpper == "SECONDARY") {
		return KeyType::Secondary;
	}
	
	return KeyType::Unknown;
}

}} // namespace
