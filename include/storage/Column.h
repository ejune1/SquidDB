#ifndef COLUMN_H
#define COLUMN_H

#include <cstdint>
#include <string>

namespace squiddb { namespace storage {

class Column {
	public:
		enum class ColumnType {
			Int32   = 0,
			Int64   = 1,
			Float   = 2,
			String  = 3,
			Unknown = 4
		};

		enum class KeyType {
			None      = 0,
			Primary   = 1,
			Secondary = 2,
			Unknown   = 3
		};

		Column(const std::string name, const std::uint16_t size, const ColumnType columnType, KeyType keyType);

		std::string getName() const;
		std::uint16_t getSize() const;
		ColumnType getColumnType() const;
		KeyType getKeyType() const;

		static std::string columnTypeString(const ColumnType columnType);
		static ColumnType parseColumnType(const std::string columnTypeString);

		static std::string keyTypeString(const KeyType keyType);
		static KeyType parseKeyType(const std::string keyTypeString);

	private:
		std::string m_name;
		std::uint16_t m_size;
		ColumnType m_type;
		KeyType m_keyType;
};

}} // namespace

#endif
