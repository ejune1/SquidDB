#include "test/catch.hpp"

#include "storage/FileManager.h"
#include "storage/StreamFile.h"
#include "utils/DirectoryCleanup.h"
#include "utils/Logger.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

using namespace squiddb;

TEST_CASE("file manager simple non rollover single file create", "[file manager]") {
	utils::Logger& logger = utils::Logger::getInstance();
	logger.setLogLevel(utils::Logger::LogLevel::Info);
	logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
	logger.start();

	std::string directory = "./data/";
	std::string tableName = "TestTable";
	std::uint16_t maxFileSizeMB = 1;

	utils::DirectoryCleanup directoryCleanup(directory);
	directoryCleanup.createFile(tableName, 0, ".ls");
	directoryCleanup.createFile(tableName, 0, ".vs");

	storage::FileManager fileManager(logger, directory, tableName, maxFileSizeMB);
	fileManager.initialize();

	std::vector<storage::StreamFile*> lsFiles = fileManager.getLsFiles();
	std::vector<storage::StreamFile*> vsFiles = fileManager.getVsFiles();

	REQUIRE(lsFiles.size() == 1);
	REQUIRE(vsFiles.size() == 1);

	REQUIRE(lsFiles[0]->getSequenceNumber() == 0);
	REQUIRE(vsFiles[0]->getSequenceNumber() == 0);

	REQUIRE(fileManager.validate() == true);

	logger.stop();
}
