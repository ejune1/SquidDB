#include "core/KeyRowInfo.h"

#include "core/RowInfo.h"

#include <cstddef>

namespace squiddb { namespace core {

KeyRowInfo::KeyRowInfo(const std::byte* key, RowInfo* rowInfo) : m_key(key) {
	m_rowInfo = rowInfo;
}

const std::byte* KeyRowInfo::getKey() const {
	return m_key;
}

RowInfo* KeyRowInfo::getRowInfo() const {
	return m_rowInfo;
}

}} // namespace
