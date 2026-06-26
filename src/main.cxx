#include "core/SkipList.h"
#include "infra/Queue.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <string>
#include <thread>
#include <unistd.h>

using namespace squiddb;

void signalHandler(int sigNum) {
	fprintf(stderr, "main::signalHandler got signal %d\n", sigNum);
}

// accept binary only or binary configFilePath
int main(int argc, char* argv[]) {
	std::signal(SIGINT, signalHandler);
	std::signal(SIGTERM, signalHandler);

	// block SIGINT and SIGTERM
	sigset_t signalsToBlock;
	sigemptyset(&signalsToBlock);
	sigaddset(&signalsToBlock, SIGINT);
	sigaddset(&signalsToBlock, SIGTERM);
	sigprocmask(SIG_BLOCK, &signalsToBlock, nullptr);

	// create logger and configuration for dependency injection
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration config(logger);

	std::string configFilePath;
	if (argc > 1) {
		configFilePath = argv[1];
	} else {
		configFilePath = "./squiddb.conf";
	}

	config.read(configFilePath);

	logger.setLogLevel(config.getLogLevel());
	logger.setOutputMode(config.getLogMode(), config.getLogFilePath());

	logger.start();
	logger.log(utils::Logger::LogLevel::Info, "welcome to squid");

	// test some stuff
	core::SkipList<int> skipList(config, logger, true /* primary */);
	skipList.insert(1, nullptr, 0);

	// wait for only SIGINT or SIGTERM
	sigset_t waitMask;
	sigfillset(&waitMask);
	sigdelset(&waitMask, SIGINT);
	sigdelset(&waitMask, SIGTERM);

	// go to sleep
	sigsuspend(&waitMask);

	// woken up by signal
	logger.log(utils::Logger::LogLevel::Info, "shutting down");
	logger.stop();

	return 0;
}
