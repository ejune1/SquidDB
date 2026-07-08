#ifndef KEYROWINFO_H
#define KEYROWINFO_H

#include "core/RowInfo.h"

#include <cstddef>

namespace squiddb { namespace core {

class KeyRowInfo {
	public:
		KeyRowInfo(const std::byte* key, RowInfo* rowInfo);
		~KeyRowInfo() = default;

		const std::byte* getKey() const;
		RowInfo* getRowInfo() const;

	private:
		const std::byte* m_key;
		RowInfo* m_rowInfo;
};

}} // namespace

#endif
