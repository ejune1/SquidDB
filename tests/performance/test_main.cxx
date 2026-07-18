#include "engine/Table.h"
#include "engine/TableIterator.h"
#include "utils/Configuration.h"
#include "utils/FileCleanup.h"
#include "utils/Logger.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace squiddb;

void insertPerformance();

int main() {
	fprintf(stderr, "Running performance test suite..\n");
	insertPerformance();
}

void insertPerformance() {
	fprintf(stderr, "Insert performance test..\n");

	utils::FileCleanup fileCleanup("./data/table1.ss");

	utils::Logger& logger = utils::Logger::getInstance();
	utils::Configuration configuration(logger);
	configuration.read("./squiddb.conf");

	logger.setLogLevel(utils::Logger::LogLevel::Warn);
	logger.setOutputMode(configuration.getLogMode(), configuration.getLogFilePath());
	logger.start();

	engine::Table table1(configuration, logger, configuration.getDataPath(), "table1");
	table1.addIndex("index1", sizeof(std::int32_t), 0 /* col type int32 */, 1 /* key type primary */);
	table1.finalizeSchema();

	fprintf(stderr, "Inserting 1,000,000 rows..\n");

	for (std::int32_t x = 0; x < 1000000; x++) {
		std::int32_t* row = static_cast<std::int32_t*>(std::malloc(sizeof(std::int32_t)));
		memcpy(row, &x, sizeof(std::int32_t));

		table1.insertRow(row);

		if (x % 10000 == 0) {
			fprintf(stderr, "Inserted %d rows..\n", x);
		}
	}
}
