#include "engine/Table.h"

#include "core/IsolationLevel.h"
#include "core/SkipList.h"
#include "engine/Index.h"
#include "engine/TableIterator.h"
#include "storage/Column.h"
#include "storage/Schema.h"
#include "utils/Configuration.h"
#include "utils/Logger.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace squiddb { namespace engine {

Table::Table(const utils::Configuration& configuration, 
	utils::Logger& logger, 
	const std::string dataPath, 
	const std::string name) : 
	m_configuration(configuration), 
	m_logger(logger), 
	m_dataPath(dataPath), 
	m_name(name), 
	m_schema(logger),
	m_fileManager(logger, dataPath, name, configuration.getMaxFileSizeMB())	{
	m_schemaFinalized = false;
	m_primary = nullptr;
	m_secondary = nullptr;
	m_secondarySize = 0;
}

Table::~Table() {
	if (m_primary != nullptr) {
		delete m_primary;
	}

	if (m_secondarySize > 0) {
		for (int x = 0; x < m_secondarySize; x++) {
			if (m_secondary[x] != nullptr) {
				delete m_secondary[x];
			}
		}

		delete [] m_secondary;
	}
}

void Table::initialize() {
	m_logger.log(utils::Logger::LogLevel::Info, "Table::initialize initializing table " + m_name);
	std::string schemaFilePath = m_dataPath + m_name + ".ss";

	if (std::filesystem::exists(schemaFilePath)) {
		m_logger.log(utils::Logger::LogLevel::Info, "Table::initialize found schema " + schemaFilePath);

		m_schema.read(schemaFilePath);
		m_schemaFinalized = true;
	}

	m_fileManager.initialize();

	createIndexes();
}

void Table::startup() {
	m_logger.log(utils::Logger::LogLevel::Info, "Table::startup starting table " + m_name);
	// TODO
	recover();
}

void Table::recover() {
	m_logger.log(utils::Logger::LogLevel::Warn, "Table::recover recovering table " + m_name);
	// TODO
}

void Table::shutdown() {
	m_logger.log(utils::Logger::LogLevel::Info, "Table::shutdown shutting down table " + m_name);
	// TODO
}

void Table::addColumn(const std::string name, const std::uint16_t size, const std::uint8_t columnType) {
	if (m_schemaFinalized == true) {
		throw std::runtime_error("Table::addColumn schema is finalized");
	}

	storage::Column::ColumnType colType = static_cast<storage::Column::ColumnType>(columnType);
	storage::Column::KeyType keyType = storage::Column::KeyType::None;

	storage::Column column(name, size, colType, keyType);
	m_schema.addColumn(column);
}

void Table::addIndex(const std::string name, const std::uint16_t size, const std::uint8_t columnType, const bool primary) {
	if (m_schemaFinalized == true) {
		throw std::runtime_error("Table::addIndex schema is finalized");
	}

	storage::Column::ColumnType colType = static_cast<storage::Column::ColumnType>(columnType);
	storage::Column::KeyType keyType = (primary == true) ? storage::Column::KeyType::Primary : storage::Column::KeyType::Secondary;

	storage::Column column(name, size, colType, keyType);
	m_schema.addColumn(column);
}

void Table::finalizeSchema() {
	m_logger.log(utils::Logger::LogLevel::Info, "Table::finalizeSchema table " + m_name);
	std::string schemaFilePath = m_dataPath + m_name + ".ss";

	m_schema.write(schemaFilePath);
	m_schemaFinalized = true;

	createIndexes();
}

bool Table::insertRow(void* row) {
	assert((m_schemaFinalized == true) && (m_primary != nullptr));

	std::uint16_t primaryOffset = m_schema.getPrimaryOffset();
	std::uint16_t rowSize = m_schema.getTotalSize();

	std::byte* rowBytes = static_cast<std::byte*>(row);
	void* key = static_cast<void*>(rowBytes + primaryOffset);

	core::ThreadContextManager* threadContextManager = core::ThreadContextManager::getInstance();
	core::ThreadContext* threadContext = threadContextManager->getThreadContext();
	core::Transaction* transaction = threadContext->getTransaction();

	bool implicitTransaction = (transaction == nullptr);
	if (implicitTransaction == true) {
		beginTransaction(threadContextManager);
		transaction = threadContext->getTransaction();
	}

	bool result = m_primary->insertRow(key, row, rowSize, transaction);

	if (implicitTransaction == true) {
		commit(threadContextManager);
	}

	// TODO secondary
	
	return result;
}

bool Table::deleteRow(const void* key) {
	assert((m_schemaFinalized == true) && (m_primary != nullptr));

	core::ThreadContextManager* threadContextManager = core::ThreadContextManager::getInstance();
	core::ThreadContext* threadContext = threadContextManager->getThreadContext();
	core::Transaction* transaction = threadContext->getTransaction();

	bool implicitTransaction = (transaction == nullptr);
	if (implicitTransaction == true) {
		beginTransaction(threadContextManager);
		transaction = threadContext->getTransaction();
	}

	bool result =  m_primary->deleteRow(key, transaction);

	if (implicitTransaction == true) {
		commit(threadContextManager);
	}

	// TODO secondary
	
	return result;
}

bool Table::updateRow(const void* key, void* row) {
	assert((m_schemaFinalized == true) && (m_primary != nullptr));

	std::uint16_t rowSize = m_schema.getTotalSize();

	// TODO this needs to check if the key is changing
	
	core::ThreadContextManager* threadContextManager = core::ThreadContextManager::getInstance();
	core::ThreadContext* threadContext = threadContextManager->getThreadContext();
	core::Transaction* transaction = threadContext->getTransaction();

	bool implicitTransaction = (transaction == nullptr);
	if (implicitTransaction == true) {
		beginTransaction(threadContextManager);
		transaction = threadContext->getTransaction();
	}

	bool result = m_primary->updateRow(key, row, rowSize, transaction);

	if (implicitTransaction == true) {
		commit(threadContextManager);
	}

	// TODO secondary
	
	return result;
}

