#include "core/ThreadContext.h"

#include "core/Transaction.h"

#include <cstdint>
#include <stdexcept>

namespace squiddb { namespace core {

ThreadContext::ThreadContext() {
	m_transaction = nullptr;
}

ThreadContext::~ThreadContext() {
	if (m_transaction != nullptr) {
		delete m_transaction;
	}
}

Transaction* ThreadContext::getTransaction() const {
	return m_transaction;
}

void ThreadContext::beginTransaction() {
	if (m_transaction != nullptr) {
		throw std::runtime_error("ThreadContext::beginTransaction transaction is not null");
	}

	// TODO something needs to manage these Ids
	std::size_t transactionId = 0;

	m_transaction = new Transaction(transactionId);
}

void ThreadContext::committed() {
	if (m_transaction == nullptr) {
		throw std::runtime_error("ThreadContext::committed transaction is null");
	}

	delete m_transaction;
	m_transaction = nullptr;
}

}} // namespace
