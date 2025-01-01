#include "legacy/main/Global.h"

#include <ScriptX/ScriptX.h>
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

void RecordOperation(const std::string& pluginName, const std::string& operation, const std::string& content) {
    if (record.is_open()) record << "[" << operation << "]<" << pluginName << "> " << content << std::endl;
}
