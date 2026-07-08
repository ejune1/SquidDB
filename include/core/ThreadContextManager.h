#ifndef THREADCONTEXTMANAGER_H
#define THREADCONTEXTMANAGER_H

#include "core/ThreadContext.h"
#include "utils/Logger.h"

#include <cstdint>
#include <mutex>
#include <thread>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace squiddb { namespace core {

class ThreadContextManager {
	public:
		~ThreadContextManager() = default;

		static ThreadContextManager& getInstance();

		ThreadContext* getThreadContext();

		void addTransactionActive(const std::size_t transactionId);
		void removeTransactionActive(const std::size_t transactionId);
		bool transactionActive(const std::size_t transactionId) const;

		// singleton
		ThreadContextManager(const ThreadContextManager&) = delete;
		ThreadContextManager& operator=(ThreadContextManager&) = delete;
		ThreadContextManager(ThreadContextManager&&) = delete;
		ThreadContextManager& operator=(ThreadContextManager&&) = delete;

	private:
		ThreadContextManager(utils::Logger& logger);

		struct ThreadCleanup {
			ThreadContextManager* m_managerInstance;
			std::thread::id m_threadId;

			ThreadCleanup(ThreadContextManager* managerInstance, std::thread::id threadId) :
		       		m_managerInstance(managerInstance), m_threadId(threadId) { }
			
			~ThreadCleanup() {
				std::unique_lock<std::shared_mutex> writeLock(m_managerInstance->m_contextMutex);
				m_managerInstance->m_context.erase(m_threadId);
			}
		};

		utils::Logger& m_logger;

		mutable std::shared_mutex m_contextMutex;
		std::unordered_map<std::thread::id, ThreadContext*> m_context;

		mutable std::shared_mutex m_transactionIdMutex;
		std::unordered_set<std::size_t> m_transactionId;
};

}} // namespace

#endif
