#include "engine/Table.h"

#include "engine/Index.h"
#include "engine/TableIterator.h"
#include "storage/Column.h"
#include "storage/Schema.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace squiddb { namespace engine {

Table::Table(utils::Logger& logger, const std::string path, const std::string name) : 
	m_logger(logger), m_path(path), m_name(name), m_schema(logger) {
	m_schemaFinalized = false;
	m_primary = nullptr;
	m_secondary = nullptr;
	m_secondarySize = 0;
}

Table::~Table() {
	if (m_primary != nullptr) {
		delete m_primary;
	}

	if (m_secondarySize > 0) {
		for (int x = 0; x < m_secondarySize; x++) {
			if (m_secondary[x] != nullptr) {
				delete m_secondary[x];
			}
		}

		delete [] m_secondary;
	}
}

void Table::initialize() {
	m_logger.log(utils::Logger::LogLevel::Info, "Table::initialize initializing table " + m_name);
	std::string schemaFilePath = m_path + m_name + ".ss";

	if (std::filesystem::exists(schemaFilePath)) {
		m_logger.log(utils::Logger::LogLevel::Info, "Table::initialize found schema " + schemaFilePath);

		m_schema.read(schemaFilePath);
		m_schemaFinalized = true;
	}

	// TODO create / start indexes
}

void Table::startup() {
	// TODO
	recover();
}

void Table::recover() {
	// TODO
}

void Table::addColumn(const std::string name, const std::uint16_t size, const std::uint8_t columnType) {
	if (m_schemaFinalized == true) {
		throw std::runtime_error("Table::addColumn schema is finalized");
	}

	storage::Column::ColumnType colType = static_cast<storage::Column::ColumnType>(columnType);
	storage::Column::KeyType keyType = storage::Column::KeyType::None;

	storage::Column column(name, size, colType, keyType);
	m_schema.addColumn(column);
}

void Table::addIndex(const std::string name, const std::uint16_t size, const std::uint8_t columnType, const bool primary) {
	if (m_schemaFinalized == true) {
		throw std::runtime_error("Table::addIndex schema is finalized");
	}

	storage::Column::ColumnType colType = static_cast<storage::Column::ColumnType>(columnType);
	storage::Column::KeyType keyType = (primary == true) ? storage::Column::KeyType::Primary : storage::Column::KeyType::Secondary;

	storage::Column column(name, size, colType, keyType);
	m_schema.addColumn(column);
}

void Table::finalizeSchema() {
	m_logger.log(utils::Logger::LogLevel::Info, "Table::finalizeSchema table " + m_name);
	std::string schemaFilePath = m_path + m_name + ".ss";

	m_schema.write(schemaFilePath);
	m_schemaFinalized = true;
}

}} // namespace
