#include "legacy/main/Global.h"

#include <filesystem>
#include <fstream>

std::ofstream record;

void InitSafeGuardRecord() {
    std::filesystem::create_directories("logs/LegacyScriptEngine");
    record.open(
        std::string("logs/LegacyScriptEngine/Sensitive_Operation_Records-") + LLSE_BACKEND_TYPE + ".log",
        std::ios::app
    );
}

void RecordOperation(std::string const& pluginName, std::string const& operation, std::string const& content) {
    if (record.is_open()) record << "[" << operation << "]<" << pluginName << "> " << content << std::endl;
}
