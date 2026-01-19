#pragma once

#include <optional>

#include "mc/server/commands/CommandPermissionLevel.h"

namespace lse {

struct Config {
    int  version        = 3;
    bool migratePlugins = true;
#ifdef LSE_BACKEND_NODEJS
    // fix addons that build with node-gyp version < 11.1.0, default: true
    std::optional<bool> fixLegacyAddons{std::nullopt};
#endif
    CommandPermissionLevel debugCommandLevel = CommandPermissionLevel::GameDirectors;
};

} // namespace lse
