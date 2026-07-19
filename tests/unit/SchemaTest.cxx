#include "test/catch.hpp"

#include "storage/Column.h"
#include "storage/Schema.h"
#include "utils/FileCleanup.h"
#include "utils/Logger.h"

#include <filesystem>

using namespace squiddb;

TEST_CASE("Schema simple create single int primary and check Column", "[schema]") {
	utils::Logger& logger = utils::Logger::getInstance();
        logger.setLogLevel(utils::Logger::LogLevel::Info);
        logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
        logger.start();

	storage::Schema schema(logger);

	storage::Column col1("col1", 32, storage::Column::ColumnType::Int32, storage::Column::KeyType::Primary);
	schema.addColumn(col1);

	REQUIRE(schema.getColumns().size() == 1);

	REQUIRE(schema.getColumns()[0].getName() == "col1");
	REQUIRE(schema.getColumns()[0].getSize() == 32);
	REQUIRE(schema.getColumns()[0].getColumnType() == storage::Column::ColumnType::Int32);
	REQUIRE(schema.getColumns()[0].getKeyType() == storage::Column::KeyType::Primary);

	logger.stop();
}

TEST_CASE("Schema simple create multiple Columns and check Columns", "[schema]") {
	utils::Logger& logger = utils::Logger::getInstance();
        logger.setLogLevel(utils::Logger::LogLevel::Info);
        logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
        logger.start();

	storage::Schema schema(logger);

	storage::Column col1("col1", 32, storage::Column::ColumnType::Int32, storage::Column::KeyType::Primary);
	storage::Column col2("col2", 64, storage::Column::ColumnType::Int64, storage::Column::KeyType::Secondary);
	storage::Column col3("col3", 100, storage::Column::ColumnType::String, storage::Column::KeyType::None);
	schema.addColumn(col1);
	schema.addColumn(col2);
	schema.addColumn(col3);

	REQUIRE(schema.getColumns().size() == 3);

	REQUIRE(schema.getColumns()[0].getName() == "col1");
	REQUIRE(schema.getColumns()[0].getSize() == 32);
	REQUIRE(schema.getColumns()[0].getColumnType() == storage::Column::ColumnType::Int32);
	REQUIRE(schema.getColumns()[0].getKeyType() == storage::Column::KeyType::Primary);

	REQUIRE(schema.getColumns()[1].getName() == "col2");
	REQUIRE(schema.getColumns()[1].getSize() == 64);
	REQUIRE(schema.getColumns()[1].getColumnType() == storage::Column::ColumnType::Int64);
	REQUIRE(schema.getColumns()[1].getKeyType() == storage::Column::KeyType::Secondary);

	REQUIRE(schema.getColumns()[2].getName() == "col3");
	REQUIRE(schema.getColumns()[2].getSize() == 100);
	REQUIRE(schema.getColumns()[2].getColumnType() == storage::Column::ColumnType::String);
	REQUIRE(schema.getColumns()[2].getKeyType() == storage::Column::KeyType::None);

	logger.stop();
}

TEST_CASE("Schema create single int primary and write/read check Column", "[schema]") {
	std::string filePath = "./table.ss";
	utils::FileCleanup fileCleanup(filePath);

	utils::Logger& logger = utils::Logger::getInstance();
        logger.setLogLevel(utils::Logger::LogLevel::Info);
        logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
        logger.start();

	storage::Schema schemaWrite(logger);

	storage::Column col1("col1", 32, storage::Column::ColumnType::Int32, storage::Column::KeyType::Primary);
	schemaWrite.addColumn(col1);
	schemaWrite.write(filePath);

	storage::Schema schemaRead(logger);
	schemaRead.read(filePath);

	REQUIRE(schemaRead.getColumns().size() == 1);

	REQUIRE(schemaRead.getColumns()[0].getName() == "col1");
	REQUIRE(schemaRead.getColumns()[0].getSize() == 32);
	REQUIRE(schemaRead.getColumns()[0].getColumnType() == storage::Column::ColumnType::Int32);
	REQUIRE(schemaRead.getColumns()[0].getKeyType() == storage::Column::KeyType::Primary);

	logger.stop();
}

TEST_CASE("Schema create multiple Columns and write/read check Columns", "[schema]") {
	std::string filePath = "./table.ss";
	utils::FileCleanup fileCleanup(filePath);

	utils::Logger& logger = utils::Logger::getInstance();
        logger.setLogLevel(utils::Logger::LogLevel::Info);
        logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
        logger.start();

	storage::Schema schemaWrite(logger);

	storage::Column col1("col1", 32, storage::Column::ColumnType::Int32, storage::Column::KeyType::Primary);
	storage::Column col2("col2", 64, storage::Column::ColumnType::Int64, storage::Column::KeyType::Secondary);
	storage::Column col3("col3", 100, storage::Column::ColumnType::String, storage::Column::KeyType::None);
	schemaWrite.addColumn(col1);
	schemaWrite.addColumn(col2);
	schemaWrite.addColumn(col3);

	schemaWrite.write(filePath);

	storage::Schema schemaRead(logger);
	schemaRead.read(filePath);

	REQUIRE(schemaRead.getColumns().size() == 3);

	REQUIRE(schemaRead.getColumns()[0].getName() == "col1");
	REQUIRE(schemaRead.getColumns()[0].getSize() == 32);
	REQUIRE(schemaRead.getColumns()[0].getColumnType() == storage::Column::ColumnType::Int32);
	REQUIRE(schemaRead.getColumns()[0].getKeyType() == storage::Column::KeyType::Primary);

	REQUIRE(schemaRead.getColumns()[1].getName() == "col2");
	REQUIRE(schemaRead.getColumns()[1].getSize() == 64);
	REQUIRE(schemaRead.getColumns()[1].getColumnType() == storage::Column::ColumnType::Int64);
	REQUIRE(schemaRead.getColumns()[1].getKeyType() == storage::Column::KeyType::Secondary);

	REQUIRE(schemaRead.getColumns()[2].getName() == "col3");
	REQUIRE(schemaRead.getColumns()[2].getSize() == 100);
	REQUIRE(schemaRead.getColumns()[2].getColumnType() == storage::Column::ColumnType::String);
	REQUIRE(schemaRead.getColumns()[2].getKeyType() == storage::Column::KeyType::None);

	logger.stop();
}
