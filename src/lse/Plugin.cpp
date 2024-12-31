#include "Plugin.h"

#include "Entry.h"
#include "legacy/engine/EngineOwnData.h"
#include "ll/api/mod/Mod.h"

#include <ll/api/mod/Manifest.h>

namespace lse {

Plugin::Plugin(const ll::mod::Manifest& manifest) : ll::mod::Mod(manifest) {}

std::shared_ptr<ll::mod::Mod> Plugin::current() {
    return lse::getPluginManager().getMod(getEngineOwnData()->pluginName);
}
} // namespace lse
