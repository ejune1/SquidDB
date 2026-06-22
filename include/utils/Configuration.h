#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>

namespace squiddb { namespace utils {

class Configuration {
	public:
		enum class ConfigurationKey {
			logFilePath = 0,
			maxMemoryMB = 1	
		};

		void read(std::string filename);

		Configuration() = default;
		~Configuration() = default;

	private:
		std::string configurationKeyString(const ConfigurationKey configurationKey) const;

		std::string m_filename;

		std::string m_logFilePath;
		unsigned int m_maxMemoryMB;
};

}} // namespace

#endif
