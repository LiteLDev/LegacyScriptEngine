#pragma once
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
#include "Configs.h"

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

// This fix is used for Python3.10's bug:
// The thread will freeze when creating a new engine while another thread is
// blocking to read stdin Side effects: sys.stdin cannot be used after this
// patch. More info to see: https://github.com/python/cpython/issues/83526
//
// Attention! When CPython is upgraded, this fix must be re-adapted or removed!!
//
namespace FixPython310Stdin {

bool patchPython310CreateStdio();
bool unpatchPython310CreateStdio();

} // namespace FixPython310Stdin

} // namespace PythonHelper

#endif
