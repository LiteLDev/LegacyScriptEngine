#include "PluginManager.h"

#include "ll/api/plugin/Plugin.h"
#include "lse/Entry.h"
#include "main/PluginManager.h"

#include <ll/api/plugin/PluginManager.h>

#if LEGACY_SCRIPT_ENGINE_BACKEND == lua

constexpr auto PluginManagerName = "lse-lua";

#elif LEGACY_SCRIPT_ENGINE_BACKEND == quickjs

constexpr auto PluginManagerName = "lse-quickjs";

#else

#error "LEGACY_SCRIPT_ENGINE_BACKEND is not defined!"

#endif

namespace lse {

PluginManager::PluginManager() : ll::plugin::PluginManager(PluginManagerName) {}

auto PluginManager::load(ll::plugin::Manifest manifest) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("loading plugin {}", manifest.name);

    if (hasPlugin(manifest.name)) {
        logger.error("plugin {} already loaded", manifest.name);
        return false;
    }

    auto pluginDir = std::filesystem::canonical(ll::plugin::getPluginsRoot() / manifest.name);

    auto entryPath = pluginDir / manifest.entry;

    return ::PluginManager::loadPlugin(entryPath.string(), false, true);
}

auto PluginManager::unload(std::string_view name) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("unloading plugin {}", name);

    // TODO: Unload plugin

    return false;
}

} // namespace lse
