#pragma once
#include <fstream>
#include <optional>
#include <string>
#include <vector>

std::vector<std::string> SplitCmdLine(const std::string& paras);

bool IsVersionLess(const std::string& v1, const std::string& v2);
bool IsVersionLess(int v1a, int v1b, int v1c, int v2a, int v2b, int v2c);

unsigned long long GetCurrentTimeStampMS();
wchar_t*           str2cwstr(std::string str);

// System
std::string  Raw_GetDateTimeStr();
std::string  Raw_RandomGuid();
std::wstring Raw_RandomGuidW();
