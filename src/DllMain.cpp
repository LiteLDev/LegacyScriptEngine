#include "main/BuiltinCommands.h"

#include <ll/api/plugin/NativePlugin.h>


extern void LSE_Load();
extern void LSE_Enable();

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) {
    LSE_Load();
    return true;
}

_declspec(dllexport) bool ll_plugin_enable(ll::plugin::NativePlugin& self) {
    LSE_Enable();
    return true;
}
}
