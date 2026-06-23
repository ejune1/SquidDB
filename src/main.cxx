#include "infra/Queue.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <chrono>
#include <cstdio>
#include <thread>

using namespace squiddb;

int main(int argc, char* argv[]) {
	// create logger and configuration for dependency injection
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration config(logger);
	config.read("./squiddb.conf");

	logger.setLogLevel(config.getLogLevel());
	logger.setOutputMode(config.getLogMode(), config.getLogFilePath());

	logger.start();

	logger.log(utils::Logger::LogLevel::Info, "welcome to squid");

	std::this_thread::sleep_for(std::chrono::seconds(3));

	logger.log(utils::Logger::LogLevel::Info, "shutting down");
	logger.stop();

	std::this_thread::sleep_for(std::chrono::seconds(3));

	return 0;
}
