#include "storage/FileManager.h"

#include "storage/StreamFile.h"
#include "utils/Logger.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iterator>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <vector>

namespace squiddb { namespace storage {

FileManager::FileManager(utils::Logger& logger, 
	const std::string dataPath, 
	const std::string tableName, 
	const std::uint16_t maxFileSizeMB) :
	m_logger(logger), 
	m_dataPath(dataPath), 
	m_tableName(tableName),
	m_maxFileSizeMB(maxFileSizeMB) { }

FileManager::~FileManager() {
	for (StreamFile* streamFile : m_lsFiles) {
		delete streamFile;
	}
	m_lsFiles.clear();

	for (StreamFile* streamFile : m_vsFiles) {
		delete streamFile;
	}
	m_vsFiles.clear();
}

void FileManager::initialize() {
	std::vector<std::string> lsFilePaths = getFilePaths(".ls");	
	std::vector<std::string> vsFilePaths = getFilePaths(".vs");

	for (std::string lsFilePath : lsFilePaths) {
		std::uint8_t sequenceNumber = getSequenceNumber(lsFilePath);
		StreamFile* streamFile = new StreamFile(lsFilePath);
		streamFile->setSequenceNumber(sequenceNumber);

		m_logger.log(utils::Logger::LogLevel::Info, "FileManager::initialize discovered ls file " + lsFilePath);

		m_lsFiles.push_back(streamFile);
	}

	for (std::string vsFilePath : vsFilePaths) {
		std::uint8_t sequenceNumber = getSequenceNumber(vsFilePath);
		StreamFile* streamFile = new StreamFile(vsFilePath);
		streamFile->setSequenceNumber(sequenceNumber);

		m_logger.log(utils::Logger::LogLevel::Info, "FileManager::initialize discovered vs file " + vsFilePath);

		m_vsFiles.push_back(streamFile);
	}

	m_logger.log(utils::Logger::LogLevel::Info, "FileManager::initialize ordering ls files..");
	orderFiles(m_lsFiles);

	m_logger.log(utils::Logger::LogLevel::Info, "FileManager::initialize ordering vs files..");
	orderFiles(m_vsFiles);
}

std::vector<std::string> FileManager::getFilePaths(const std::string extension) const {
	if ((std::filesystem::exists(m_dataPath) == false) || (std::filesystem::is_directory(m_dataPath) == false)) {
		throw std::runtime_error("FileManager::getFilePaths directory doesn't exist " + m_dataPath);
	}

	std::vector<std::string> filePaths;
	std::filesystem::directory_iterator directoryIterator(m_dataPath);

	for (const std::filesystem::directory_entry& directoryEntry : directoryIterator) {
		if (directoryEntry.is_regular_file() == false) {
			throw std::runtime_error("FileManager::getFilePaths directory contains abnormal file " + m_dataPath);
		}

		std::filesystem::path filePath = directoryEntry.path();
		std::string fileName = filePath.filename().string();

		bool tableMatch = (fileName.rfind(m_tableName, 0) == 0);
		bool extensionMatch = (filePath.extension() == extension);

		if ((tableMatch == true) && (extensionMatch == true)) {
			filePaths.push_back(filePath.string());
		}
	}

	return filePaths;
}

std::uint8_t FileManager::getSequenceNumber(const std::string filePath) const {
	std::string::size_type lastDot = filePath.find_last_of('.');
	std::string::size_type prevDot = filePath.find_last_of('.', lastDot - 1);

	if ((lastDot == std::string::npos) || (prevDot == std::string::npos)) {
		throw std::runtime_error("FileManager::getSequenceNumber invalid file name " + filePath);
	}

	std::string::size_type start = prevDot + 1;
	std::string::size_type length = lastDot - start;
	std::string numString = filePath.substr(start, length);

	int num = std::stoi(numString);

	if ((num < 0) || (num > 255)) {
		throw std::runtime_error("FileManager::getSequenceNumber invalid sequence number " + filePath);
	}

	return static_cast<std::uint8_t>(num);
}

void FileManager::orderFiles(std::vector<StreamFile*>& files) {
	if (files.size() <= 1) {
		return;
	}

	if (files.size() > 256) {
		throw std::runtime_error("FileManager::orderFiles too many files " + m_tableName);
	}

	// first sort by sequence number
	std::sort(files.begin(), files.end(), [](const StreamFile* a, const StreamFile* b) {
		return a->getSequenceNumber() < b->getSequenceNumber();
	});

	// max files is max value of std::uint8_t + 1
	if (files.size() == 256) {
		return;
	}

	// if there is a gap in the sequence number, it has rolled over. first sequence number is first after gap
	std::vector<StreamFile*>::iterator gapIt = std::adjacent_find(files.begin(), files.end(),
		[](const StreamFile* a, const StreamFile* b) {
			return ((b->getSequenceNumber() - a->getSequenceNumber()) > 1);
		}
	);

	// rotate based on gap if found
	if (gapIt != files.end()) {
		std::vector<StreamFile*>::iterator start = std::next(gapIt);
		std::rotate(files.begin(), start, files.end());
	}
}

}} // namespace
