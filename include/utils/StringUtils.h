#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

namespace squiddb { namespace utils {

class StringUtils {
	public:
		StringUtils() = delete;

		static std::string toUpper(const std::string& string);
		static std::string trim(const std::string& string);
		static std::vector<std::string> split(const std::string& string, const char separator);
};

}} // namespace

#endif
