#include "engine/EngineManager.h"

#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "ll/api/utils/StringUtils.h"

#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
#include "legacy/main/NodeJsHelper.h"
#endif

#include <map>
#include <mutex>
#include <shared_mutex>

using namespace script;

///////////////////////////////// API /////////////////////////////////

bool EngineManager::unregisterEngine(ScriptEngine* toDelete) {
    std::unique_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto engine = globalShareData->globalEngineList.begin(); engine != globalShareData->globalEngineList.end();
         ++engine)
        if (*engine == toDelete) {
            globalShareData->globalEngineList.erase(engine);
            return true;
        }
    return false;
}

bool EngineManager::registerEngine(ScriptEngine* engine) {
    std::unique_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    globalShareData->globalEngineList.push_back(engine);
    return true;
}

ScriptEngine* EngineManager::newEngine(std::string pluginName) {
    ScriptEngine* engine = nullptr;

#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
    engine = NodeJsHelper::newEngine();
#elif !defined(SCRIPTX_BACKEND_WEBASSEMBLY)
    engine = new ScriptEngineImpl();
#else
    engine = ScriptEngineImpl::instance();
#endif

    engine->setData(std::make_shared<EngineOwnData>());
    registerEngine(engine);
    if (!pluginName.empty()) {
        getEngineData(engine)->pluginName = pluginName;
    }
    return engine;
}

bool EngineManager::isValid(ScriptEngine* engine, bool onlyCheckLocal) {
    std::shared_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto i = globalShareData->globalEngineList.begin(); i != globalShareData->globalEngineList.end(); ++i)
        if (*i == engine) {
            if (engine->isDestroying()) return false;
            if (onlyCheckLocal && getEngineType(engine) != LLSE_BACKEND_TYPE) return false;
            else return true;
        }
    return false;
}

std::vector<ScriptEngine*> EngineManager::getLocalEngines() {
    std::vector<ScriptEngine*>          res;
    std::shared_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto& engine : globalShareData->globalEngineList) {
        if (getEngineType(engine) == LLSE_BACKEND_TYPE) res.push_back(engine);
    }
    return res;
}

std::vector<ScriptEngine*> EngineManager::getGlobalEngines() {
    std::vector<ScriptEngine*>          res;
    std::shared_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto& engine : globalShareData->globalEngineList) {
        res.push_back(engine);
    }
    return res;
}

ScriptEngine* EngineManager::getEngine(std::string name, bool onlyLocalEngine) {
    std::shared_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto& engine : globalShareData->globalEngineList) {
        if (onlyLocalEngine && getEngineType(engine) != LLSE_BACKEND_TYPE) continue;
        auto ownerData = getEngineData(engine);
        if (ownerData->pluginName == name) return engine;
    }
    return nullptr;
}

std::string EngineManager::getEngineType(ScriptEngine* engine) { return getEngineData(engine)->engineType; }
