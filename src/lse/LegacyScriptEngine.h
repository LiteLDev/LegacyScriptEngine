#pragma once

#include <ll/api/plugin/NativePlugin.h>

namespace lse {

class LegacyScriptEngine {
public:
    explicit LegacyScriptEngine(ll::plugin::NativePlugin& self);

    LegacyScriptEngine(LegacyScriptEngine&&)                 = delete;
    LegacyScriptEngine(const LegacyScriptEngine&)            = delete;
    LegacyScriptEngine& operator=(LegacyScriptEngine&&)      = delete;
    LegacyScriptEngine& operator=(const LegacyScriptEngine&) = delete;

    ~LegacyScriptEngine() = default;

    [[nodiscard]] ll::plugin::NativePlugin& getSelf() const;

    bool enable();

private:
    ll::plugin::NativePlugin& mSelf;
};

} // namespace lse
