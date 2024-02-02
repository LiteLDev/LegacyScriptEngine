#include "PluginManager.h"

#include "Entry.h"
#include "Plugin.h"
#include "legacy/main/PluginManager.h"

#include <fmt/format.h>
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

// temporary fix
// struct ll::plugin::PluginManager::Impl {
//     std::string                                 type;    // NOLINT
//     std::recursive_mutex                        mutex;   // NOLINT
//     UnorderedStringMap<std::shared_ptr<Plugin>> plugins; // NOLINT
//     explicit Impl(std::string type) : type(std::move(type)) {}
// };
// ll::plugin::PluginManager::~PluginManager() = default;
// auto ll::plugin::PluginManager::enable(std::string_view name) -> bool {
//     auto lockInstance(lock());
//     auto plugin = getPlugin(name);
//     if (!plugin) {
//         return false;
//     }
//     return plugin->onEnable();
// }
// auto ll::plugin::PluginManager::disable(std::string_view name) -> bool {
//     auto lockInstance(lock());
//     auto plugin = getPlugin(name);
//     if (!plugin) {
//         return false;
//     }
//     return plugin->onDisable();
// }
// auto ll::plugin::PluginManager::load(ll::plugin::Manifest /*manifest*/) -> bool { return true; } // NOLINT
// auto ll::plugin::PluginManager::unload(std::string_view /*name*/) -> bool { return true; }

namespace lse {

PluginManager::PluginManager() : ll::plugin::PluginManager(PluginManagerName) {}

auto PluginManager::load(ll::plugin::Manifest manifest) -> bool {
    auto& logger = getSelfPluginInstance().getLogger();

    if (hasPlugin(manifest.name)) {
        throw std::runtime_error("plugin already loaded");
    }

    auto plugin = std::make_shared<Plugin>(manifest);

    plugin->onLoad([&logger](ll::plugin::Plugin& plugin) -> bool {
        const auto& manifest = plugin.getManifest();

        logger.info("loading plugin {}", manifest.name);

        auto pluginDir = std::filesystem::canonical(ll::plugin::getPluginsRoot() / manifest.name);
        auto entryPath = pluginDir / manifest.entry;

        if (!::PluginManager::loadPlugin(entryPath.string(), false, true)) {
            throw std::runtime_error(fmt::format("failed to load plugin {}", manifest.name));
        }
        return true;
    });

    plugin->onUnload([&logger](ll::plugin::Plugin& plugin) -> bool {
        auto pluginName = plugin.getManifest().name;

        logger.info("unloading plugin {}", pluginName);

        if (!::PluginManager::unloadPlugin(pluginName)) {
            throw std::runtime_error(fmt::format("failed to unload plugin {}", pluginName));
        }

        return true;
    });

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
