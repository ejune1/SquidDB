#ifndef TABLE_H
#define TABLE_H

#include "core/ThreadContext.h"
#include "core/ThreadContextManager.h"
#include "engine/Index.h"
#include "engine/TableIterator.h"
#include "storage/FileManager.h"
#include "storage/Schema.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <cstdint>
#include <string>

namespace squiddb { namespace engine {

class Table {
	public:
		Table(const utils::Configuration& configuration, 
			utils::Logger& logger, 
			const std::string dataPath, 
			const std::string name);

		~Table();

		// read schema from disk if it exists, initialize files, and startup or recover
		void initialize();
		// startup is currently recover only
		void startup();
		void recover();
		void shutdown();

		void addColumn(const std::string name, const std::uint16_t size, const std::uint8_t columnType);
		void addIndex(const std::string name, const std::uint16_t size, const std::uint8_t columnType, const bool primary);
		// causes schema write
		void finalizeSchema();

		// force read keys from row
		bool insertRow(void* row);
		bool deleteRow(const void* key);
		bool updateRow(const void* key, void* row);

		// uses primary
		TableIterator* scan();
		TableIterator* scan(const std::string& indexName);
		TableIterator* rangeScan(const std::string& indexName, const void* startKey, const void* endKey);

		// must be called after using iterator
		void destroyTableIterator(TableIterator* tableIterator);

		void beginTransaction(core::ThreadContextManager* threadContextManager);
		void commit(core::ThreadContextManager* threadContextManager);
		void rollback(core::ThreadContextManager* threadContextManager);

	private:
		void createIndexes();

		void commitLogValue();
		void commitMemory(core::Transaction* transaction);
		void abortTransaction(core::Transaction* transaction);

		const utils::Configuration& m_configuration;
		utils::Logger& m_logger;

		const std::string m_dataPath;
		const std::string m_name;
		storage::Schema m_schema;
		bool m_schemaFinalized;
		
		storage::FileManager m_fileManager;

		Index* m_primary;
		Index** m_secondary;
		std::uint8_t m_secondarySize;
};

}} // namespace

#endif
