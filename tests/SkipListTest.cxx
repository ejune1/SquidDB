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

	core::SkipList<int> skipList(configuration, logger, true /* primary */);
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
