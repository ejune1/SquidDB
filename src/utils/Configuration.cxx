#include "utils/Configuration.h"

#include "utils/Logger.h"
#include "utils/StringUtils.h"

#include <cstdint>
#include <fstream>
#include <string>

namespace squiddb { namespace utils {

Configuration::Configuration(Logger& logger) : m_logger(logger) { }

void Configuration::read(std::string filename) {
	m_filename = filename;

	std::ifstream file(m_filename);
	std::string line;
	int lineNum = 0;

	while (std::getline(file, line)) {
		lineNum++;

		line = StringUtils::trim(line);

		if ((line.empty() == true) || (line[0] == '#')) {
			continue;
		}

		size_t colon = line.find(':');
		if (colon == std::string::npos) {
			m_logger.log(Logger::LogLevel::Warn, "Configuration::read got back config line: " + line);
		}

		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);

		key = StringUtils::trim(key);
		value = StringUtils::trim(value);

		ConfigurationKey configurationKey = parseConfigurationKey(key);

		switch (configurationKey) {
			case ConfigurationKey::LogFilePath:
				m_logFilePath = value;
				m_logger.log(Logger::LogLevel::Info, "Configuration::read got key: LOGFILEPATH value: " + value);
				break;
			case ConfigurationKey::LogLevel:
				m_logLevel = m_logger.parseLogLevel(value);
				m_logger.log(Logger::LogLevel::Info, "Configuration::read got key: LOGLEVEL value: " + value);
				break;
			case ConfigurationKey::LogMode:
				m_logMode = m_logger.parseLogMode(value);
				m_logger.log(Logger::LogLevel::Info, "Configuration::read got key: LOGMODE value: " + value);
				break;
			case ConfigurationKey::MaxMemoryMB:
				m_maxMemoryMB = std::stoi(value);
				m_logger.log(Logger::LogLevel::Info, "Configuration::read got key: MAXMEMORYMB value: " + value);
				break;
			case ConfigurationKey::MaxNodeHeight:
				m_maxNodeHeight = std::stoi(value);
				m_logger.log(Logger::LogLevel::Info, "Configuration::read got key: MAXNODEHEIGHT value: " + value);
				break;
			case ConfigurationKey::Unknown:
				m_logger.log(Logger::LogLevel::Warn, "Configuration::read got bad config key: " + key);
				break;
		}
	}
}

std::string Configuration::getFilename() const {
	return m_filename;
}

std::string Configuration::getLogFilePath() const {
	return m_logFilePath;
}

Logger::LogLevel Configuration::getLogLevel() const {
	return m_logLevel;
}

Logger::LogMode Configuration::getLogMode() const {
	return m_logMode;
}

unsigned int Configuration::getMaxMemoryMB() const {
	return m_maxMemoryMB;
}

std::uint8_t Configuration::getMaxNodeHeight() const {
	return m_maxNodeHeight;
}

std::string Configuration::configurationKeyString(const ConfigurationKey configurationKey) const {
	switch (configurationKey) {
		case ConfigurationKey::LogFilePath:   return "LOGFILEPATH";
		case ConfigurationKey::LogLevel:      return "LOGLEVEL";
		case ConfigurationKey::LogMode:       return "LOGMODE";
		case ConfigurationKey::MaxMemoryMB:   return "MAXMEMORYMB";
		case ConfigurationKey::MaxNodeHeight: return "MAXNODEHEIGHT";
		case ConfigurationKey::Unknown:       return "UNKNOWN";
	}
	return "UNKNOWN";
}

Configuration::ConfigurationKey Configuration::parseConfigurationKey(const std::string& keyString) const {
	std::string keyStringUpper = StringUtils::toUpper(keyString);

	if (keyStringUpper == "LOGFILEPATH") {
		return ConfigurationKey::LogFilePath;
	}

	if (keyStringUpper == "LOGLEVEL") {
		return ConfigurationKey::LogLevel;
	}

	if (keyStringUpper == "LOGMODE") {
		return ConfigurationKey::LogMode;
	}

	if (keyStringUpper == "MAXMEMORYMB") {
		return ConfigurationKey::MaxMemoryMB;
	}

	if (keyStringUpper == "MAXNODEHEIGHT") {
		return ConfigurationKey::MaxNodeHeight;
	}

	return ConfigurationKey::Unknown;
}

}} // namespace
