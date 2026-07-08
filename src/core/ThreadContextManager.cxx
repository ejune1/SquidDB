#include "core/ThreadContextManager.h"

#include "core/ThreadContext.h"
#include "utils/Logger.h"

#include <cstdint>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace squiddb { namespace core {

ThreadContextManager::ThreadContextManager(utils::Logger& logger) : m_logger(logger) { }

ThreadContextManager& ThreadContextManager::getInstance() {
	static ThreadContextManager instance(utils::Logger::getInstance());
	
	return instance;
}

ThreadContext* ThreadContextManager::getThreadContext() {
	// only lock m_context on thread creation and destroy
	thread_local ThreadContext threadContext;
	thread_local bool initialized = false;

	thread_local std::thread::id threadId = std::this_thread::get_id();
	thread_local ThreadCleanup threadCleanup(this, threadId);

	if (initialized == false) {
		{
			std::unique_lock<std::shared_mutex> writeLock(m_contextMutex);
			m_context[threadId] = &threadContext;
		}
		initialized = true;
	}

	return &threadContext;
}

void ThreadContextManager::addTransactionActive(const std::size_t transactionId) {
	std::pair<std::unordered_set<std::size_t>::iterator, bool> result;

	{
		std::unique_lock<std::shared_mutex> writeLock(m_transactionIdMutex);
		result = m_transactionId.insert(transactionId);
	}

	if (result.second == false) {
		std::string logMessage = "ThreadContextManager::addTransactionActive transaction already active";
		m_logger.log(utils::Logger::LogLevel::Warn, logMessage);
	}
}

void ThreadContextManager::removeTransactionActive(const std::size_t transactionId) {
	std::size_t found = 0;

	{
		std::unique_lock<std::shared_mutex> writeLock(m_transactionIdMutex);
		found = m_transactionId.erase(transactionId);
	}

	if (found == 0) {
		std::string logMessage = "ThreadContextManager::removeTransactionActive transaction not active";
		m_logger.log(utils::Logger::LogLevel::Warn, logMessage);
	}
}

bool ThreadContextManager::transactionActive(const std::size_t transactionId) const {
	bool found = false;
	{
		std::shared_lock<std::shared_mutex> readLock(m_transactionIdMutex);
		found = (m_transactionId.find(transactionId) != m_transactionId.end());
	}

	return found;
}

}} // namespace
