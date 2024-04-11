#include "Plugin.h"

#include "Entry.h"
#include "legacy/engine/EngineOwnData.h"

#include <ll/api/plugin/Manifest.h>

namespace lse {

Plugin::Plugin(const ll::plugin::Manifest& manifest) : ll::plugin::Plugin(manifest) {}

std::shared_ptr<ll::plugin::Plugin> Plugin::current() {
    return lse::getPluginManager().getPlugin(ENGINE_OWN_DATA()->pluginName);
}
} // namespace lse
