#pragma once

#include "Config.h"
#include "PluginManager.h"
#include "ll/api/mod/NativeMod.h"

namespace lse {

[[nodiscard]] Config const& getConfig();

[[nodiscard]] PluginManager& getPluginManager();

[[nodiscard]] ll::mod::NativeMod& getSelfModInstance();

} // namespace lse
