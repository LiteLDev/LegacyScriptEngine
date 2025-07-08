#pragma once

#include <optional>

namespace lse {

struct Config {
    int  version        = 1;
    bool migratePlugins = true;
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
    std::optional<bool> fixOldAddon{std::nullopt};
#endif
};

} // namespace lse
