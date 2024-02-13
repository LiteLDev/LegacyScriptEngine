#pragma once

#include <ll/api/plugin/Manifest.h>
#include <ll/api/plugin/Plugin.h>

namespace lse {

class Plugin : public ll::plugin::Plugin {
public:
    Plugin(const ll::plugin::Manifest& manifest);
};

} // namespace lse
