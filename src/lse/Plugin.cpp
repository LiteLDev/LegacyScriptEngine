#include "Plugin.h"

#include <ll/api/plugin/Manifest.h>
#include <utility>

namespace lse {

Plugin::Plugin(ll::plugin::Manifest manifest) : ll::plugin::Plugin(std::move(manifest)) {}

} // namespace lse
