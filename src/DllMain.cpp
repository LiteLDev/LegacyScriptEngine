#include "main/BuiltinCommands.h"

#include <ll/api/plugin/NativePlugin.h>


extern void entry();

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) {
    entry();

    return true;
}

_declspec(dllexport) bool ll_plugin_enable(ll::plugin::NativePlugin& self) {
    RegisterDebugCommand();

    return true;
}
}
