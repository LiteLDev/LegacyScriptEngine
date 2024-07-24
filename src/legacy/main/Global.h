#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"

#include <ll/api/Expected.h>
#include <ll/api/Logger.h>
#include <ll/api/i18n/I18n.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

// 全局工具
extern nlohmann::ordered_json globalConfig;

typedef unsigned long long QWORD;

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

using ll::i18n_literals::operator""_tr;

inline std::string DimId2Name(int dimid) {
    std::string name;
    switch (dimid) {
    case 0:
        name = "base.getDimName.0"_tr();
        break;
    case 1:
        name = "base.getDimName.1"_tr();
        break;
    case 2:
        name = "base.getDimName.2"_tr();
        break;
    default:
        name = "base.getDimName.unknown"_tr();
        break;
    }
    return name;
}

// 全局变量
extern bool isCmdRegisterEnabled;
