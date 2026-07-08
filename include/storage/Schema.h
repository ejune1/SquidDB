#ifndef SCHEMA_H
#define SCHEMA_H

#include "storage/Column.h"
#include "utils/Logger.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace squiddb { namespace storage {

class Schema {
	public:
		Schema(utils::Logger& logger);
		~Schema() = default;

		void read(const std::string filePath);
		void write(const std::string filePath);

		void addColumn(const Column column);
		const std::vector<Column>& getColumns() const;

		std::int32_t readInt32(const std::string columnName, const std::byte* row);
		std::int64_t readInt64(const std::string columnName, const std::byte* row);
		// TODO reads for other data types
		
		std::uint16_t getOffset(const std::string& columnName) const;
		std::uint16_t getTotalSize() const;

		// provide fast access to primary information for now
		std::uint16_t getPrimaryOffset() const;
		std::uint16_t getPrimarySize() const;

	private:
		utils::Logger& m_logger;

		std::vector<Column> m_column;
		std::unordered_map<std::string, std::uint16_t> m_offset;

		std::uint16_t m_totalSize;

		std::uint16_t m_primaryOffset;
		std::uint16_t m_primarySize;
};

}} // namespace

#endif
