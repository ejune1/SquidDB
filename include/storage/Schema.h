#ifndef SCHEMA_H
#define SCHEMA_H

#include "storage/Column.h"

#include <vector>

namespace squiddb { namespace storage {

class Schema {


	private:
		std::vector<Column> m_column;
		std::vector<size_t> m_offset;
};

}} // namespace

#endif
