#include "test/catch.hpp"

#include "core/SkipList.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

using namespace squiddb;

TEST_CASE("SkipList<int> simple inserts, 1 level with size and contains checking", "[skiplist]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 1 /* maxNodeHeight */);
	skipList.initialize();

	//SECTION("new SkipList should be empty") {
	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.empty() == true);
	//}

	//SECTION("inserting one key at level 1") {
	skipList.insert(0 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	REQUIRE(skipList.size() == 1);
	REQUIRE(skipList.size(true /* calculate */) == 1);
	REQUIRE(skipList.empty() == false);
	REQUIRE(skipList.contains(0) == true);
	//}

	skipList.insert(1 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(2 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(3 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(4 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);

	REQUIRE(skipList.size() == 5);
	REQUIRE(skipList.size(true /* calculate */) == 5);
	REQUIRE(skipList.empty() == false);
	
	REQUIRE(skipList.contains(0) == true);
	REQUIRE(skipList.contains(1) == true);
	REQUIRE(skipList.contains(2) == true);
	REQUIRE(skipList.contains(3) == true);
	REQUIRE(skipList.contains(4) == true);

	// teardown happens after each section
	logger.stop();
}

TEST_CASE("SkipList<int> simple inserts, constant 2 levels with contains checking", "[skiplist]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 2 /* maxNodeHeight */);
	skipList.initialize();

	for (int x = -50; x < 50; x++) {
		skipList.insert(x /* key */, nullptr /* data */, 0 /* size */, 2 /* nodeHeight */);
	}

	REQUIRE(skipList.size() == 100);
	REQUIRE(skipList.size(true /* calculate */) == 100);
	REQUIRE(skipList.empty() == false);
	
	for (int x = -50; x < 50; x++) {
		REQUIRE(skipList.contains(x) == true);
	}

	// teardown happens after each section
	logger.stop();
}

TEST_CASE("SkipList<int> inserts, max 2 levels with contains checking", "[skiplist]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 2 /* maxNodeHeight */);
	skipList.initialize();

	for (int x = -50; x < 50; x++) {
		skipList.insert(x /* key */, nullptr /* data */, 0 /* size */);
	}

	REQUIRE(skipList.size() == 100);
	REQUIRE(skipList.size(true /* calculate */) == 100);
	REQUIRE(skipList.empty() == false);
	
	for (int x = -50; x < 50; x++) {
		REQUIRE(skipList.contains(x) == true);
	}

	// teardown happens after each section
	logger.stop();
}

TEST_CASE("SkipList<int> inserts, max 5 levels with contains checking", "[skiplist]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 5 /* maxNodeHeight */);
	skipList.initialize();

	for (int x = 0; x < 100; x++) {
		skipList.insert(x /* key */, nullptr /* data */, 0 /* size */);
	}

	REQUIRE(skipList.size() == 100);
	REQUIRE(skipList.size(true /* calculate */) == 100);
	REQUIRE(skipList.empty() == false);
	
	for (int x = 0; x < 100; x++) {
		REQUIRE(skipList.contains(x) == true);
	}

	// teardown happens after each section
	logger.stop();
}

TEST_CASE("SkipList<int> simple removes, 1 level with size and contains checking", "[skiplist]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 1 /* maxNodeHeight */);
	skipList.initialize();

	skipList.insert(0 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(1 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(2 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(3 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);
	skipList.insert(4 /* key */, nullptr /* data */, 0 /* size */, 1 /* nodeHeight */);

	REQUIRE(skipList.size() == 5);
	REQUIRE(skipList.size(true /* calculate */) == 5);
	REQUIRE(skipList.empty() == false);
	
	REQUIRE(skipList.contains(0) == true);
	REQUIRE(skipList.contains(1) == true);
	REQUIRE(skipList.contains(2) == true);
	REQUIRE(skipList.contains(3) == true);
	REQUIRE(skipList.contains(4) == true);

	bool result = skipList.remove(0);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 4);
	REQUIRE(skipList.contains(0) == false);

	result = skipList.remove(1);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 3);
	REQUIRE(skipList.contains(1) == false);

	result = skipList.remove(2);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 2);
	REQUIRE(skipList.contains(2) == false);

	result = skipList.remove(3);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 1);
	REQUIRE(skipList.contains(3) == false);

	result = skipList.remove(4);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.contains(4) == false);

	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.size(true /* calculate */) == 0);

	// teardown happens after each section
	logger.stop();
}

TEST_CASE("SkipList<int> removes, max 5 levels with contains checking", "[skiplist]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 5 /* maxNodeHeight */);
	skipList.initialize();

	for (int x = 0; x < 100; x++) {
		skipList.insert(x /* key */, nullptr /* data */, 0 /* size */);
	}

	// remove every other key
	for (int x = 0; x < 100; x++) {
		if (x % 2 == 0) {
			bool result = skipList.remove(x);
			REQUIRE(result == true);
		}
	}
	
	for (int x = 0; x < 100; x++) {
		bool result = skipList.contains(x);
		if (x % 2 == 0) {
			REQUIRE(result == false);
		} else {
			REQUIRE(result == true);
		}
	}

	//remove the rest of them
	for (int x = 0; x < 100; x++) {
		if (x % 2 != 0) {
			bool result = skipList.remove(x);
			REQUIRE(result == true);
		}
	}

	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.size(true /* calculate */) == 0);
	REQUIRE(skipList.empty() == true);

	// teardown happens after each section
	logger.stop();
}
