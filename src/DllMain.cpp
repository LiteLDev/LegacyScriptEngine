#include <ll/api/plugin/NativePlugin.h>

extern void entry();

extern "C" {
_declspec(dllexport) bool ll_plugin_load(ll::plugin::NativePlugin& self) {
    entry();

    return true;
}
}
