#ifndef THREADCONTEXT_H
#define THREADCONTEXT_H

#include "core/Transaction.h"

namespace squiddb { namespace core {

class ThreadContext {
	public:
		ThreadContext();
		~ThreadContext();

		Transaction* getTransaction() const;
		void beginTransaction();
		void committed();
	
	private:
		Transaction* m_transaction;
};

}} // namespace

#endif
