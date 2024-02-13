#pragma once

#include <string_view>

class PluginManager {
public:
    static bool unloadPlugin(std::string_view name);
};
