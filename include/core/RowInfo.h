#ifndef ROWINFO_H
#define ROWINFO_H

#include <cstddef>
#include <cstdint>

namespace squiddb { namespace core {

class RowInfo {
	public:
		enum class Status {
			None = 0,
			Committed = 1,
			Uncommitted = 2
		};

		RowInfo(Status status, std::byte* data, std::uint16_t size);
		~RowInfo() = default;

		Status getStatus() const;
		void setStatus(Status status);

		RowInfo* getNext() const;
		void setNext(RowInfo* rowInfo);

		std::byte* getData() const;
		void setData(std::byte* data, std::uint16_t size);

		std::uint16_t getSize() const;

		std::uint8_t getVFileIndex() const;
		void setVFileIndex(std::uint8_t vFileIndex);

		std::uint32_t getVFileOffset() const;
		void setVFileOffset(std::uint32_t vFileOffset);

		bool getDirty() const;
		void setDirty(bool dirty);

	private:
		Status m_status;
		RowInfo* m_next;

		std::byte* m_data;
		std::uint16_t m_size;

		std::uint8_t m_vFileIndex;
		std::uint32_t m_vFileOffset;

		// TODO use flags
		bool m_dirty;
};

}} // namespace

#endif
