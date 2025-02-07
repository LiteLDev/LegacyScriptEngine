#include "Plugin.h"

#include "Entry.h"
#include "legacy/engine/EngineOwnData.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/Mod.h"

namespace lse {

Plugin::Plugin(const ll::mod::Manifest& manifest) : ll::mod::Mod(std::move(manifest)) {}

Plugin::~Plugin() { release(); }

std::shared_ptr<ll::mod::Mod> Plugin::current() {
    return lse::LegacyScriptEngine::getInstance().getManager().getMod(getEngineOwnData()->pluginName);
}
} // namespace lse
