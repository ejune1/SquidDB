#ifndef COLUMN_H
#define COLUMN_H

#include <string>

namespace squiddb { namespace storage {

class Column {
	public:
		enum class ColumnType {
			INT = 0,
			LONG = 1,
			FLOAT = 2,
			STRING = 3,
			UNKNOWN = 4
		};

		Column(const std::string name, const size_t size, const ColumnType columnType, bool primaryKey, bool secondaryKey);

		std::string getName() const;
		size_t getSize() const;
		ColumnType getColumnType() const;
		bool getPrimaryKey() const;
		bool getSecondaryKey() const;

	private:
		std::string m_name;
		size_t m_size;
		ColumnType m_type;

		bool m_primaryKey;
		bool m_secondaryKey;
};

}} // namespace

#endif
