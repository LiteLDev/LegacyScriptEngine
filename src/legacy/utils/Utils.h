#pragma once
#include <fstream>
#include <optional>
#include <string>
#include <vector>

std::vector<std::string> SplitCmdLine(std::string const& paras);

bool IsVersionLess(std::string const& v1, std::string const& v2);
bool IsVersionLess(int v1a, int v1b, int v1c, int v2a, int v2b, int v2c);

unsigned long long   GetCurrentTimeStampMS();
std::vector<wchar_t> str2cwstr(std::string const& str);

// System
std::string  Raw_GetDateTimeStr();
std::string  Raw_RandomGuid();
std::wstring Raw_RandomGuidW();
