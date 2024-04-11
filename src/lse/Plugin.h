#pragma once

#include "PluginManager.h"

#include <ll/api/plugin/Manifest.h>
#include <ll/api/plugin/Plugin.h>

namespace lse {

class Plugin : public ll::plugin::Plugin {
    friend PluginManager;

public:
    Plugin(const ll::plugin::Manifest& manifest);

    static std::shared_ptr<ll::plugin::Plugin> current();
};
} // namespace lse
