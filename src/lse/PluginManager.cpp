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

namespace {

auto loadPlugin(ll::plugin::Plugin& plugin, ll::Logger& logger) -> bool {
    const auto& manifest = plugin.getManifest();

    logger.info("loading plugin {}", manifest.name);

    auto pluginDir = std::filesystem::canonical(ll::plugin::getPluginsRoot() / manifest.name);
    auto entryPath = pluginDir / manifest.entry;

    if (!::PluginManager::loadPlugin(entryPath.string(), false, true)) {
        throw std::runtime_error(fmt::format("failed to load plugin {}", manifest.name));
    }

    return true;
}

auto unloadPlugin(ll::plugin::Plugin& plugin, ll::Logger& logger) -> bool {
    auto pluginName = plugin.getManifest().name;

    logger.info("unloading plugin {}", pluginName);

    if (!::PluginManager::unloadPlugin(pluginName)) {
        throw std::runtime_error(fmt::format("failed to unload plugin {}", pluginName));
    }

    return true;
}

} // namespace

PluginManager::PluginManager() : ll::plugin::PluginManager(PluginManagerName) {}

auto PluginManager::load(ll::plugin::Manifest manifest) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    if (hasPlugin(manifest.name)) {
        throw std::runtime_error("plugin already loaded");
    }

    auto plugin = std::make_shared<Plugin>(manifest);

    // Register callbacks.
    plugin->onLoad([&logger](ll::plugin::Plugin& plugin) -> bool { return loadPlugin(plugin, logger); });
    plugin->onUnload([&logger](ll::plugin::Plugin& plugin) -> bool { return unloadPlugin(plugin, logger); });

    if (!plugin->onLoad()) {
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

    if (!plugin->onUnload()) {
        throw std::runtime_error(fmt::format("failed to unload plugin {}", name));
    }

    if (!erasePlugin(name)) {
        throw std::runtime_error(fmt::format("failed to unregister plugin {}", name));
    }

    return false;
}

} // namespace lse
