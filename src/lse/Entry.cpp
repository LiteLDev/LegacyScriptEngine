#include "Entry.h"

#include "PluginManager.h"
#include "legacy/api/EventAPI.h"
#include "legacy/api/MoreGlobal.h"
#include "legacy/engine/GlobalShareData.h"
#include "legacy/engine/LocalShareData.h"
#include "legacy/engine/MessageSystem.h"
#include "legacy/main/BuiltinCommands.h"
#include "legacy/main/EconomicSystem.h"
#include "legacy/main/Loader.h"
#include "legacy/main/SafeGuardRecord.h"

#include <functional>
#include <ll/api/io/FileUtils.h>
#include <ll/api/plugin/NativePlugin.h>
#include <ll/api/plugin/PluginManagerRegistry.h>
#include <memory>

namespace lse {

namespace {

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

    return true;
}

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

    pluginManager = std::make_shared<PluginManager>();

    auto& pluginManagerRegistry = ll::plugin::PluginManagerRegistry::getInstance();

    if (!pluginManagerRegistry.addManager(pluginManager)) {
        throw std::runtime_error("failed to register plugin manager");
    }

    return true;
}

auto loadBaseLib(ll::plugin::NativePlugin& self) -> bool {
    auto path = self.getPluginDir() / "BaseLib.lua";

    auto content = ll::file_utils::readFile(path);

    if (!content) {
        throw std::runtime_error("failed to read " + path.string());
    }

    depends.emplace(path.string(), *content);
}

} // namespace lse
