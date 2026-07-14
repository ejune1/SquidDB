#include "core/KeyRowInfo.h"

#include "core/RowInfo.h"

#include <cstdint>
#include <cstddef>

namespace squiddb { namespace core {

KeyRowInfo::KeyRowInfo(const std::byte* key, const std::uint16_t keySize, RowInfo* rowInfo) : 
	m_key(key), m_keySize(keySize) {
	m_rowInfo = rowInfo;
}

const std::byte* KeyRowInfo::getKey() const {
	return m_key;
}

std::uint16_t KeyRowInfo::getKeySize() const {
	return m_keySize;
}

RowInfo* KeyRowInfo::getRowInfo() const {
	return m_rowInfo;
}

}} // namespace
