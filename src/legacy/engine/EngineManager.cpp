#include "engine/EngineManager.h"

#include "engine/EngineOwnData.h"
#include "engine/GlobalShareData.h"
#include "ll/api/utils/StringUtils.h"

#if defined(LSE_BACKEND_NODEJS)
#include "legacy/main/NodeJsHelper.h"
#endif

#include <map>
#include <mutex>
#include <shared_mutex>

using namespace script;

///////////////////////////////// API /////////////////////////////////

bool EngineManager::unregisterEngine(std::shared_ptr<ScriptEngine> toDelete) {
    std::unique_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto engine = globalShareData->globalEngineList.begin(); engine != globalShareData->globalEngineList.end();
         ++engine)
        if (*engine == toDelete) {
            globalShareData->globalEngineList.erase(engine);
            return true;
        }
    return false;
}

bool EngineManager::registerEngine(std::shared_ptr<ScriptEngine> engine) {
    std::unique_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    globalShareData->globalEngineList.push_back(engine);
    return true;
}

std::shared_ptr<ScriptEngine> EngineManager::newEngine(std::string pluginName) {
    std::shared_ptr<ScriptEngine> engine = nullptr;

#if defined(LSE_BACKEND_NODEJS)
    engine = NodeJsHelper::newEngine();
#elif !defined(SCRIPTX_BACKEND_WEBASSEMBLY)
    engine = std::shared_ptr<ScriptEngineImpl>(new ScriptEngineImpl, ScriptEngine::Deleter());
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
        if (i->get() == engine) {
            if (engine->isDestroying()) return false;
            if (onlyCheckLocal && getEngineType(engine) != LLSE_BACKEND_TYPE) return false;
            else return true;
        }
    return false;
}

bool EngineManager::isValid(std::shared_ptr<ScriptEngine> engine, bool onlyCheckLocal) {
    return isValid(engine.get(), onlyCheckLocal);
}

std::vector<std::shared_ptr<ScriptEngine>> EngineManager::getLocalEngines() {
    std::vector<std::shared_ptr<ScriptEngine>> res;
    std::shared_lock<std::shared_mutex>        lock(globalShareData->engineListLock);
    for (auto& engine : globalShareData->globalEngineList) {
        if (getEngineType(engine) == LLSE_BACKEND_TYPE) res.push_back(engine);
    }
    return res;
}

std::vector<std::shared_ptr<ScriptEngine>> EngineManager::getGlobalEngines() {
    std::vector<std::shared_ptr<ScriptEngine>> res;
    std::shared_lock<std::shared_mutex>        lock(globalShareData->engineListLock);
    for (auto& engine : globalShareData->globalEngineList) {
        res.push_back(engine);
    }
    return res;
}

std::shared_ptr<ScriptEngine> EngineManager::getEngine(std::string name, bool onlyLocalEngine) {
    std::shared_lock<std::shared_mutex> lock(globalShareData->engineListLock);
    for (auto& engine : globalShareData->globalEngineList) {
        if (onlyLocalEngine && getEngineType(engine) != LLSE_BACKEND_TYPE) continue;
        auto ownerData = getEngineData(engine);
        if (ownerData->pluginName == name) return engine;
    }
    return nullptr;
}

std::string EngineManager::getEngineType(std::shared_ptr<ScriptEngine> engine) {
    return getEngineData(engine)->engineType;
}

std::string EngineManager::getEngineType(ScriptEngine* engine) { return getEngineData(engine)->engineType; }