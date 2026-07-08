#include "core/Transaction.h"

#include "core/KeyRowInfo.h"

#include <cstdint>
#include <vector>

namespace squiddb { namespace core {

Transaction::Transaction(const std::size_t transactionId) : m_transactionId(transactionId) { }

std::size_t Transaction::getTransactionId() const {
	return m_transactionId;
}

std::vector<KeyRowInfo*> Transaction::getAffectedRows() const {
	return m_keyRowInfo;
}

void Transaction::addAffectedRow(KeyRowInfo* keyRowInfo) {
	m_keyRowInfo.push_back(keyRowInfo);
}

}} // namespace
