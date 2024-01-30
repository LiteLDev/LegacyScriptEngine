#include "Entry.h"

#include "Config.h"
#include "PluginManager.h"
#include "PluginMigration.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/MoreGlobal.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/engine/LocalShareData.h"
#include "legacy/engine/MessageSystem.h"
#include "legacy/main/BuiltinCommands.h"
#include "legacy/main/EconomicSystem.h"
#include "legacy/main/Loader.h"
#include "legacy/main/SafeGuardRecord.h"

#include <fmt/format.h>
#include <functional>
#include <ll/api/Config.h>
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

namespace lse {

namespace {

Config config; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::shared_ptr<PluginManager> pluginManager; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

std::unique_ptr<std::reference_wrapper<ll::plugin::NativePlugin>>
    selfPluginInstance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

} // namespace

auto enable(ll::plugin::NativePlugin& self) -> bool;
auto load(ll::plugin::NativePlugin& self) -> bool;
auto loadBaseLib(ll::plugin::NativePlugin& self) -> bool;

extern "C" {

_declspec(dllexport) auto ll_plugin_load(ll::plugin::NativePlugin& self) -> bool { return load(self); }

_declspec(dllexport) auto ll_plugin_enable(ll::plugin::NativePlugin& self) -> bool { return enable(self); }

// LegacyScriptEngine should not be disabled or unloaded.
}

auto enable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("enabling...");

    RegisterDebugCommand();

    logger.info("enabled");

    return true;
}

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

auto load(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    logger.info("loading...");

    selfPluginInstance = std::make_unique<std::reference_wrapper<ll::plugin::NativePlugin>>(self);

    // Load configuration.
    const auto& configFilePath = self.getConfigDir() / "config.json";
    if (!ll::config::loadConfig(config, configFilePath)) {
        logger.warn("cannot load configurations from {}", configFilePath);
        logger.info("saving default configurations to {}", configFilePath);

        if (!ll::config::saveConfig(config, configFilePath)) {
            throw std::runtime_error(fmt::format("cannot save default configurations to {}", configFilePath));
        }
    }

    // Initialize LLSE stuff.
    ll::i18n::load(self.getLangDir());

    InitLocalShareData();
    InitGlobalShareData();
    InitSafeGuardRecord();
    EconomySystem::init();

    loadBaseLib(self);

    LoadDebugEngine();

    InitBasicEventListeners();
    InitMessageSystem();
    MoreGlobal::Init();

    // Register plugin manager.
    pluginManager = std::make_shared<PluginManager>();

    auto& pluginManagerRegistry = ll::plugin::PluginManagerRegistry::getInstance();

    if (!pluginManagerRegistry.addManager(pluginManager)) {
        throw std::runtime_error("failed to register plugin manager");
    }

    // Migrate plugins if needed.
    // Must execute after plugin manager is registered.
    if (config.migratePlugins) {
        migratePlugins();
    }

    logger.info("loaded");

    return true;
}

auto loadBaseLib(ll::plugin::NativePlugin& self) -> bool {
    auto path = self.getPluginDir() / "baselib" / BaseLibFileName;

    auto content = ll::file_utils::readFile(path);

    if (!content) {
        throw std::runtime_error(fmt::format("failed to read {}", path.string()));
    }

    depends.emplace(path.string(), *content);

    return true;
}

} // namespace lse
