#ifndef STREAMFILE_H
#define STREAMFILE_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <mutex>
#include <string>

namespace squiddb { namespace storage {

class StreamFile {
	public:
		StreamFile(const std::string& filePath);
		~StreamFile();

		void open();
		void close();

		void read(std::byte* bytes, const std::size_t length);
		void write(const std::byte* bytes, const std::size_t length);

		std::unique_lock<std::mutex> lock();

	private:
		enum class Operation {
			None = 0,
			Read = 1,
			Write = 2
		};

		const std::string& m_filePath;
		std::FILE* m_file;

		std::uint8_t m_protocolVersion;

		std::size_t m_fileSize;
		std::size_t m_minActiveTransaction;

		std::mutex m_mutex;

		Operation m_lastOperation;
};

}} // namespace

#endif
