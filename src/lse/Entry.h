#pragma once

#include "Config.h"
#include "PluginManager.h"
#include "ll/api/mod/NativeMod.h"

namespace lse {

class LegacyScriptEngine {
public:
    static LegacyScriptEngine& getInstance();

    LegacyScriptEngine() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    [[nodiscard]] Config const& getConfig();

    [[nodiscard]] PluginManager& getManager();

    bool load();

    bool enable();

    bool disable();

    // bool unload();

private:
    ll::mod::NativeMod&            mSelf;
    Config                         config;        // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    std::shared_ptr<PluginManager> pluginManager; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
};

} // namespace lse
