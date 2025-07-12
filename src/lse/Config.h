#pragma once

#include <optional>

namespace lse {

struct Config {
    int  version        = 1;
    bool migratePlugins = true;
#ifdef LSE_BACKEND_NODEJS
    // fix addons that build with node-gyp version < 11.1.0, default: true
    std::optional<bool> fixLegacyAddons{std::nullopt};
#endif
};

} // namespace lse
