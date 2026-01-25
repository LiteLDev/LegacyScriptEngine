#pragma once

#include "ll/api/command/runtime/ParamKind.h"
#include "ll/api/mod/Manifest.h"
#include "ll/api/mod/Mod.h"
#include "lse/PluginManager.h"

namespace lse {

class ScriptPlugin : public ll::mod::Mod {
    friend PluginManager;

public:
    struct ParamInfo {
        std::string                  name;
        ll::command::ParamKind::Kind type;
        bool                         optional;
        std::string                  enumName;
        CommandParameterOption       option;
        std::string                  identifier;
    };

    std::unordered_map<std::string, std::vector<ParamInfo>> registeredCommands;

    explicit ScriptPlugin(ll::mod::Manifest manifest);
    ~ScriptPlugin();

    static std::shared_ptr<ll::mod::Mod> current();
};
} // namespace lse
