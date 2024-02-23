#include "Entry.h"

#include "Config.h"
#include "PluginManager.h"
#include "PluginMigration.h"
#include "legacy/api/MoreGlobal.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/main/EconomicSystem.h"

#include <ScriptX/ScriptX.h>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <ll/api/Config.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/PluginManagerRegistry.h>
#include <memory>
#include <stdexcept>

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_LUA

constexpr auto BaseLibFileName = "BaseLib.lua";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS

constexpr auto BaseLibFileName = "BaseLib.js";

#endif

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

std::unique_ptr<std::reference_wrapper<ll::plugin::NativePlugin>>
    selfPluginInstance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void loadConfig(const ll::plugin::NativePlugin& self, Config& config);
void loadDebugEngine(const ll::plugin::NativePlugin& self);
void registerPluginManager(const std::shared_ptr<PluginManager>& pluginManager);

auto enable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    try {
        logger.info("enabling...");

        RegisterDebugCommand();

        logger.info("enabled");

        return true;

    } catch (const std::exception& error) {
        logger.error(fmt::format("failed to enable: {}", error.what()));
        return false;
    }
}

void initializeLegacyStuff() {
    InitLocalShareData();
    InitGlobalShareData();
    InitSafeGuardRecord();
    EconomySystem::init();

    InitBasicEventListeners();
    InitMessageSystem();
    MoreGlobal::Init();
}

auto load(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    try {
        logger.info("loading...");

        ll::i18n::load(self.getLangDir());

        config             = Config();
        pluginManager      = std::make_shared<PluginManager>();
        selfPluginInstance = std::make_unique<std::reference_wrapper<ll::plugin::NativePlugin>>(self);

        loadConfig(self, config);

        if (config.migratePlugins) {
            migratePlugins(*pluginManager);
        }

        registerPluginManager(pluginManager);

        // Legacy stuff should be initialized before any possible call to legacy code.
        initializeLegacyStuff();

        loadDebugEngine(self);

        logger.info("loaded");

        return true;

    } catch (const std::exception& error) {
        logger.error(fmt::format("failed to load: {}", error.what()));
        return false;
    }
}

void loadConfig(const ll::plugin::NativePlugin& self, Config& config) {
    const auto& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(config, configFilePath) && !ll::config::saveConfig(config, configFilePath)) {
        throw std::runtime_error(fmt::format("cannot save default configurations to {}", configFilePath));
    }
}

void loadDebugEngine(const ll::plugin::NativePlugin& self) {
    auto& scriptEngine = *EngineManager::newEngine();

    script::EngineScope engineScope(scriptEngine);

    BindAPIs(&scriptEngine);

    // Load BaseLib.
    auto baseLibPath    = self.getPluginDir() / "baselib" / BaseLibFileName;
    auto baseLibContent = ll::file_utils::readFile(baseLibPath);
    if (!baseLibContent) {
        throw std::runtime_error(fmt::format("failed to read BaseLib at {}", baseLibPath.string()));
    }
    scriptEngine.eval(baseLibContent.value());

    debugEngine = &scriptEngine;
}

void registerPluginManager(const std::shared_ptr<PluginManager>& pluginManager) {
    auto& pluginManagerRegistry = ll::plugin::PluginManagerRegistry::getInstance();

    if (!pluginManagerRegistry.addManager(pluginManager)) {
        throw std::runtime_error("failed to register plugin manager");
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

auto getSelfPluginInstance() -> ll::plugin::NativePlugin& {
    if (!selfPluginInstance) {
        throw std::runtime_error("selfPluginInstance is null");
    }

    return *selfPluginInstance;
}

} // namespace lse

extern "C" {
_declspec(dllexport) auto ll_plugin_load(ll::plugin::NativePlugin& self) -> bool { return lse::load(self); }

_declspec(dllexport) auto ll_plugin_enable(ll::plugin::NativePlugin& self) -> bool { return lse::enable(self); }

// LegacyScriptEngine  should not be disabled or unloaded currently.
}
