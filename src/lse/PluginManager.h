#pragma once

#include <ll/api/plugin/Manifest.h>
#include <ll/api/plugin/PluginManager.h>
#include <string_view>

namespace lse {

class PluginManager : public ll::plugin::PluginManager {
public:
    PluginManager();

    PluginManager(const PluginManager&)                    = delete;
    PluginManager(PluginManager&&)                         = delete;
    auto operator=(const PluginManager&) -> PluginManager& = delete;
    auto operator=(PluginManager&&) -> PluginManager&      = delete;

    ~PluginManager() override = default;

private:
    ll::Expected<> load(ll::plugin::Manifest manifest) override;
    ll::Expected<> unload(std::string_view name) override;
};

} // namespace lse
