#include "core/ThreadContextManager.h"

#include "core/ThreadContext.h"

#include <cstdint>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <unordered_map>

namespace squiddb { namespace core {

ThreadContextManager::ThreadContextManager() { }

ThreadContextManager& ThreadContextManager::getInstance() {
	static ThreadContextManager instance;
	
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
			std::unique_lock<std::shared_mutex> writeLock(m_mutex);
			m_context[threadId] = &threadContext;
		}
		initialized = true;
	}

	return &threadContext;
}

bool ThreadContextManager::transactionInProgress(const std::size_t transactionId) const {
	// TODO keep another map with key transactionId so we can look this up fast
	bool found = false;
	{
		std::shared_lock<std::shared_mutex> readLock(m_mutex);

		for (std::pair<const std::thread::id, ThreadContext*> pair : m_context) {
			Transaction* transaction = pair.second->getTransaction();

			if (transaction != nullptr) {
				if (transaction->getTransactionId() == transactionId) {
					found = true;
					break;
				}
			}
		}
	}

	return found;
}

}} // namespace
