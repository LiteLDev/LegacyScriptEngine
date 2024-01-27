#include "LegacyScriptEngine.h"

#include <ll/api/plugin/NativePlugin.h>
#include <memory>


namespace lse {

namespace {
std::unique_ptr<LegacyScriptEngine> legacyScriptEngine; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
} // namespace

LegacyScriptEngine& getLegacyScriptEngine() { return *legacyScriptEngine; }

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) {
    legacyScriptEngine = std::make_unique<LegacyScriptEngine>(self);
    return true;
}

_declspec(dllexport) bool ll_plugin_enable(ll::plugin::NativePlugin& /*unused*/) {
    return legacyScriptEngine->enable();
}

// LegacyScriptEngine should not be disabled or unloaded.
}

} // namespace lse
