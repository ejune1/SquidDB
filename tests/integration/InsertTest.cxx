#include "test/catch.hpp"

#include "engine/Table.h"
#include "engine/TableIterator.h"
#include "utils/Configuration.h"
#include "utils/FileCleanup.h"
#include "utils/Logger.h"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

using namespace squiddb;

TEST_CASE("10000 insert test", "[insert]") {
	utils::FileCleanup fileCleanup("./data/table1.ss");

	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(configuration.getLogLevel());
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	engine::Table table1(configuration, logger, configuration.getDataPath(), "table1");
	table1.addIndex("index1", sizeof(std::int32_t), 0 /* col type int32 */, 1 /* key type primary */);
	table1.finalizeSchema();

	for (std::int32_t x = 0; x < 10000; x++) {
		std::int32_t* row = static_cast<std::int32_t*>(std::malloc(sizeof(std::int32_t)));
		memcpy(row, &x, sizeof(std::int32_t));

		bool result = table1.insertRow(row);
		REQUIRE(result == true);
	}

	engine::TableIterator* it = table1.scan();

	for (std::int32_t x = 0; x < 10000; x++) {
		REQUIRE(it->valid() == true);

		const void* key = it->getKey();
		const void* row = it->getData();

		int result = memcmp(key, &x, sizeof(std::int32_t));
		REQUIRE(result == 0);

		result = memcmp(row, &x, sizeof(std::int32_t));
		REQUIRE(result == 0);

		it->next();
	}

	REQUIRE(it->valid() == false);

	delete it;

	table1.shutdown();
	logger.stop();
}
