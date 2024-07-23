#pragma once

#include <expected>
#include <ll/api/mod/Manifest.h>
#include <ll/api/mod/ModManager.h>
#include <string_view>

namespace lse {

class PluginManager : public ll::mod::ModManager {
public:
    PluginManager();

    PluginManager(const PluginManager&)                    = delete;
    PluginManager(PluginManager&&)                         = delete;
    auto operator=(const PluginManager&) -> PluginManager& = delete;
    auto operator=(PluginManager&&) -> PluginManager&      = delete;

    ~PluginManager() override = default;

private:
    ll::Expected<> load(ll::mod::Manifest manifest) override;
    ll::Expected<> unload(std::string_view name) override;
};

} // namespace lse
