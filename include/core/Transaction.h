#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "core/IsolationLevel.h"
#include "core/KeyRowInfo.h"
#include "core/RowInfo.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace squiddb { namespace core {

class Transaction {
	public:
		Transaction(const std::size_t transactionId, const std::size_t viewpoint, const IsolationLevel isolationLevel);
		~Transaction();

		std::size_t getTransactionId() const;
		std::size_t getViewpoint() const;

		IsolationLevel getIsolationLevel() const;

		std::vector<KeyRowInfo*> getAffectedRows() const;

		void addAffectedRow(const std::byte* key, const std::uint16_t keySize, RowInfo* rowInfo);
		void addAffectedRow(KeyRowInfo* keyRowInfo);

	private:
		const std::size_t m_transactionId;

		// should be the min active transactionId - 1 when this tx is created
		const std::size_t m_viewpoint;

		const IsolationLevel m_isolationLevel;

		std::vector<KeyRowInfo*> m_keyRowInfo;
};

}} // namespace

#endif
