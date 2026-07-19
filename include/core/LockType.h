#ifndef LOCKTYPE_H
#define LOCKTYPE_H

namespace squiddb { namespace core {

enum class LockType {
	None  = 0,
	Read  = 1,
	Write = 2
};

}} // namespace

#endif
