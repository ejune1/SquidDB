#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include "storage/StreamFile.h"
#include "utils/Logger.h"

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

namespace squiddb { namespace storage {

class FileManager {
	public:
		FileManager(utils::Logger& logger, const std::string dataPath, const std::uint16_t maxFileSizeMB);
		~FileManager() = default;

		void initialize();

		std::unique_lock<std::mutex> lockLsFiles();
		// for point reads, drop the lock after getting the file
		std::unique_lock<std::mutex> lockVsFiles();
		// std::unique_lock<std::mutex> lockIsFiles();

		StreamFile* getCurrentLsFile() const;
		StreamFile* getCurrentVsFile() const;
		// StreamFile* getCurrentIsFile() const;

		StreamFile* getLsFile(std::uint8_t sequenceNumber) const;
		StreamFile* getVsFile(std::uint8_t sequenceNumber) const;
		// StreamFile* getIsFile(std::uint8_t sequenceNumber) const;

		StreamFile* addLsFile();
		StreamFile* addVsFile();
		// StreamFile* addIsFile();

		void removeLsVsFiles(std::size_t minTransactionId);
		// void removeIsFiles();

		FileManager(const FileManager&) = delete;
		FileManager& operator=(const FileManager&) = delete;
		FileManager(FileManager&&) = delete;
		FileManager& operator=(FileManager&&) = delete;

	private:
		utils::Logger& m_logger;
		const std::string m_dataPath;
		const std::uint16_t m_maxFileSizeMB;

		std::vector<StreamFile> m_lsFiles;
		std::vector<StreamFile> m_vsFiles;
		// std::vector<StreamFile> m_isFiles;

		std::mutex m_mutexLsFiles;
		std::mutex m_mutexVsFiles;
		// std::mutex m_mutexIsFiles;
};

}} // namespace

#endif
