#ifndef KEYROWINFO_H
#define KEYROWINFO_H

#include "core/RowInfo.h"

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

class KeyRowInfo {
	public:
		KeyRowInfo(const std::byte* key, const std::uint16_t keySize, RowInfo* rowInfo);
		~KeyRowInfo() = default;

		const std::byte* getKey() const;
		std::uint16_t getKeySize() const;

		RowInfo* getRowInfo() const;

	private:
		const std::byte* m_key;
		const std::uint16_t m_keySize;

		RowInfo* m_rowInfo;
};

}} // namespace

#endif
