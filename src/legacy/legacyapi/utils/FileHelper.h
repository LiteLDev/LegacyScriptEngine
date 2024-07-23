#pragma once
#include <Windows.h>
#include <expected>
#include <ll/api/Logger.h>
#include <string>

namespace lse::legacy {
std::vector<std::string>    GetFileNameList(const std::string& dir);
std::pair<int, std::string> UncompressFile(const std::string& filePath, const std::string& toDir, int processTimeout);
} // namespace lse::legacy
