#include "core/Transaction.h"

#include "core/IsolationLevel.h"
#include "core/KeyRowInfo.h"

#include <cstdint>
#include <vector>

namespace squiddb { namespace core {

Transaction::Transaction(const std::size_t transactionId, const std::size_t viewpoint, const IsolationLevel isolationLevel) : 
	m_transactionId(transactionId), m_viewpoint(viewpoint), m_isolationLevel(isolationLevel) { }

std::size_t Transaction::getTransactionId() const {
	return m_transactionId;
}

std::size_t Transaction::getViewpoint() const {
	return m_viewpoint;
}

IsolationLevel Transaction::getIsolationLevel() const {
	return m_isolationLevel;
}

std::vector<KeyRowInfo*> Transaction::getAffectedRows() const {
	return m_keyRowInfo;
}

void Transaction::addAffectedRow(KeyRowInfo* keyRowInfo) {
	m_keyRowInfo.push_back(keyRowInfo);
}

}} // namespace
