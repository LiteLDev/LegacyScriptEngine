#pragma once
#include "ll/api/Logger.h"

#include <Windows.h>
#include <fstream>
#include <optional>
#include <string>

namespace lse::legacy {
std::vector<std::string>    GetFileNameList(const std::string& dir);
std::pair<int, std::string> UncompressFile(const std::string& filePath, const std::string& toDir, int processTimeout);
} // namespace lse::legacy
