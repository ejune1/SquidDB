#include "storage/FileManager.h"

#include "storage/StreamFile.h"
#include "utils/Logger.h"

#include <cstdint>
#include <filesystem>
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

void FileManager::initialize() {
	
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

}} // namespace