TableIterator* Table::scan() const {
	// TODO: transactions
	return m_primary->scan();
}

TableIterator* Table::scan(const std::string& /* indexName */) const {
	// TODO transactios, secondary

	return m_primary->scan();
}

TableIterator* Table::rangeScan(const std::string& /* indexName */, const void* startKey, const void* endKey) const {
	// TODO transactions, secondary

	return m_primary->rangeScan(startKey, endKey);
}

void Table::beginTransaction(core::ThreadContextManager* threadContextManager) {
	std::size_t transactionId = threadContextManager->getNextTransactionId();
	std::size_t viewpoint = 0;
	
	if (transactionId > 1) {
		std::size_t minActiveTransactionId = threadContextManager->getMinActiveTransactionId();
		if ((minActiveTransactionId == 0) || (minActiveTransactionId > transactionId)) {
			viewpoint = transactionId - 1;
		} else {
			viewpoint = minActiveTransactionId - 1;
		}
	}
	core::IsolationLevel isolationLevel = core::IsolationLevel::RepeatableRead;
	threadContextManager->getThreadContext()->beginTransaction(transactionId, viewpoint, isolationLevel);
}

void Table::commit(core::ThreadContextManager* threadContextManager) {
	core::ThreadContext* threadContext = threadContextManager->getThreadContext();
	core::Transaction* transaction = threadContext->getTransaction();
	std::size_t transactionId = transaction->getTransactionId();

	commitLogValue();
	commitMemory(transaction);

	threadContext->committed();
	threadContextManager->removeTransactionActive(transactionId);
}

void Table::rollback(core::ThreadContextManager* threadContextManager) {
	core::ThreadContext* threadContext = threadContextManager->getThreadContext();
	core::Transaction* transaction = threadContext->getTransaction();
	std::size_t transactionId = transaction->getTransactionId();

	abortTransaction(transaction);
	
	threadContext->aborted();
	threadContextManager->removeTransactionActive(transactionId);
}

void Table::createIndexes() {
	if (m_schemaFinalized == false) {
		throw std::runtime_error("Table::createIndexes schema not finalized");
	}
	m_logger.log(utils::Logger::LogLevel::Info, "Table::createIndexes create indexes for " + m_name);

	for (const storage::Column& column : m_schema.getColumns()) {
		storage::Column::KeyType keyType = column.getKeyType();

		if (keyType == storage::Column::KeyType::Primary) {
			if (m_primary != nullptr) {
				throw std::runtime_error("Table::createIndexes found multiple primary for " + m_name);
			}

			storage::Column::ColumnType columnType = column.getColumnType();
			switch (columnType) {
				case storage::Column::ColumnType::Int32: {
					std::string logMessage = "Table::createIndexes creating int32 primary index for " + m_name;
					m_logger.log(utils::Logger::LogLevel::Info, logMessage);

					core::SkipList<std::int32_t>* skipList= new core::SkipList<std::int32_t>(m_logger, true /* primaryIndex */, 
						m_configuration.getMaxNodeHeight());
					skipList->initialize();
					m_primary = skipList;
					break;
				}

				case storage::Column::ColumnType::Int64:
				case storage::Column::ColumnType::Float:
				case storage::Column::ColumnType::String:
				case storage::Column::ColumnType::Unknown:
				default: {
					std::string columnTypeString = storage::Column::columnTypeString(columnType);
					throw std::runtime_error("Table::createIndexes index type not supported " + columnTypeString + " for " + m_name);
				}
			}
		}

		// TODO create secondary indexes
	}

	if (m_primary == nullptr) {
		throw std::runtime_error("Table::createIndexes did not find primary index for " + m_name);
	}
}

void Table::commitLogValue() {
	// TODO
	#if 0
	ThreadContext threadContext* = ThreadContextManager::getInstance().getThreadContext();
	Transaction* transaction = threadContext->getTransaction();
	storage::FileManager* fileManager = storage::FileManager::getInstance();

	fileManager.lockLsFiles();
	fileManager.lockVsFiles();

	storage::FileStream* lsFile = fileManager.getCurrentLsFile();
	storage::FileStream* vsFile = fileManager.getCurrentVsFile();
	
	lsFile->lock();
	vsFile->lock();

	std::uint8_t vsIndex = vsFile->getIndex();
	std::size_t vsPosition = vsFile->getPosition();

	// need something to hold the Key and Row in transaction that i can walk here
	for (const KeyRowContainer& keyRow : transaction->getAffected()) {
		Protocol_v1_0::writeLogValue(keyRow, lsFile, vsFile);

		keyRow->rowInfo->setFileIndex(vsIndex);
		keyRow->rowInfo->setFilePosition(vsPosition);
		keyRow->setDirty(true);

		vsPosition = vsFile->getPosition();
	}

	vsFile->unlock();
	lsFile->unlock();

	fileManager.unlockVsFiles();
	fileManager.unlockLsFiles();
	#endif
}

void Table::commitMemory(core::Transaction* transaction) {
	for (core::KeyRowInfo* keyRowInfo : transaction->getAffectedRows()) {
		keyRowInfo->getRowInfo()->setStatus(core::RowInfo::Status::Committed);
	}
}

void Table::abortTransaction(core::Transaction* transaction) {
	for (core::KeyRowInfo* keyRowInfo : transaction->getAffectedRows()) {
		keyRowInfo->getRowInfo()->setStatus(core::RowInfo::Status::Aborted);
	}
}

}} // namespace
