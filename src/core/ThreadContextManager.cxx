#include "core/ThreadContextManager.h"

#include "core/ThreadContext.h"
#include "utils/Logger.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <limits>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace squiddb { namespace core {

ThreadContextManager::ThreadContextManager(utils::Logger& logger) : m_logger(logger) {
	m_currentTransactionId = 0;
}

ThreadContextManager* ThreadContextManager::getInstance() {
	static ThreadContextManager instance(utils::Logger::getInstance());
	
	return &instance;
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

std::size_t ThreadContextManager::getNextTransactionId() {
	std::pair<std::unordered_set<std::size_t>::iterator, bool> result;
	std::size_t nextTransactionId = 0;

	{
		std::unique_lock<std::shared_mutex> writeLock(m_transactionIdMutex);
		m_currentTransactionId += 1;
		nextTransactionId = m_currentTransactionId;

		result = m_transactionId.insert(nextTransactionId);
	}

	if (result.second == false) {
		std::string logMessage = "ThreadContextManager::getNextTransactionId transaction already active";
		m_logger.log(utils::Logger::LogLevel::Warn, logMessage);
	}

	return nextTransactionId;
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

std::size_t ThreadContextManager::getMinActiveTransactionId() const {
	std::size_t minimum = 0;
	{
		std::shared_lock<std::shared_mutex> readLock(m_transactionIdMutex);
		if (m_transactionId.empty() != true) {
			std::unordered_set<std::size_t>::const_iterator minIt = std::min_element(m_transactionId.begin(), m_transactionId.end());
			minimum = *minIt;
		}
	}

	return minimum;
}

}} // namespace
