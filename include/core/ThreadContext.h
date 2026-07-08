#ifndef THREADCONTEXT_H
#define THREADCONTEXT_H

#include "core/Transaction.h"

#include <cstdint>

namespace squiddb { namespace core {

class ThreadContext {
	public:
		ThreadContext();
		~ThreadContext();

		Transaction* getTransaction() const;

		void beginTransaction(const std::size_t transactionId);
		void committed();
		void aborted();
	
	private:
		Transaction* m_transaction;
};

}} // namespace

#endif
