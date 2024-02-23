#include "Plugin.h"

#include <ll/api/plugin/Manifest.h>

namespace lse {

Plugin::Plugin(const ll::plugin::Manifest& manifest) : ll::plugin::Plugin(manifest) {}

} // namespace lse
