#include "utils/Logger.h"
#include "infra/Queue.h"

#include <chrono>
#include <cstdio>
#include <thread>

int main(int argc, char* argv[]) {
	squiddb::utils::Logger& logger = squiddb::utils::Logger::getInstance();
	//logger.setOutputMode(squiddb::utils::Logger::LogMode::File, "./squiddb.log");
	logger.setOutputMode(squiddb::utils::Logger::LogMode::Terminal);

	logger.start();

	logger.log(squiddb::utils::Logger::LogLevel::Info, "welcome to squid");

	std::this_thread::sleep_for(std::chrono::seconds(3));

	logger.log(squiddb::utils::Logger::LogLevel::Info, "shutting down");
	logger.stop();

	std::this_thread::sleep_for(std::chrono::seconds(3));

	return 0;
}
