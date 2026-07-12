#include "storage/StreamFile.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unistd.h>

namespace squiddb { namespace storage {

StreamFile::StreamFile(const std::string filePath) : m_filePath(filePath) {
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

std::string StreamFile::getFilePath() const {
	return m_filePath;
}

void StreamFile::open() {
	if (m_file != nullptr) {
		throw std::runtime_error("StreamFile::open file already opened " + m_filePath);
	}

	m_file = std::fopen(m_filePath.c_str(), "a+b");

	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::open could not open file " + m_filePath);
	}

	m_fileSize = std::filesystem::file_size(m_filePath);

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

	// would never want to go from write to read (use pread for point reads)
	if (m_lastOperation == Operation::Write) {
		throw std::runtime_error("StreamFile::read last operation was write " + m_filePath);
	}

	std::size_t bytesRead = std::fread(reinterpret_cast<void*>(bytes), 1, length, m_file);

	if (bytesRead != length) {
		throw std::runtime_error("StreamFile::read read " + std::to_string(bytesRead) + " bytes");
	}

	m_lastOperation = Operation::Read;	
}

void StreamFile::write(const std::byte* bytes, const std::size_t length) {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::write file is nullptr " + m_filePath);
	}

	// can go from read to write on the last file
	if (m_lastOperation == Operation::Read) {
		std::fseek(m_file, 0, SEEK_CUR);
	}

	std::size_t bytesWritten = std::fwrite(reinterpret_cast<const void*>(bytes), 1, length, m_file);

	if (bytesWritten != length) {
		throw std::runtime_error("StreamFile::write wrote " + std::to_string(bytesWritten) + " bytes");
	}

	m_fileSize += length;

	m_lastOperation = Operation::Write;
}

void StreamFile::flush() {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::flush file is nullptr " + m_filePath);
	}

	if (m_lastOperation != Operation::Write) {
		throw std::runtime_error("StreamFile::flush last operation not write " + m_filePath);
	}

	std::fflush(m_file);
}

void StreamFile::pointRead(off_t offset, std::byte* bytes, const std::size_t length) {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::pointRead file is nullptr " + m_filePath);
	}

	int fd = fileno(m_file);

	ssize_t bytesRead = pread(fd, reinterpret_cast<char*>(bytes), length, offset);

	if ((bytesRead < 0) || (static_cast<std::size_t>(bytesRead) != length)) {
		throw std::runtime_error("StreamFile::pointRead read " + std::to_string(bytesRead) + " bytes");
	}
}

std::shared_lock<std::shared_mutex> StreamFile::readLock() {
	return std::shared_lock<std::shared_mutex>(m_mutex);
}

std::unique_lock<std::shared_mutex> StreamFile::writeLock() {
	return std::unique_lock<std::shared_mutex>(m_mutex);
}

std::uint8_t StreamFile::getProtocolVersion() const {
	return m_protocolVersion;
}

void StreamFile::setProtocolVersion(const std::uint8_t protocolVersion) {
	m_protocolVersion = protocolVersion;
}

std::uintmax_t StreamFile::getFileSize() const {
	return m_fileSize;
}

long StreamFile::getPosition() {
	if (m_file == nullptr) {
		throw std::runtime_error("StreamFile::getPosition file is nullptr " + m_filePath);
	}

	// this should only happen after a replay (startup)
	if (m_lastOperation == Operation::Read) {
		fseek(m_file, 0, SEEK_END);
	}

	return ftell(m_file);
}

std::size_t StreamFile::getMinActiveTransaction() const {
	return m_minActiveTransaction;
}

void StreamFile::setMinActiveTransaction(const std::size_t minActiveTransaction) {
	m_minActiveTransaction = minActiveTransaction;
}

std::uint8_t StreamFile::getSequenceNumber() const {
	return m_sequenceNumber;
}

void StreamFile::setSequenceNumber(std::uint8_t sequenceNumber) {
	m_sequenceNumber = sequenceNumber;
}

}} // namespace
