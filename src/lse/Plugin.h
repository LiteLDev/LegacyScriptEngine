#pragma once

#include "PluginManager.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/Mod.h"
#include "ll/api/command/runtime/ParamKind.h"

namespace lse {

class Plugin : public ll::mod::Mod {
    friend PluginManager;

public:
    struct ParamInfo {
        std::string                  name;
        ll::command::ParamKind::Kind type;
        bool                         optional;
        std::string                  enumName;
        CommandParameterOption       option;
    };

    std::unordered_map<std::string, std::vector<ParamInfo>> registeredCommands;

    Plugin(const ll::mod::Manifest& manifest);
    ~Plugin();

    static std::shared_ptr<ll::mod::Mod> current();
};
} // namespace lse
