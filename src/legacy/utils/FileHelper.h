#pragma once

#include <ll/api/Expected.h>
#include <ll/api/io/Logger.h>
#include <string>

namespace lse::legacy {
std::pair<int, std::string> UncompressFile(std::string const& filePath, std::string const& toDir, int processTimeout);
} // namespace lse::legacy
