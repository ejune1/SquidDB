#include "core/RowInfo.h"

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

RowInfo::RowInfo(RowInfo::Status status, std::byte* data, std::uint16_t size) {
	m_status = status;
	m_next = nullptr;
	m_data = data;
	m_size = size;
	m_vFileIndex = 0;
	m_vFileOffset = 0;
	m_dirty = false;
}

RowInfo::Status RowInfo::getStatus() const {
	return m_status;
}

void RowInfo::setStatus(RowInfo::Status status) {
	m_status = status;
}

RowInfo* RowInfo::getNext() const {
	return m_next;
}

void RowInfo::setNext(RowInfo* rowInfo) {
	m_next = rowInfo;
}

std::byte* RowInfo::getData() const {
	return m_data;
}

void RowInfo::setData(std::byte* data, std::uint16_t size) {
	m_data = data;
	m_size = size;
}

std::uint16_t RowInfo::getSize() const {
	return m_size;
}

std::uint8_t RowInfo::getVFileIndex() const {
	return m_vFileIndex;
}

void RowInfo::setVFileIndex(std::uint8_t vFileIndex) {
	m_vFileIndex = vFileIndex;
}

std::uint32_t RowInfo::getVFileOffset() const {
	return m_vFileOffset;
}

void RowInfo::setVFileOffset(std::uint32_t vFileOffset) {
	m_vFileOffset = vFileOffset;
}

bool RowInfo::getDirty() const {
	return m_dirty;
}

void RowInfo::setDirty(bool dirty) {
	m_dirty = dirty;
}

}} // namespace
