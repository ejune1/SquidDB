#ifndef DIRECTORYCLEANUP_H
#define DIRECTORYCLEANUP_H

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace squiddb { namespace utils {

struct DirectoryCleanup {
	std::filesystem::path directory;
	std::vector<std::filesystem::path> filePaths;

	DirectoryCleanup(const std::string directoryString) : directory(directoryString) { }

	~DirectoryCleanup() {
		for (std::filesystem::path filePath : filePaths) {
			if (std::filesystem::exists(filePath) == true) {
				std::filesystem::remove(filePath);
			}
		}
	}

	void createFile(const std::string tableName, int sequenceNumber, const std::string extension) {
		char buffer[32];
		std::snprintf(buffer, sizeof(buffer), "%03d", sequenceNumber);

		std::filesystem::path filePath = directory / (tableName + "." + buffer + extension);
		std::ofstream ofs(filePath);
		ofs.close();

		filePaths.push_back(filePath);
	}

	void addFile(const std::string filePath) {
		filePaths.push_back(filePath);
	}
};

}} // namespace

#endif
