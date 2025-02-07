#pragma once
#include "ll/api/base/Macro.h"

#include <Windows.h>
#include <ll/api/Expected.h>
#include <ll/api/io/Logger.h>
#include <string>

namespace lse::legacy {
std::pair<int, std::string> UncompressFile(const std::string& filePath, const std::string& toDir, int processTimeout);
} // namespace lse::legacy
