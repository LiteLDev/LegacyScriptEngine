#pragma once
#include "ll/api/Expected.h"
#include "ll/api/base/Macro.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/ModManager.h"

#include <string_view>

namespace lse {

class PluginManager final : public ll::mod::ModManager {
public:
    PluginManager();
    ~PluginManager() override;

private:
    ll::Expected<> load(ll::mod::Manifest manifest) override;
    ll::Expected<> unload(std::string_view name) override;
};

} // namespace lse
