#pragma once

#include "ll/api/mod/NativeMod.h"
#include "lse/Config.h"
#include "lse/PluginManager.h"

namespace lse {

class LegacyScriptEngine {
public:
    static LegacyScriptEngine& getInstance();

    static inline ll::io::Logger& getLogger() { return getInstance().getSelf().getLogger(); }

    LegacyScriptEngine() : mSelf(*ll::mod::NativeMod::current()) {}

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }

    [[nodiscard]] Config const& getConfig();

    [[nodiscard]] PluginManager& getManager();

    bool load();

    bool enable();

    bool unload();

private:
    ll::mod::NativeMod&            mSelf;
    Config                         config;        // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
    std::shared_ptr<PluginManager> pluginManager; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
};

} // namespace lse
