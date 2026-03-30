#pragma once
#include <string>

void InitSafeGuardRecord();
void RecordOperation(std::string const& pluginName, std::string const& operation, std::string const& content);
