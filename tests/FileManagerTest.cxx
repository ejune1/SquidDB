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

	// file manager needs to be destroyed before directory cleanup
	{
		storage::FileManager fileManager(logger, directory, tableName, maxFileSizeMB);
		fileManager.initialize();

		std::vector<storage::StreamFile*> lsFiles = fileManager.getLsFiles();
		std::vector<storage::StreamFile*> vsFiles = fileManager.getVsFiles();

		REQUIRE(lsFiles.size() == 1);
		REQUIRE(vsFiles.size() == 1);

		REQUIRE(lsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(vsFiles[0]->getSequenceNumber() == 0);

		REQUIRE(fileManager.validate() == true);
	}

	logger.stop();
}

TEST_CASE("file manager simple non rollover multiple file create", "[file manager]") {
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
	directoryCleanup.createFile(tableName, 1, ".ls");
	directoryCleanup.createFile(tableName, 1, ".vs");
	directoryCleanup.createFile(tableName, 2, ".ls");
	directoryCleanup.createFile(tableName, 2, ".vs");
	directoryCleanup.createFile(tableName, 3, ".ls");
	directoryCleanup.createFile(tableName, 3, ".vs");
	directoryCleanup.createFile(tableName, 4, ".ls");
	directoryCleanup.createFile(tableName, 4, ".vs");

	{
		storage::FileManager fileManager(logger, directory, tableName, maxFileSizeMB);
		fileManager.initialize();

		std::vector<storage::StreamFile*> lsFiles = fileManager.getLsFiles();
		std::vector<storage::StreamFile*> vsFiles = fileManager.getVsFiles();

		REQUIRE(lsFiles.size() == 5);
		REQUIRE(vsFiles.size() == 5);

		REQUIRE(lsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(vsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(lsFiles[1]->getSequenceNumber() == 1);
		REQUIRE(vsFiles[1]->getSequenceNumber() == 1);
		REQUIRE(lsFiles[2]->getSequenceNumber() == 2);
		REQUIRE(vsFiles[2]->getSequenceNumber() == 2);
		REQUIRE(lsFiles[3]->getSequenceNumber() == 3);
		REQUIRE(vsFiles[3]->getSequenceNumber() == 3);
		REQUIRE(lsFiles[4]->getSequenceNumber() == 4);
		REQUIRE(vsFiles[4]->getSequenceNumber() == 4);

		REQUIRE(fileManager.validate() == true);
	}

	logger.stop();
}

TEST_CASE("file manager simple rollover multiple file create", "[file manager]") {
	utils::Logger& logger = utils::Logger::getInstance();
	logger.setLogLevel(utils::Logger::LogLevel::Info);
	logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
	logger.start();

	std::string directory = "./data/";
	std::string tableName = "TestTable";
	std::uint16_t maxFileSizeMB = 1;

	utils::DirectoryCleanup directoryCleanup(directory);
	directoryCleanup.createFile(tableName, 254, ".ls");
	directoryCleanup.createFile(tableName, 254, ".vs");
	directoryCleanup.createFile(tableName, 255, ".ls");
	directoryCleanup.createFile(tableName, 255, ".vs");
	directoryCleanup.createFile(tableName, 0, ".ls");
	directoryCleanup.createFile(tableName, 0, ".vs");
	directoryCleanup.createFile(tableName, 1, ".ls");
	directoryCleanup.createFile(tableName, 1, ".vs");
	directoryCleanup.createFile(tableName, 2, ".ls");
	directoryCleanup.createFile(tableName, 2, ".vs");

	{
		storage::FileManager fileManager(logger, directory, tableName, maxFileSizeMB);
		fileManager.initialize();

		std::vector<storage::StreamFile*> lsFiles = fileManager.getLsFiles();
		std::vector<storage::StreamFile*> vsFiles = fileManager.getVsFiles();

		REQUIRE(lsFiles.size() == 5);
		REQUIRE(vsFiles.size() == 5);

		REQUIRE(lsFiles[0]->getSequenceNumber() == 254);
		REQUIRE(vsFiles[0]->getSequenceNumber() == 254);
		REQUIRE(lsFiles[1]->getSequenceNumber() == 255);
		REQUIRE(vsFiles[1]->getSequenceNumber() == 255);
		REQUIRE(lsFiles[2]->getSequenceNumber() == 0);
		REQUIRE(vsFiles[2]->getSequenceNumber() == 0);
		REQUIRE(lsFiles[3]->getSequenceNumber() == 1);
		REQUIRE(vsFiles[3]->getSequenceNumber() == 1);
		REQUIRE(lsFiles[4]->getSequenceNumber() == 2);
		REQUIRE(vsFiles[4]->getSequenceNumber() == 2);

		REQUIRE(fileManager.validate() == true);
	}

	logger.stop();
}

TEST_CASE("file manager add files, getCurrentFile, getFile with restart", "[file manager]") {
	utils::Logger& logger = utils::Logger::getInstance();
	logger.setLogLevel(utils::Logger::LogLevel::Info);
	logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
	logger.start();

	std::string directory = "./data/";
	std::string tableName = "TestTable";
	std::uint16_t maxFileSizeMB = 1;

	utils::DirectoryCleanup directoryCleanup(directory);

	{
		storage::FileManager fileManager(logger, directory, tableName, maxFileSizeMB);
		fileManager.initialize();

		storage::StreamFile* lsFile = fileManager.addLsFile();
		storage::StreamFile* vsFile = fileManager.addVsFile();

		REQUIRE(lsFile->getSequenceNumber() == 0);
		REQUIRE(vsFile->getSequenceNumber() == 0);

		REQUIRE(fileManager.getCurrentLsFile() == lsFile);
		REQUIRE(fileManager.getCurrentVsFile() == vsFile);

		directoryCleanup.addFile(lsFile->getFilePath());
		directoryCleanup.addFile(vsFile->getFilePath());

		lsFile = fileManager.addLsFile();
		vsFile = fileManager.addVsFile();

		REQUIRE(lsFile->getSequenceNumber() == 1);
		REQUIRE(vsFile->getSequenceNumber() == 1);

		REQUIRE(fileManager.getCurrentLsFile() == lsFile);
		REQUIRE(fileManager.getCurrentVsFile() == vsFile);

		directoryCleanup.addFile(lsFile->getFilePath());
		directoryCleanup.addFile(vsFile->getFilePath());

		lsFile = fileManager.addLsFile();
		vsFile = fileManager.addVsFile();

		REQUIRE(lsFile->getSequenceNumber() == 2);
		REQUIRE(vsFile->getSequenceNumber() == 2);

		REQUIRE(fileManager.getCurrentLsFile() == lsFile);
		REQUIRE(fileManager.getCurrentVsFile() == vsFile);

		directoryCleanup.addFile(lsFile->getFilePath());
		directoryCleanup.addFile(vsFile->getFilePath());

		std::vector<storage::StreamFile*> lsFiles = fileManager.getLsFiles();
		std::vector<storage::StreamFile*> vsFiles = fileManager.getVsFiles();

		REQUIRE(lsFiles.size() == 3);
		REQUIRE(vsFiles.size() == 3);

		REQUIRE(lsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(vsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(lsFiles[1]->getSequenceNumber() == 1);
		REQUIRE(vsFiles[1]->getSequenceNumber() == 1);
		REQUIRE(lsFiles[2]->getSequenceNumber() == 2);
		REQUIRE(vsFiles[2]->getSequenceNumber() == 2);

		REQUIRE(fileManager.validate() == true);
	}

	{
		storage::FileManager fileManager(logger, directory, tableName, maxFileSizeMB);
		fileManager.initialize();

		std::vector<storage::StreamFile*> lsFiles = fileManager.getLsFiles();
		std::vector<storage::StreamFile*> vsFiles = fileManager.getVsFiles();

		REQUIRE(lsFiles.size() == 3);
		REQUIRE(vsFiles.size() == 3);

		REQUIRE(lsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(vsFiles[0]->getSequenceNumber() == 0);
		REQUIRE(lsFiles[1]->getSequenceNumber() == 1);
		REQUIRE(vsFiles[1]->getSequenceNumber() == 1);
		REQUIRE(lsFiles[2]->getSequenceNumber() == 2);
		REQUIRE(vsFiles[2]->getSequenceNumber() == 2);

		REQUIRE(fileManager.validate() == true);

	}

	logger.stop();
}
