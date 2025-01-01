#pragma once

#include "Config.h"
#include "PluginManager.h"
#include "ll/api/mod/NativeMod.h"

namespace lse {

[[nodiscard]] auto getConfig() -> const Config&;

[[nodiscard]] auto getPluginManager() -> PluginManager&;

[[nodiscard]] auto getSelfPluginInstance() -> ll::mod::NativeMod&;

} // namespace lse
