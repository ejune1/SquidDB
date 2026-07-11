#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "utils/Logger.h"

#include <cstdint>
#include <string>

namespace squiddb { namespace utils {

class Configuration {
	public:
		enum class ConfigurationKey {
			LogFilePath =   0,
			LogLevel =      1,
			LogMode =       2,
			MaxMemoryMB =   3,
			MaxNodeHeight = 4,
			DataPath =      5,
			MaxFileSizeMB = 6,
			Unknown =       7
		};

		Configuration(Logger& logger);
		~Configuration() = default;

		void read(std::string filename);

		std::string getFilename() const;

		std::string getLogFilePath() const;
		Logger::LogLevel getLogLevel() const;
		Logger::LogMode getLogMode() const;

		std::uint32_t getMaxMemoryMB() const;
		std::uint8_t getMaxNodeHeight() const;

		std::string getDataPath() const;
		std::uint16_t getMaxFileSizeMB() const;

	private:
		std::string configurationKeyString(const ConfigurationKey configurationKey) const;
		ConfigurationKey parseConfigurationKey(const std::string& keyString) const;

		Logger& m_logger; 
		std::string m_filename;

		std::string m_logFilePath;
		Logger::LogLevel m_logLevel;
		Logger::LogMode m_logMode;

		std::uint32_t m_maxMemoryMB;
		std::uint8_t m_maxNodeHeight; 

		std::string m_dataPath;
		std::uint16_t m_maxFileSizeMB;
};

}} // namespace

#endif
