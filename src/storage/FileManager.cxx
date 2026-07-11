#include "storage/FileManager.h"

#include "storage/StreamFile.h"
#include "utils/Logger.h"

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

namespace squiddb { namespace storage {

FileManager::FileManager(utils::Logger& logger, const std::string dataPath, const std::uint16_t maxFileSizeMB) :
	m_logger(logger), m_dataPath(dataPath), m_maxFileSizeMB(maxFileSizeMB) { }

}} // namespace
