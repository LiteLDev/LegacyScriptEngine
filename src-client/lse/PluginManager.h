#pragma once
#include "ll/api/Expected.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/ModManager.h"

#include <string_view>

namespace lse {

class PluginManager final : public ll::mod::ModManager {
public:
    PluginManager();
    ~PluginManager() override;
    void enableAllPlugins();
    void disableAllPlugins();

private:
    ll::Expected<> load(ll::mod::Manifest manifest) override;
    ll::Expected<> unload(std::string_view name) override;
    ll::Expected<> enable(std::string_view name) override;
    ll::Expected<> disable(std::string_view name) override;
    ll::Expected<> enableScriptPlugin(std::string_view name);
    ll::Expected<> disableScriptPlugin(std::string_view name);
};

} // namespace lse
