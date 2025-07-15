#pragma once

#include "ll/api/i18n/I18n.h"
#include "mc/deps/core/math/Vec3.h"
#include "mc/world/level/BlockPos.h"
class FloatVec4;

class IntVec4 {
public:
    int x, y, z;
    int dim;

    [[nodiscard]] inline BlockPos getBlockPos() const { return {x, y, z}; }

    [[nodiscard]] inline int getDimensionId() const { return dim; }

    inline IntVec4& operator=(FloatVec4 const&);
};

class FloatVec4 {
public:
    float x, y, z;
    int   dim;

    [[nodiscard]] inline Vec3 getVec3() const { return {x, y, z}; }

    [[nodiscard]] inline int getDimensionId() const { return dim; }

    [[nodiscard]] inline IntVec4 toIntVec4() const {
        IntVec4 pos{};
        pos = *this;
        return pos;
    }

    inline FloatVec4& operator=(IntVec4 const&);
};

inline IntVec4& IntVec4::operator=(FloatVec4 const& rhs) {
    x   = (int)floor(rhs.x);
    y   = (int)floor(rhs.y);
    z   = (int)floor(rhs.z);
    dim = rhs.dim;
    return *this;
};
inline FloatVec4& FloatVec4::operator=(IntVec4 const& rhs) {
    x   = (float)rhs.x;
    y   = (float)rhs.y;
    z   = (float)rhs.z;
    dim = rhs.dim;
    return *this;
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

#if defined(LSE_BACKEND_QUICKJS)
// QuickJs
inline constexpr std::string LLSE_BACKEND_TYPE = "Js";

#elif defined(LSE_BACKEND_LUA)
// Lua
inline constexpr std::string LLSE_BACKEND_TYPE = "Lua";

#elif defined(LSE_BACKEND_NODEJS)
// NodeJs
inline constexpr std::string LLSE_BACKEND_TYPE = "NodeJs";

#elif defined(LSE_BACKEND_PYTHON)
// Python
inline constexpr std::string LLSE_BACKEND_TYPE = "Python";
#endif

// Debug engine information
#if defined(LSE_BACKEND_NODEJS)
inline constexpr std::string LLSE_DEBUG_CMD = "nodejsdebug";
#elif defined(LSE_BACKEND_QUICKJS)
inline constexpr std::string LLSE_DEBUG_CMD = "jsdebug";
#elif defined(LSE_BACKEND_LUA)
inline constexpr std::string LLSE_DEBUG_CMD = "luadebug";
#elif defined(LSE_BACKEND_PYTHON)
inline constexpr std::string LLSE_DEBUG_CMD = "pydebug";
#endif

inline constexpr wchar_t       LLSE_GLOBAL_DATA_NAME[]                 = L"LLSE_GLOBAL_DATA_SECTION";
inline constexpr unsigned long LLSE_MESSAGE_SYSTEM_WAIT_CHECK_INTERVAL = 5;
inline constexpr size_t        LLSE_POOL_THREAD_COUNT                  = 4;
inline constexpr int           LLSE_VALID_BACKENDS_COUNT               = 4;
