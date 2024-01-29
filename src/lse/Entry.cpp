#include "Entry.h"

#include "api/EventAPI.h"
#include "api/MoreGlobal.h"
#include "engine/GlobalShareData.h"
#include "engine/LocalShareData.h"
#include "engine/MessageSystem.h"
#include "main/EconomicSystem.h"
#include "main/Loader.h"
#include "main/SafeGuardRecord.h"

#include <functional>
#include <ll/api/io/FileUtils.h>
#include <ll/api/plugin/NativePlugin.h>
#include <memory>

namespace lse {

namespace {

std::unique_ptr<std::reference_wrapper<ll::plugin::NativePlugin>>
    selfPluginInstance; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

} // namespace

auto enable(ll::plugin::NativePlugin& /*self*/) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("enabling...");

    return true;
}

auto getSelfPluginInstance() -> ll::plugin::NativePlugin& {
    if (!selfPluginInstance) {
        throw std::runtime_error("selfPluginInstance is null");
    }

    return *selfPluginInstance;
}

auto loadBaseLib() -> bool;
auto load(ll::plugin::NativePlugin& self) -> bool {
    auto& logger = self.getLogger();

    logger.info("loading...");

    selfPluginInstance = std::make_unique<std::reference_wrapper<ll::plugin::NativePlugin>>(self);

    ll::i18n::load(self.getLangDir());

    InitLocalShareData();
    InitGlobalShareData();
    InitSafeGuardRecord();
    EconomySystem::init();

    loadBaseLib();

    // TODO: Load main

    LoadDebugEngine();

    InitBasicEventListeners();
    InitMessageSystem();
    MoreGlobal::Init();

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

extern "C" {

_declspec(dllexport) auto ll_plugin_load(ll::plugin::NativePlugin& self) -> bool { return load(self); }

_declspec(dllexport) auto ll_plugin_enable(ll::plugin::NativePlugin& self) -> bool { return enable(self); }

// LegacyScriptEngine should not be disabled or unloaded.
}

} // namespace lse
