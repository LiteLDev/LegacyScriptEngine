#include "Entry.h"

#include "Config.h"
#include "PluginManager.h"
#include "PluginMigration.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/main/EconomicSystem.h"
#include "lse/api/MoreGlobal.h"

#include <ScriptX/ScriptX.h>
#include <exception>
#include <ll/api/Config.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/utils/ErrorUtils.h>
#include <memory>
#include <stdexcept>

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_LUA

constexpr auto BaseLibFileName = "BaseLib.lua";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS

constexpr auto BaseLibFileName = "BaseLib.js";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON

#include "legacy/main/PythonHelper.h"
constexpr auto BaseLibFileName = "BaseLib.py";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS

#include "legacy/main/NodeJsHelper.h"

#endif

using namespace ll::i18n_literals;

// Do not use legacy headers directly, otherwise there will be tons of errors.
void                  BindAPIs(script::ScriptEngine* engine);
void                  InitBasicEventListeners();
void                  InitGlobalShareData();
void                  InitLocalShareData();
void                  InitMessageSystem();
void                  InitSafeGuardRecord();
void                  RegisterDebugCommand();
bool                  isInConsoleDebugMode; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
script::ScriptEngine* debugEngine;          // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

namespace lse {

namespace {

Config config; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::shared_ptr<PluginManager> pluginManager; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::unique_ptr<std::reference_wrapper<ll::mod::NativeMod>>
    selfPluginInstance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void loadConfig(const ll::mod::NativeMod& self, Config& config);
void loadDebugEngine(const ll::mod::NativeMod& self);
void registerPluginManager(const std::shared_ptr<PluginManager>& pluginManager);

auto enable(ll::mod::NativeMod& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();
    if (!MoreGlobal::onEnable()) {
        logger.error("Failed to enable MoreGlobal"_tr());
    }
    try {
#ifndef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
        RegisterDebugCommand();
#endif

        return true;

    } catch (const std::exception& error) {
        logger.error("Failed to enable: {0}"_tr(error.what()));
        return false;
    }
}

void initializeLegacyStuff() {
    InitLocalShareData();
    InitGlobalShareData();
    InitSafeGuardRecord();
    EconomySystem::init();
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    PythonHelper::initPythonRuntime();
#endif

    InitBasicEventListeners();
    InitMessageSystem();
    MoreGlobal::onLoad();
}

auto load(ll::mod::NativeMod& self) -> bool {
    auto& logger = self.getLogger();
#ifdef NDEBUG
    ll::error_utils::setSehTranslator();
#endif

    try {
        ll::i18n::load(self.getLangDir());

        config             = Config();
        pluginManager      = std::make_shared<PluginManager>();
        selfPluginInstance = std::make_unique<std::reference_wrapper<ll::mod::NativeMod>>(self);

        loadConfig(self, config);

        if (config.migratePlugins) {
            migratePlugins(*pluginManager);
        }

        registerPluginManager(pluginManager);

        // Legacy stuff should be initialized before any possible call to legacy code.
        initializeLegacyStuff();

        loadDebugEngine(self);

        return true;

    } catch (const std::exception& error) {
        logger.error("Failed to load: {0}"_tr(error.what()));
        return false;
    }
}

void loadConfig(const ll::mod::NativeMod& self, Config& cfg) {
    const auto& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(cfg, configFilePath) && !ll::config::saveConfig(cfg, configFilePath)) {
        throw std::runtime_error("Cannot save default configurations to {0}"_tr(configFilePath));
    }
}

void loadDebugEngine(const ll::mod::NativeMod& self) {
#ifndef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS // NodeJs backend didn't enable debug engine now
    auto& scriptEngine = *EngineManager::newEngine();

    script::EngineScope engineScope(scriptEngine);

    BindAPIs(&scriptEngine);

    // Load BaseLib.
    auto baseLibPath    = self.getModDir() / "baselib" / BaseLibFileName;
    auto baseLibContent = ll::file_utils::readFile(baseLibPath);
    if (!baseLibContent) {
        throw std::runtime_error("Failed to read BaseLib at {0}"_tr(baseLibPath.string()));
    }
    scriptEngine.eval(baseLibContent.value());

    debugEngine = &scriptEngine;
#endif
}

void registerPluginManager(const std::shared_ptr<PluginManager>& pm) {
    auto& pluginManagerRegistry = ll::mod::ModManagerRegistry::getInstance();

    if (!pluginManagerRegistry.addManager(pm)) {
        throw std::runtime_error("Failed to register plugin manager"_tr());
    }
}

} // namespace

auto getConfig() -> const Config& { return config; }

auto getPluginManager() -> PluginManager& {
    if (!pluginManager) {
        throw std::runtime_error("pluginManager is null");
    }

    return *pluginManager;
}

auto getSelfPluginInstance() -> ll::mod::NativeMod& {
    if (!selfPluginInstance) {
        throw std::runtime_error("selfPluginInstance is null");
    }

    return *selfPluginInstance;
}

} // namespace lse

extern "C" {
_declspec(dllexport) auto ll_mod_load(ll::mod::NativeMod& self) -> bool { return lse::load(self); }

_declspec(dllexport) auto ll_mod_enable(ll::mod::NativeMod& self) -> bool { return lse::enable(self); }

// LegacyScriptEngine  should not be disabled or unloaded currently.
}
