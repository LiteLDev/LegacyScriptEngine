#pragma once
#include "ll/api/Logger.h"
#include <Windows.h>
#include <fstream>
#include <optional>
#include <string>

std::vector<std::string> GetFileNameList(const std::string &dir);
bool CreateDirs(const std::string path);
std::optional<std::string> ReadAllFile(const std::string &filePath,
                                       bool isBinary = false);
bool WriteAllFile(const std::string &filePath, const std::string &content,
                  bool isBinary = false);
std::pair<int, std::string> UncompressFile(const std::string &filePath,
                                           const std::string &toDir,
                                           int processTimeout);