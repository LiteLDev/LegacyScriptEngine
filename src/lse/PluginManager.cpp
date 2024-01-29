#include "PluginManager.h"

#include "Entry.h"
#include "legacy/main/PluginManager.h"

#include <ll/api/plugin/Plugin.h>
#include <ll/api/plugin/PluginManager.h>

#if LEGACY_SCRIPT_ENGINE_BACKEND == lua

constexpr auto PluginManagerName = "lse-lua";

#elif LEGACY_SCRIPT_ENGINE_BACKEND == quickjs

constexpr auto PluginManagerName = "lse-quickjs";

#else

#error "LEGACY_SCRIPT_ENGINE_BACKEND is not defined!"

#endif

// temporary fix
struct ll::plugin::PluginManager::Impl {
    std::string                                 type;    // NOLINT
    std::recursive_mutex                        mutex;   // NOLINT
    UnorderedStringMap<std::shared_ptr<Plugin>> plugins; // NOLINT
    explicit Impl(std::string type) : type(std::move(type)) {}
};
ll::plugin::PluginManager::~PluginManager() = default;
auto ll::plugin::PluginManager::enable(std::string_view name) -> bool {
    auto lockInstance(lock());
    auto plugin = getPlugin(name);
    if (!plugin) {
        return false;
    }
    return plugin->onEnable();
}
auto ll::plugin::PluginManager::disable(std::string_view name) -> bool {
    auto lockInstance(lock());
    auto plugin = getPlugin(name);
    if (!plugin) {
        return false;
    }
    return plugin->onDisable();
}
auto ll::plugin::PluginManager::load(ll::plugin::Manifest /*manifest*/) -> bool { return true; } // NOLINT
auto ll::plugin::PluginManager::unload(std::string_view /*name*/) -> bool { return true; }

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
