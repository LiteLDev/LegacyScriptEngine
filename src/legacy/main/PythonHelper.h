#pragma once
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
#include <ScriptX/ScriptX.h>
#include <filesystem>
#include <map>
#include <string>

namespace PythonHelper {

bool initPythonRuntime();

// raw, will throw exception if fail
bool loadPluginCode(script::ScriptEngine* engine, std::string entryScriptPath, std::string pluginDirPath);

std::string findEntryScript(const std::string& dirPath);
std::string getPluginPackageName(const std::string& dirPath);
std::string getPluginPackDependencyFilePath(const std::string& dirPath);

bool processPythonDebugEngine(const std::string& cmd);

bool processConsolePipCmd(const std::string& cmd);
int  executePipCommand(std::string cmd);

} // namespace PythonHelper

#endif
