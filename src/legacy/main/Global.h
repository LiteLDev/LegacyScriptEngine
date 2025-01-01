#pragma once

#include "ll/api/i18n/I18n.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockPos.h"

#include <string>
#include <vector>

class IntVec4 {
public:
    int x, y, z;
    int dim;

    inline BlockPos getBlockPos() { return {x, y, z}; }

    inline int getDimensionId() { return dim; }
};

class FloatVec4 {
public:
    float x, y, z;
    int   dim;

    inline Vec3 getVec3() { return {x, y, z}; }

    inline int getDimensionId() { return dim; }

    inline IntVec4 toIntVec4() {
        auto px = (int)x;
        auto py = (int)y;
        auto pz = (int)z;
        if (px < 0 && px != x) px = px - 1;
        if (py < 0 && py != y) py = py - 1;
        if (pz < 0 && pz != z) pz = pz - 1;
        return {px, py, pz, dim};
    }
};

using namespace ll::i18n_literals;

inline std::string DimId2Name(int dimid) {
    std::string name;
    switch (dimid) {
    case 0:
        name = "Overworld"_tr();
        break;
    case 1:
        name = "Nether"_tr();
        break;
    case 2:
        name = "End"_tr();
        break;
    default:
        name = "Other dimension"_tr();
        break;
    }
    return name;
}

// 全局变量
extern bool isCmdRegisterEnabled;

#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS)
// QuickJs
constexpr std::string LLSE_BACKEND_TYPE = "Js";

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_LUA)
// Lua
constexpr std::string LLSE_BACKEND_TYPE = "Lua";

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
// NodeJs
const std::string LLSE_BACKEND_TYPE = "NodeJs";

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
// Python
const std::string LLSE_BACKEND_TYPE = "Python";
#endif

// Debug engine information
#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
constexpr std::string LLSE_DEBUG_CMD = "nodejsdebug";
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS)
constexpr std::string LLSE_DEBUG_CMD = "jsdebug";
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_LUA)
constexpr std::string LLSE_DEBUG_CMD = "luadebug";
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
constexpr std::string LLSE_DEBUG_CMD = "pydebug";
#endif

constexpr wchar_t       LLSE_GLOBAL_DATA_NAME[]                 = L"LLSE_GLOBAL_DATA_SECTION";
constexpr unsigned long LLSE_MESSAGE_SYSTEM_WAIT_CHECK_INTERVAL = 5;
constexpr size_t        LLSE_POOL_THREAD_COUNT                  = 4;
constexpr int           LLSE_VALID_BACKENDS_COUNT               = 4;
constexpr auto          LLSE_NPM_EXECUTE_PATH                   = "plugins/legacy-script-engine-nodejs";