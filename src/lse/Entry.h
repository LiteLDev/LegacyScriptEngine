#pragma once

#include "Config.h"
#include "PluginManager.h"

#include <ll/api/plugin/NativePlugin.h>

namespace lse {

[[nodiscard]] auto getConfig() -> const Config&;

[[nodiscard]] auto getPluginManager() -> PluginManager&;

[[nodiscard]] auto getSelfPluginInstance() -> ll::plugin::NativePlugin&;

} // namespace lse
