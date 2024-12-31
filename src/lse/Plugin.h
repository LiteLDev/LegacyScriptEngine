#pragma once

#include "PluginManager.h"

#include <ll/api/mod/Manifest.h>
#include <ll/api/mod/Mod.h>

namespace lse {

class Plugin : public ll::mod::Mod {
    friend PluginManager;

public:
    Plugin(const ll::mod::Manifest& manifest);
    ~Plugin();

    static std::shared_ptr<ll::mod::Mod> current();
};
} // namespace lse
