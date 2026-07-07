#include "utils/StringUtils.h"

#include <cctype>
#include <string>
#include <vector>

namespace squiddb { namespace utils {

std::string StringUtils::toUpper(const std::string& string) {
	std::string result = "";
	result.reserve(100);

	for (size_t x = 0; x < string.length(); x++) {
		result += std::toupper(string[x]);
	}

	return result;
}

std::string StringUtils::trim(const std::string& string) {
	if (string.empty() == true) {
		return "";
	}

	int first = -1;
	for (size_t x = 0; x < string.length(); x++) {
		if (std::isspace(string[x]) == false) {
			first = x;
			break;
		}
	}

	if (first == -1) {
		return "";
	}

	size_t last = -1;
	for (size_t x = string.length() - 1; x != 0; x--) {
		if (std::isspace(string[x]) == false) {
			last = x;
			break;
		}
	}

	// make sure we don't keep reallocating strings
	std::string result = "";
	result.reserve(100);

	for (size_t x = first; x <= last; x++) {
		result += string[x];
	}

	return result;
}

std::vector<std::string> StringUtils::split(const std::string& string, const char separator) {
	std::vector<std::string> tokens;

	std::string token = "";

	for (char c : string) {
		if (c == separator) {
			tokens.push_back(token);
			token="";
			continue;
		}
		token += c;
	}

	tokens.push_back(token);
	return tokens;
}

}} //namespace
