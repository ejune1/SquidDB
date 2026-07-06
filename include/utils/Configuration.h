#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "utils/Logger.h"

#include <cstdint>
#include <string>

namespace squiddb { namespace utils {

class Configuration {
	public:
		enum class ConfigurationKey {
			LogFilePath = 0,
			LogLevel = 1,
			LogMode = 2,
			MaxMemoryMB = 3,
			MaxNodeHeight = 4,
			DataPath = 5,
			Unknown = 6
		};

		Configuration(Logger& logger);
		~Configuration() = default;

		void read(std::string filename);

		std::string getFilename() const;

		std::string getLogFilePath() const;
		Logger::LogLevel getLogLevel() const;
		Logger::LogMode getLogMode() const;

		unsigned int getMaxMemoryMB() const;
		std::uint8_t getMaxNodeHeight() const;

		std::string getDataPath() const;

	private:
		std::string configurationKeyString(const ConfigurationKey configurationKey) const;
		ConfigurationKey parseConfigurationKey(const std::string& keyString) const;

		Logger& m_logger; 
		std::string m_filename;

		std::string m_logFilePath;
		Logger::LogLevel m_logLevel;
		Logger::LogMode m_logMode;

		unsigned int m_maxMemoryMB;
		std::uint8_t m_maxNodeHeight; 

		std::string m_dataPath;
};

}} // namespace

#endif
