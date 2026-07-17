#include "core/Transaction.h"

#include "core/IsolationLevel.h"
#include "core/KeyRowInfo.h"
#include "core/RowInfo.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace squiddb { namespace core {

Transaction::Transaction(const std::size_t transactionId, const std::size_t viewpoint, const IsolationLevel isolationLevel) : 
	m_transactionId(transactionId), m_viewpoint(viewpoint), m_isolationLevel(isolationLevel) {
	m_implicit = false;
}

Transaction::~Transaction() {
	for (KeyRowInfo* keyRowInfo : m_keyRowInfo) {
		delete keyRowInfo;
	}
	m_keyRowInfo.clear();
}

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

void Transaction::addAffectedRow(const std::byte* key, const std::uint16_t keySize, RowInfo* rowInfo) {
	KeyRowInfo* keyRowInfo = new KeyRowInfo(key, keySize, rowInfo);
	m_keyRowInfo.push_back(keyRowInfo);
}

void Transaction::addAffectedRow(KeyRowInfo* keyRowInfo) {
	m_keyRowInfo.push_back(keyRowInfo);
}

RowInfo* Transaction::isolateRowInfo(RowInfo* rowInfo) const {
	if (rowInfo == nullptr) {
		throw std::runtime_error("Transaction::isolateRowInfo rowInfo is nullptr");
	}

	RowInfo* isolatedInfo = nullptr;

	// TODO handle aborted and multithreading
	// remove this check - make sure chain is valid for simple single threaded transactions
	RowInfo* validInfo = rowInfo;
	while (validInfo != nullptr) {
		if (validInfo->getStatus() == RowInfo::Status::Uncommitted) {
			throw std::runtime_error("Transaction::isolateRowInfo found uncommitted row");	
		}

		if (validInfo->getStatus() == RowInfo::Status::Aborted) {
			throw std::runtime_error("Transaction::isolateRowInfo found aborted row");
		}

		if ((validInfo->getTransactionId() != m_transactionId) && (validInfo->getTransactionId() > m_viewpoint)) {
			throw std::runtime_error("Transaction::isolateRowInfo found later transaction");
		}

		validInfo = validInfo->getNext();
	}

	switch (m_isolationLevel) {
		case IsolationLevel::ReadUncommitted: 
			isolatedInfo = isolateReadUncommitted(rowInfo);
			break;
		case IsolationLevel::ReadCommitted:
			isolatedInfo = isolateReadCommitted(rowInfo);
			break;
		case IsolationLevel::RepeatableRead:
			isolatedInfo = isolateRepeatableRead(rowInfo);
			break;
		case IsolationLevel::Serializable:
			isolatedInfo = isolateSerializable(rowInfo);
			break;
		default:
			throw std::runtime_error("Transaction::isolateRowInfo unknown isolation level");
	}

	return isolatedInfo;
}

bool Transaction::isImplicit() const {
	return m_implicit;
}

void Transaction::setImplicit(bool implicit) {
	m_implicit = implicit;
}

RowInfo* Transaction::isolateReadUncommitted(RowInfo* rowInfo) const {
	RowInfo* isolatedInfo = rowInfo;

	while (isolatedInfo->getNext() != nullptr) {
		isolatedInfo = isolatedInfo->getNext();
	}

	return isolatedInfo;
}

RowInfo* Transaction::isolateReadCommitted(RowInfo* rowInfo) const {
	RowInfo* isolatedInfo = rowInfo;

	if (isolatedInfo->getStatus() != RowInfo::Status::Committed) {
		return nullptr;
	}

	while ((isolatedInfo->getNext() != nullptr) && (isolatedInfo->getNext()->getStatus() == RowInfo::Status::Committed)) {
		isolatedInfo = isolatedInfo->getNext();
	}

	return isolatedInfo;
}

RowInfo* Transaction::isolateRepeatableRead(RowInfo* rowInfo) const {
	RowInfo* isolatedInfo = rowInfo;

	RowInfo::Status status = isolatedInfo->getStatus();
	if ((status != RowInfo::Status::Committed) || 
	    ((isolatedInfo->getTransactionId() != m_transactionId) &&
	     (isolatedInfo->getTransactionId() > m_viewpoint))) {
		return nullptr;
	}

	while ((isolatedInfo->getNext() != nullptr) &&
	       (isolatedInfo->getNext()->getStatus() == RowInfo::Status::Committed) &&
	       ((isolatedInfo->getNext()->getTransactionId() == m_transactionId) ||
		(isolatedInfo->getNext()->getTransactionId() <= m_viewpoint))) {
		isolatedInfo = isolatedInfo->getNext();
	}
	
	return isolatedInfo;
}

RowInfo* Transaction::isolateSerializable(RowInfo* rowInfo) const {
	return isolateRepeatableRead(rowInfo);
}

}} // namespace
