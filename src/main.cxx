#include "core/SkipList.h"
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
	utils::Configuration configuration(logger);

	std::string configurationFilePath;
	if (argc > 1) {
		configurationFilePath = argv[1];
	} else {
		configurationFilePath = "./squiddb.conf";
	}

	configuration.read(configurationFilePath);

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());

	logger.start();
	logger.log(utils::Logger::LogLevel::Info, "welcome to squid");

	// test some stuff
	core::SkipList<int> skipList(configuration, logger, true /* primary */);
	skipList.initialize();

	skipList.insert(1 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);

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
