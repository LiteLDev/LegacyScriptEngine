#include "Entry.h"

#include "Config.h"
#include "PluginManager.h"
#include "PluginMigration.h"
#include "legacy/api/MoreGlobal.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/main/EconomicSystem.h"
#include "legacy/main/PythonHelper.h"

#include <ScriptX/ScriptX.h>
#include <exception>
#include <fmt/format.h>
#include <functional>
#include <ll/api/Config.h>
#include <ll/api/Logger.h>
#include <ll/api/i18n/I18n.h>
#include <ll/api/io/FileUtils.h>
#include <ll/api/mod/ModManagerRegistry.h>
#include <ll/api/mod/NativeMod.h>
#include <ll/api/utils/ErrorUtils.h>
#include <ll/api/utils/StringUtils.h>
#include <memory>
#include <processenv.h>
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

    try {
        logger.info("enabling...");
#ifndef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
        RegisterDebugCommand();
#endif
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
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    // This fix is used for Python3.10's bug:
    // The thread will freeze when creating a new engine while another thread is blocking to read stdin
    // Side effects: sys.stdin cannot be used after this patch.
    // More info to see: https://github.com/python/cpython/issues/83526
    //
    // Attention! When CPython is upgraded, this fix must be re-adapted or removed!!
    //
    // PythonHelper::FixPython310Stdin::patchPython310CreateStdio();
    PythonHelper::initPythonRuntime();
#endif

    InitBasicEventListeners();
    InitMessageSystem();
    MoreGlobal::Init();
}

auto load(ll::mod::NativeMod& self) -> bool {
    auto& logger = self.getLogger();
#ifdef NDEBUG
    ll::error_utils::setSehTranslator();
#endif

    try {
        logger.info("loading...");

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

        logger.info("loaded");

        return true;

    } catch (const std::exception& error) {
        logger.error(fmt::format("failed to load: {}", error.what()));
        return false;
    }
}

void loadConfig(const ll::mod::NativeMod& self, Config& config) {
    const auto& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(config, configFilePath) && !ll::config::saveConfig(config, configFilePath)) {
        throw std::runtime_error(fmt::format("cannot save default configurations to {}", configFilePath));
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
        throw std::runtime_error(fmt::format("failed to read BaseLib at {}", baseLibPath.string()));
    }
    scriptEngine.eval(baseLibContent.value());

    debugEngine = &scriptEngine;
#endif
}

void registerPluginManager(const std::shared_ptr<PluginManager>& pluginManager) {
    auto& pluginManagerRegistry = ll::mod::ModManagerRegistry::getInstance();

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
