#pragma once
#include "ScriptX/ScriptX.h"

#include <map>
#include <string>
#include <vector>

class EngineManager {
public:
    static script::ScriptEngine* newEngine(std::string pluginName = "");
    static bool                  registerEngine(script::ScriptEngine* engine);
    static bool                  unregisterEngine(script::ScriptEngine* engine);
    static bool                  isValid(script::ScriptEngine* engine, bool onlyCheckLocal = false);

    static std::vector<script::ScriptEngine*> getLocalEngines();
    static std::vector<script::ScriptEngine*> getGlobalEngines();
    static script::ScriptEngine*              getEngine(std::string name, bool onlyLocalEngine = false);

    static std::string getEngineType(script::ScriptEngine* engine);
};
