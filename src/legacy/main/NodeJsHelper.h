#pragma once
#pragma warning(disable : 4251)
#include "legacy/main/Global.h" // IWYU pragma: keep
#include "node.h"

#include <ScriptX/ScriptX.h>
#include <string>

namespace NodeJsHelper {

bool initNodeJs();
void shutdownNodeJs();

std::shared_ptr<script::ScriptEngine> newEngine();
bool                                  stopEngine(std::shared_ptr<script::ScriptEngine> const& engine);
bool                                  stopEngine(node::Environment* env);
std::shared_ptr<script::ScriptEngine> getEngine(node::Environment const* env);

bool loadPluginCode(
    std::shared_ptr<script::ScriptEngine> engine,
    std::string                           entryScriptPath,
    std::string                           pluginDirPath,
    bool                                  esm = false
); // raw

std::string findEntryScript(std::string const& dirPath);
std::string getPluginPackageName(std::string const& dirPath);
bool        doesPluginPackHasDependency(std::string const& dirPath);
bool        isESModulesSystem(std::string const& dirPath);

bool processConsoleNpmCmd(std::string const& cmd);
int executeNpmCommand(std::vector<std::string> npmArgs = {"i", "--omit=dev", "--no-fund"}, std::string workingDir = "");

} // namespace NodeJsHelper
