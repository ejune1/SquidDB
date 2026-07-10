#ifndef STREAMFILE_H
#define STREAMFILE_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <unistd.h>

namespace squiddb { namespace storage {

class StreamFile {
	public:
		StreamFile(const std::string& filePath);
		~StreamFile();

		void open();
		void close();

		// stream read or write, use writeLock (read changes state)
		void read(std::byte* bytes, const std::size_t length);
		void write(const std::byte* bytes, const std::size_t length);
		void flush();

		// use readLock
		void pointRead(off_t offset, std::byte* bytes, const std::size_t length);

		std::shared_lock<std::shared_mutex> readLock();
		std::unique_lock<std::shared_mutex> writeLock();

		std::uint8_t getProtocolVersion() const;
		void setProtocolVersion(const std::uint8_t protocolVersion);

		std::uintmax_t getFileSize() const;

		std::size_t getMinActiveTransaction() const;
		void setMinActiveTransaction(const std::size_t minActiveTransaction);

	private:
		enum class Operation {
			None  = 0,
			Read  = 1,
			Write = 2
		};

		const std::string& m_filePath;
		std::FILE* m_file;

		std::uint8_t m_protocolVersion;

		std::uintmax_t m_fileSize;
		std::size_t m_minActiveTransaction;

		std::shared_mutex m_mutex;

		Operation m_lastOperation;
};

}} // namespace

#endif
