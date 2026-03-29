#pragma once
#include "utils/UsingScriptX.h"

#include <string>
#include <vector>

class EngineManager {
public:
    static std::shared_ptr<ScriptEngine> newEngine(std::string const& pluginName = "");
    static bool                          registerEngine(std::shared_ptr<ScriptEngine> const& engine);
    static bool                          unregisterEngine(std::shared_ptr<ScriptEngine> const& engine);
    static bool                          isValid(ScriptEngine* engine, bool onlyCheckLocal = false);
    static bool isValid(std::shared_ptr<ScriptEngine> const& engine, bool onlyCheckLocal = false);
    static std::shared_ptr<ScriptEngine> checkAndGet(ScriptEngine* engine, bool onlyCheckLocal = false);

    static std::vector<std::shared_ptr<ScriptEngine>> getLocalEngines();
    static std::vector<std::shared_ptr<ScriptEngine>> getGlobalEngines();
    static std::shared_ptr<ScriptEngine>              getEngine(std::string const& name, bool onlyLocalEngine = false);

    static std::string getEngineType(std::shared_ptr<ScriptEngine> const& engine);
    static std::string getEngineType(ScriptEngine* engine);
};
