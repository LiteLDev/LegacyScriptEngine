#include "PluginManager.h"

#include "Entry.h"
#include "Plugin.h"
#include "legacy/main/PluginManager.h"

#include <fmt/format.h>
#include <ll/api/Logger.h>
#include <ll/api/plugin/Plugin.h>
#include <ll/api/plugin/PluginManager.h>
#include <memory>
#include <stdexcept>

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_LUA

constexpr auto PluginManagerName = "lse-lua";

#endif

#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS

constexpr auto PluginManagerName = "lse-quickjs";

#endif

namespace lse {

PluginManager::PluginManager() : ll::plugin::PluginManager(PluginManagerName) {}

auto PluginManager::load(ll::plugin::Manifest manifest) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    logger.info("loading plugin {}", manifest.name);

    if (hasPlugin(manifest.name)) {
        throw std::runtime_error("plugin already loaded");
    }

    auto plugin = std::make_shared<Plugin>(manifest);

    auto pluginDir = std::filesystem::canonical(ll::plugin::getPluginsRoot() / manifest.name);
    auto entryPath = pluginDir / manifest.entry;

    if (!::PluginManager::loadPlugin(entryPath.string(), false, true)) {
        throw std::runtime_error(fmt::format("failed to load plugin {}", manifest.name));
    }

    if (!addPlugin(manifest.name, plugin)) {
        throw std::runtime_error(fmt::format("failed to register plugin {}", manifest.name));
    }

    return true;
}

auto PluginManager::unload(std::string_view name) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    auto plugin = std::static_pointer_cast<Plugin>(getPlugin(name));

    logger.info("unloading plugin {}", name);

    if (!::PluginManager::unloadPlugin(std::string(name))) {
        throw std::runtime_error(fmt::format("failed to unload plugin {}", name));
    }

    if (!erasePlugin(name)) {
        throw std::runtime_error(fmt::format("failed to unregister plugin {}", name));
    }

    return false;
}

} // namespace lse
