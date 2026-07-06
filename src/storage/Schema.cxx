#include "storage/Schema.h"

#include "storage/Column.h"
#include "utils/Logger.h"
#include "utils/StringUtils.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace squiddb { namespace storage {

Schema::Schema(utils::Logger& logger) : m_logger(logger) { }

void Schema::read(const std::string /* filePath */) {
	// TODO
}

void Schema::write(const std::string filePath) {
	std::ofstream outputFile(filePath);	

	if (outputFile.is_open() == false) {
		throw std::runtime_error("Schema::write unable to write file " + filePath);
	}

	m_logger.log(utils::Logger::LogLevel::Info, "Schema::write writing schema file " + filePath);

	for (Column& column : m_column) {
		outputFile << column.getName() << ":";
		outputFile << column.getSize() << ":";
		outputFile << Column::columnTypeString(column.getColumnType()) << ":";
		outputFile << Column::keyTypeString(column.getKeyType());
		outputFile << "\n";
	}

	outputFile.close();
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
