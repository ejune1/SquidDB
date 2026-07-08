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

Schema::Schema(utils::Logger& logger) : m_logger(logger) {
	m_totalSize = 0;
	m_primaryOffset = 0;
	m_primarySize = 0;
}

void Schema::read(const std::string filePath) {
	std::ifstream inputFile(filePath);
	std::string line;
	int lineNum = 0;

	if (inputFile.is_open() == false) {
		throw std::runtime_error("Schema::read unable to read file " + filePath);
	}

	while (std::getline(inputFile, line)) {
		lineNum++;

		line = utils::StringUtils::trim(line);

		if ((line.empty() == true) || (line[0] == '#')) {
			continue;
		}

		std::vector<std::string> tokens = utils::StringUtils::split(line, ':');
		if (tokens.size() != 4) {
			throw std::runtime_error("Schema::read invalid schema line " + line);
		}

		// TODO do some checking here
		std::string name = tokens[0];
		std::uint16_t size = static_cast<std::uint16_t>(std::stoul(tokens[1]));
		Column::ColumnType columnType = Column::parseColumnType(tokens[2]);
		Column::KeyType keyType = Column::parseKeyType(tokens[3]);

		std::string logString = name + ":" + std::to_string(size) + ":" + Column::columnTypeString(columnType) +
			":" + Column::keyTypeString(keyType);

		m_logger.log(utils::Logger::LogLevel::Info, "Schema::read got Column " + logString);

		Column column(name, size, columnType, keyType);
		addColumn(column);
	}
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
	if (column.getKeyType() == Column::KeyType::Primary) {
		m_primaryOffset = m_totalSize;
		m_primarySize = column.getSize();
	}

	m_offset[column.getName()] = m_totalSize;
	m_totalSize += column.getSize();

	m_column.push_back(column);
}

const std::vector<Column>& Schema::getColumns() const {
	return m_column;
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

std::uint16_t Schema::getOffset(const std::string& columnName) const {
	return m_offset.at(columnName);
}

std::uint16_t Schema::getTotalSize() const {
	return m_totalSize;
}

std::uint16_t Schema::getPrimaryOffset() const {
	return m_primaryOffset;
}

std::uint16_t Schema::getPrimarySize() const {
	return m_primarySize;
}

}} // namespace
