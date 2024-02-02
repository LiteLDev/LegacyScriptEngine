#pragma once

#include "PluginManager.h"

#include <ll/api/plugin/Manifest.h>
#include <ll/api/plugin/Plugin.h>

namespace lse {

class Plugin : public ll::plugin::Plugin {
public:
    Plugin(ll::plugin::Manifest manifest);

private:
    friend PluginManager;
};

} // namespace lse
