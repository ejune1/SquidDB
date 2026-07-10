#ifndef ISOLATION_LEVEL_H
#define ISOLATION_LEVEL_H

#include <cstdint>

namespace squiddb { namespace core {

enum class IsolationLevel : std::uint8_t {
	ReadUncommitted = 0,
	ReadCommitted   = 1,
	RepeatableRead  = 2,
	Serializable    = 3
};

}} // namespace

#endif
