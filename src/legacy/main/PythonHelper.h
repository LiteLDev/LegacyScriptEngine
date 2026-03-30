#pragma once
#include <ScriptX/ScriptX.h>
#include <filesystem>
#include <string>

namespace PythonHelper {

bool initPythonRuntime();

// raw, will throw exception if fail
bool loadPluginCode(
    std::shared_ptr<script::ScriptEngine> const& engine,
    std::string const&                           entryScriptPath,
    std::string                                  pluginDirPath
);

std::string findEntryScript(std::string const& dirPath);
std::string getPluginPackageName(std::string const& dirPath);
std::string getPluginPackDependencyFilePath(std::string const& dirPath);

bool processPythonDebugEngine(std::string const& cmd);

bool processConsolePipCmd(std::string const& cmd);
int  executePipCommand(std::string cmd);

} // namespace PythonHelper
