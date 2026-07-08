#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "core/KeyRowInfo.h"

#include <cstdint>
#include <vector>

namespace squiddb { namespace core {

class Transaction {
	public:
		Transaction(const std::size_t transactionId);
		~Transaction() = default;

		std::size_t getTransactionId() const;

		std::vector<KeyRowInfo*> getAffectedRows() const;
		void addAffectedRow(KeyRowInfo* keyRowInfo);

	private:
		const std::size_t m_transactionId;

		std::vector<KeyRowInfo*> m_keyRowInfo;
};

}} // namespace

#endif
