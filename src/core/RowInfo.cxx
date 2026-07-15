#include "core/RowInfo.h"

#include <atomic>
#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

RowInfo::RowInfo(RowInfo::Status status, std::byte* data, std::uint16_t size) :
	m_dirty(false), m_creating(false), m_updating(false), m_deleting(false) {
	m_status = status;
	m_next = nullptr;
	m_data = data;
	m_size = size;
	m_vFileIndex = 0;
	m_vFileOffset = 0;
}

RowInfo::Status RowInfo::getStatus() const {
	return m_status;
}

void RowInfo::setStatus(RowInfo::Status status) {
	m_status = status;
}

std::size_t RowInfo::getTransactionId() const {
	return m_transactionId.load();
}

void RowInfo::setTransactionId(std::size_t transactionId) {
	m_transactionId.store(transactionId);
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
	return m_dirty.load();
}

void RowInfo::setDirty(bool dirty) {
	m_dirty.store(dirty);
}

bool RowInfo::getCreating() const {
	return m_creating.load();
}

void RowInfo::setCreating(bool creating) {
	m_creating.store(creating);
}

bool RowInfo::getUpdating() const {
	return m_updating.load();
}

void RowInfo::setUpdating(bool updating) {
	m_updating.store(updating);
}

bool RowInfo::getDeleting() const {
	return m_deleting.load();
}

void RowInfo::setDeleting(bool deleting) {
	m_deleting.store(deleting);
}

}} // namespace
