#pragma once

#include "ll/api/i18n/I18nAPI.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "main/Configs.h"
#include "utils/JsonHelper.h"
#include <string>
#include <vector>

#include "ll/api/Logger.h"
#include "ll/api/service/Bedrock.h"
#include "mc/math/Vec3.h"
#include "mc/world/level/BlockPos.h"

using std::string;
using std::vector;

// 全局工具
extern ll::Logger   logger;
extern ordered_json globalConfig;

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

using ll::i18n::detail::tr;
using ll::i18n_literals::operator""_tr;

inline std::string DimId2Name(int dimid) {
    std::string name;
    switch (dimid) {
    case 0:
        name = tr("base.getDimName.0");
        break;
    case 1:
        name = tr("base.getDimName.1");
        break;
    case 2:
        name = tr("base.getDimName.2");
        break;
    default:
        name = tr("base.getDimName.unknown");
        break;
    }
    return name;
}

// 全局变量
extern bool isCmdRegisterEnabled;