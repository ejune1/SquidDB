#include "test/catch.hpp"

#include "core/SkipList.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <algorithm>
#include <random>
#include <vector>

using namespace squiddb;

TEST_CASE("SkipList<int> simple inserts, 1 level with size and contains checking", "[skiplist][insert]") {
	// runs before each section
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 1 /* maxNodeHeight */);
	skipList.initialize();

	// verify empty list is valid
	REQUIRE(skipList.validate() == true);

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

	REQUIRE(skipList.validate() == true);

	// teardown happens after each section
	logger.stop();
}

TEST_CASE("SkipList<int> simple inserts, constant 2 levels with contains checking", "[skiplist][insert]") {
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

	REQUIRE(skipList.validate() == true);

	logger.stop();
}

TEST_CASE("SkipList<int> inserts, max 2 levels with contains checking", "[skiplist][insert]") {
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

	REQUIRE(skipList.validate() == true);

	logger.stop();
}

TEST_CASE("SkipList<int> inserts, max 5 levels with contains checking", "[skiplist][insert]") {
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

	REQUIRE(skipList.validate() == true);

	logger.stop();
}

TEST_CASE("SkipList<int> inserts decreasing order, max 5 levels with contains checking", "[skiplist][insert]") {
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 5 /* maxNodeHeight */);
	skipList.initialize();

	for (int x = 99; x >= 0; x--) {
		skipList.insert(x /* key */, nullptr /* data */, 0 /* size */);
	}

	REQUIRE(skipList.size() == 100);
	REQUIRE(skipList.size(true /* calculate */) == 100);
	REQUIRE(skipList.empty() == false);
	
	for (int x = 99; x >= 0; x--) {
		REQUIRE(skipList.contains(x) == true);
	}

	REQUIRE(skipList.validate() == true);

	logger.stop();
}

TEST_CASE("SkipList<int> inserts random, max 5 levels with contains checking", "[skiplist][insert]") {
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 5 /* maxNodeHeight */);
	skipList.initialize();

	int nums[100] = {0};
	std::minstd_rand0 gen(std::random_device{}());
	std::uniform_int_distribution<int> dist(0, 99);

	for (int x = 0; x < 100; x++) {
		int key = dist(gen);
		skipList.insert(key /* key */, nullptr /* data */, 0 /* size */);
		nums[x] = key;
	}

	REQUIRE(skipList.empty() == false);
	
	for (int x = 0; x < 100; x++) {
		int key = nums[x];
		REQUIRE(skipList.contains(key) == true);
	}

	REQUIRE(skipList.validate() == true);

	logger.stop();
}

TEST_CASE("SkipList<int> simple removes, 1 level with size and contains checking", "[skiplist][remove]") {
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
	REQUIRE(skipList.validate() == true);

	result = skipList.remove(1);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 3);
	REQUIRE(skipList.contains(1) == false);
	REQUIRE(skipList.validate() == true);

	result = skipList.remove(2);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 2);
	REQUIRE(skipList.contains(2) == false);
	REQUIRE(skipList.validate() == true);

	result = skipList.remove(3);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 1);
	REQUIRE(skipList.contains(3) == false);
	REQUIRE(skipList.validate() == true);

	result = skipList.remove(4);
	REQUIRE(result == true);
	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.contains(4) == false);
	REQUIRE(skipList.validate() == true);

	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.size(true /* calculate */) == 0);

	logger.stop();
}

TEST_CASE("SkipList<int> removes, max 5 levels with contains checking", "[skiplist][remove]") {
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

	REQUIRE(skipList.validate() == true);
	
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

	REQUIRE(skipList.validate() == true);

	REQUIRE(skipList.size() == 0);
	REQUIRE(skipList.size(true /* calculate */) == 0);
	REQUIRE(skipList.empty() == true);

	logger.stop();
}

TEST_CASE("SkipList<int> mixed random, max 10 levels with contains checking", "[skiplist][mixed]") {
	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	core::SkipList<int> skipList(logger, true /* primary */, 10 /* maxNodeHeight */);
	skipList.initialize();

	std::vector<int> nums;
		
	std::minstd_rand0 gen(std::random_device{}());
	std::uniform_int_distribution<int> dist4(0, 3);
	std::uniform_int_distribution<int> dist10000(0, 9999);

	for (int x = 0; x < 1000; x++) {
		int key = 0;
		while (true) {
			key = dist10000(gen);
			auto it = std::find(nums.begin(), nums.end(), key);
			if (it == nums.end()) {
				break;
			}
		}

		skipList.insert(key, nullptr /* data */, 0 /* size */);
		nums.push_back(key);

		// 1 in 5 chance to remove
		if (dist4(gen) == 0) {
			std::uniform_int_distribution<int> distNums(0, nums.size() - 1);
			int removeIndex = distNums(gen);
			int num = nums[removeIndex];

			bool removed = skipList.remove(num);
			REQUIRE(removed == true);
			nums.erase(nums.begin() + removeIndex);
		}
	}

	REQUIRE(skipList.empty() == false);
	
	for (long unsigned int x = 0; x < nums.size(); x++) {
		int key = nums[x];
		REQUIRE(skipList.contains(key) == true);
	}

	REQUIRE(skipList.validate() == true);

	logger.stop();
}
