#pragma once
#include "ScriptX/ScriptX.h"

#include <map>
#include <string>
#include <vector>

class EngineManager {
public:
    static std::shared_ptr<script::ScriptEngine> newEngine(std::string pluginName = "");
    static bool                                  registerEngine(std::shared_ptr<script::ScriptEngine> engine);
    static bool                                  unregisterEngine(std::shared_ptr<script::ScriptEngine> engine);
    static bool                                  isValid(script::ScriptEngine* engine, bool onlyCheckLocal = false);
    static bool isValid(std::shared_ptr<script::ScriptEngine> engine, bool onlyCheckLocal = false);

    static std::vector<std::shared_ptr<script::ScriptEngine>> getLocalEngines();
    static std::vector<std::shared_ptr<script::ScriptEngine>> getGlobalEngines();
    static std::shared_ptr<script::ScriptEngine>              getEngine(std::string name, bool onlyLocalEngine = false);

    static std::string getEngineType(std::shared_ptr<script::ScriptEngine> engine);
    static std::string getEngineType(script::ScriptEngine* engine);
};
