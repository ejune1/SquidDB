#ifndef FILECLEANUP_H
#define FILECLEANUP_H

#include <filesystem>
#include <string>

namespace squiddb { namespace utils {

struct FileCleanup {
	std::filesystem::path filePath;
	FileCleanup(const std::string& path) : filePath(path) { }

	~FileCleanup() {
		if (std::filesystem::exists(filePath)) {
			std::filesystem::remove(filePath);
		}
	}
};

}} // namespace

#endif
