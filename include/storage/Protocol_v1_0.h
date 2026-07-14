#ifndef PROTOCOL_V1_0.h
#define PROTOCOL_V1_0.h

#include "core/KeyRowInfo.h"
#include "storage/StreamFile.h"

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace storage {

class Protocol_v1_0 {
	public:
		Protocol_v1_0() = delete;

		static void markFile(StreamFile* file);

		static void readValue(StreamFile* vsFile, std::byte* bytes, const std::uint16_t length);
		static void writeValue(StreamFile* vsFile, const KeyRowInfo* keyRowInfo);

		static void readLog(StreamFile* lsFile, KeyRowInfo* keyRowInfo);
		static void writeLog(StreamFile* lsFile, const KeyRowInfo* keyRowInfo);

	private:
		static const std:uint32_t SQUID_MAGIC = 0x51701DDB;
		static const std::uint32_t VERSION    = 0x00000001;
};

}} // namespace

#endif
