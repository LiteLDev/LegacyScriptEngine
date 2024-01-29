#pragma once

#include <ll/api/plugin/NativePlugin.h>

namespace lse {

[[nodiscard]] auto getSelfPluginInstance() -> ll::plugin::NativePlugin&;

} // namespace lse
