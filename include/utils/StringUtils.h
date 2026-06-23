#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>

namespace squiddb { namespace utils {

class StringUtils {
	public:
		StringUtils() = delete;

		static std::string toUpper(const std::string& string);
		static std::string trim(const std::string& string);
};

}} // namespace

#endif
