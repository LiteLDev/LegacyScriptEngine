#include "legacy/engine/EngineOwnData.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/Mod.h"
#include "lse/Entry.h"
#include "lse/ScriptPlugin.h"

namespace lse {

ScriptPlugin::ScriptPlugin(ll::mod::Manifest manifest) : ll::mod::Mod(std::move(manifest)) {}

ScriptPlugin::~ScriptPlugin() { release(); }

std::shared_ptr<ll::mod::Mod> ScriptPlugin::current() {
    return lse::LegacyScriptEngine::getInstance().getManager().getMod(getEngineOwnData()->pluginName);
}
} // namespace lse
