#include "storage/StreamFile.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <stdexcept>
#include <string>

namespace squiddb { namespace storage {

StreamFile::StreamFile(const std::string& filePath) : m_filePath(filePath) {
	m_file = nullptr;
	m_protocolVersion = 0;
	m_fileSize = 0;
	m_minActiveTransaction = 0;
	m_lastOperation = Operation::None;
}

StreamFile::~StreamFile() {
	if (m_file != nullptr) {
		std::fclose(m_file);
		m_file = nullptr;
	}

	m_lastOperation = Operation::None;
}

void StreamFile::open() {
	if (m_file != nullptr) {
		throw std::runtime_error("StreamFile::open file already opened " + m_filePath);
	}

	m_file = std::fopen(m_filePath.c_str(), "a+b");

	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::open could not open file " + m_filePath);
	}

	m_lastOperation = Operation::None;
}

void StreamFile::close() {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::close file is nullptr " + m_filePath);
	}

	std::fclose(m_file);
	m_file = nullptr;

	m_lastOperation = Operation::None;
}

void StreamFile::read(std::byte* bytes, const std::size_t length) {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::read file is nullptr " + m_filePath);
	}

	if (m_lastOperation == Operation::Write) {
		std::fseek(m_file, 0 /* offset */, SEEK_CUR);
	}

	std::size_t bytesRead = std::fread(reinterpret_cast<void*>(bytes), 1, length, m_file);

	if (bytesRead < length) {
		throw std::runtime_error("StreamFile::read read " + std::to_string(bytesRead) + " bytes");
	}

	m_lastOperation = Operation::Read;	
}

void StreamFile::write(const std::byte* bytes, const std::size_t length) {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::write file is nullptr " + m_filePath);
	}

	if (m_lastOperation == Operation::Read) {
		std::fseek(m_file, 0, SEEK_CUR);
	}

	std::size_t bytesWritten = std::fwrite(reinterpret_cast<const void*>(bytes), 1, length, m_file);

	if (bytesWritten < length) {
		throw std::runtime_error("StreamFile::write wrote " + std::to_string(bytesWritten) + " bytes");
	}

	m_lastOperation = Operation::Write;
}

std::unique_lock<std::mutex> StreamFile::lock() {
	return std::unique_lock<std::mutex>(m_mutex);
}

}} // namespace
