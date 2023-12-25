#include "main/Configs.h"
#include <filesystem>
#include <fstream>
#include <scriptx/include/scriptx/ScriptX.h>

using namespace std;

ofstream record;

void InitSafeGuardRecord() {
  filesystem::create_directories("logs/LiteLoader");
  record.open(string("logs/LiteLoader/Sensitive_Operation_Records-") +
                  LLSE_BACKEND_TYPE + ".log",
              ios::app);
}

void RecordOperation(const string &pluginName, const std::string &operation,
                     const string &content) {
  if (record.is_open())
    record << "[" << operation << "]<" << pluginName << "> " << content << endl;
}