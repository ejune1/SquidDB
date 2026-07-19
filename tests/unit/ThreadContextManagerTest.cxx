#include "test/catch.hpp"

#include "core/ThreadContextManager.h"

#include <cstdint>
#include <random>
#include <set>

using namespace squiddb;

TEST_CASE("thread context manager active transaction Ids and min active transaction Id", "[thread context manager]") {
	utils::Logger& logger = utils::Logger::getInstance();
	logger.setLogLevel(utils::Logger::LogLevel::Info);
	logger.setOutputMode(utils::Logger::LogMode::Terminal, "" /* logFilePath */);
	logger.start();

	std::random_device randomDevice;
	std::mt19937 gen(randomDevice());
	std::uniform_int_distribution<int> distributionBit(0, 1);

	core::ThreadContextManager* threadContextManager = core::ThreadContextManager::getInstance();
	std::set<std::size_t> transactionIds;

	std::size_t minTransactionId = threadContextManager->getMinActiveTransactionId();
	REQUIRE(minTransactionId == 0);

	for (int x = 0; x < 100; x++) {
		std::size_t transactionId = threadContextManager->getNextTransactionId();
		transactionIds.insert(transactionId);

		bool containsTransactionId = threadContextManager->transactionActive(transactionId);
		REQUIRE(containsTransactionId == true);

		// random remove a transaction with 50% chance
		int randomBit = distributionBit(gen);
		if (randomBit == 0) {
			std::uniform_int_distribution<int> distributionSize(0, transactionIds.size() - 1);
			int index = distributionSize(gen);

			auto it = std::next(transactionIds.begin(), index);
			std::size_t removedTransactionId = *it;

			transactionIds.erase(it);

			threadContextManager->removeTransactionActive(removedTransactionId);
			containsTransactionId = threadContextManager->transactionActive(removedTransactionId);

			REQUIRE(containsTransactionId == false);
		}

		std::size_t minId = *transactionIds.begin();
		minTransactionId = threadContextManager->getMinActiveTransactionId();

		REQUIRE(minId == minTransactionId);
	}

	for (std::size_t transactionId : transactionIds) {
		bool containsTransactionId = threadContextManager->transactionActive(transactionId);
		REQUIRE(containsTransactionId == true);
	}

	while (transactionIds.empty() == false) {
		auto node = transactionIds.extract(transactionIds.begin());
		std::size_t transactionId = node.value();

		threadContextManager->removeTransactionActive(transactionId);
		bool containsTransactionId = threadContextManager->transactionActive(transactionId);

		REQUIRE(containsTransactionId == false);

		std::size_t minId = *transactionIds.begin();
		minTransactionId = threadContextManager->getMinActiveTransactionId();

		REQUIRE(minId == minTransactionId);
	}

	minTransactionId = threadContextManager->getMinActiveTransactionId();
	REQUIRE(minTransactionId == 0);

	logger.stop();
}
