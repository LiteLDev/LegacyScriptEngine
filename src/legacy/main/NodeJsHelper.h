#pragma once
#pragma warning(disable : 4251)
#include "legacy/main/Global.h"

#include <ScriptX/ScriptX.h>
#include <map>
#include <node.h>
#include <string>

namespace NodeJsHelper {

bool initNodeJs();
void shutdownNodeJs();

script::ScriptEngine* newEngine();
bool                  stopEngine(script::ScriptEngine* engine);
bool                  stopEngine(node::Environment* env);
script::ScriptEngine* getEngine(node::Environment* env);

bool loadPluginCode(
    script::ScriptEngine* engine,
    std::string           entryScriptPath,
    std::string           pluginDirPath,
    bool                  esm = false
); // raw

std::string findEntryScript(const std::string& dirPath);
std::string getPluginPackageName(const std::string& dirPath);
bool        doesPluginPackHasDependency(const std::string& dirPath);
bool        isESModulesSystem(const std::string& dirPath);

bool processConsoleNpmCmd(const std::string& cmd);
int  executeNpmCommand(const std::string& cmd, std::string workingDir = "");

} // namespace NodeJsHelper
