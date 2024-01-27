#include "LegacyScriptEngine.h"

#include <ll/api/plugin/NativePlugin.h>

namespace lse {

LegacyScriptEngine::LegacyScriptEngine(ll::plugin::NativePlugin& self) : mSelf(self) {
    mSelf.getLogger().info("loading...");

    // Code for loading the plugin goes here.
}

ll::plugin::NativePlugin& LegacyScriptEngine::getSelf() const { return mSelf; }

bool LegacyScriptEngine::enable() {
    mSelf.getLogger().info("enabling...");

    // Code for enabling the plugin goes here.

    return true;
}


} // namespace lse
