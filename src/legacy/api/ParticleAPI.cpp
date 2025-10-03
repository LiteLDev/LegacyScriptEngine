//
// Created by OEOTYAN on 2022/08/27.
//
#include "ParticleAPI.h"

#include "McAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/math/Color.h"
#include "mc/network/packet/SpawnParticleEffectPacket.h"
#include "mc/util/MolangVariable.h"
#include "mc/util/MolangVariableMap.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/phys/AABB.h"

#define GETVEC3(v, d, u)                                                                                               \
    if (IsInstanceOf<IntPos>(u)) {                                                                                     \
        auto pos2  = EngineScope::currentEngine()->getNativeInstance<IntPos>(u);                                       \
        v          = Vec3(pos2->x, pos2->y, pos2->z);                                                                  \
        v         += 0.5f;                                                                                             \
        d          = pos2->dim;                                                                                        \
    } else if (IsInstanceOf<FloatPos>(u)) {                                                                            \
        auto pos2 = EngineScope::currentEngine()->getNativeInstance<FloatPos>(u);                                      \
        v         = Vec3(pos2->x, pos2->y, pos2->z);                                                                   \
        d         = pos2->dim;                                                                                         \
    } else {                                                                                                           \
        LOG_WRONG_ARG_TYPE(__FUNCTION__);                                                                              \
        return Local<Value>();                                                                                         \
    }

namespace {
template <typename T>
std::string fto_string(const T a_value) {
    std::ostringstream out;
    out << a_value;
    return out.str();
}
ParticleSpawner::ColorPalette getColorType(std::string s) {
    switch (s[0]) {
    case 'B':
        return ParticleSpawner::ColorPalette::BLACK;
        break;
    case 'I':
        return ParticleSpawner::ColorPalette::INDIGO;
        break;
    case 'L':
        return ParticleSpawner::ColorPalette::LAVENDER;
        break;
    case 'T':
        return ParticleSpawner::ColorPalette::TEAL;
        break;
    case 'C':
        return ParticleSpawner::ColorPalette::COCOA;
        break;
    case 'D':
        return ParticleSpawner::ColorPalette::DARK;
        break;
    case 'O':
        return ParticleSpawner::ColorPalette::OATMEAL;
        break;
    case 'W':
        return ParticleSpawner::ColorPalette::WHITE;
        break;
    case 'R':
        return ParticleSpawner::ColorPalette::RED;
        break;
    case 'A':
        return ParticleSpawner::ColorPalette::APRICOT;
        break;
    case 'Y':
        return ParticleSpawner::ColorPalette::YELLOW;
        break;
    case 'G':
        return ParticleSpawner::ColorPalette::GREEN;
        break;
    case 'V':
        return ParticleSpawner::ColorPalette::VATBLUE;
        break;
    case 'S':
        return ParticleSpawner::ColorPalette::SLATE;
        break;
    case 'P':
        return ParticleSpawner::ColorPalette::PINK;
        break;
    case 'F':
        return ParticleSpawner::ColorPalette::FAWN;
        break;
    default:
        return ParticleSpawner::ColorPalette::WHITE;
        break;
    }
}
} // namespace

ParticleSpawner* ParticleSpawner::create(const Arguments& args) {
    if (args.size() < 3) return nullptr;
    try {
        ParticleSpawner* p = new ParticleSpawner(args.thiz());
        return p;
    } catch (...) {
        return nullptr;
    }
}
Local<Value> McClass::newParticleSpawner(const Arguments& args) {
    auto         size          = args.size();
    unsigned int displayRadius = UINT_MAX;
    bool         highDetial    = true;
    bool         doubleSide    = true;

    if (size > 0) {
        CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
        displayRadius = args[0].asNumber().toInt64();
    }
    if (size > 1) {
        CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);
        highDetial = args[1].asBoolean().value();
    }
    if (size > 2) {
        CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);
        doubleSide = args[2].asBoolean().value();
    }
    return EngineScope::currentEngine()->newNativeClass<ParticleSpawner>(displayRadius, highDetial, doubleSide);
}

Local<Value> ParticleSpawner::spawnParticle(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    Vec3        pos;
    int         dimId;
    std::string particleName;
    GETVEC3(pos, dimId, args[0])
    CHECK_ARG_TYPE(args[1], ValueKind::kString);
    particleName = args[1].asString().toString();
    auto dim     = ll::service::getLevel()->getDimension(dimId).lock();
    if (dim) {
        dim->forEachPlayer([&](Player& player) {
            SpawnParticleEffectPacket pkt(pos, particleName, dimId, std::nullopt);
            player.sendNetworkPacket(pkt);
            return true;
        });
    }
    return Boolean::newBoolean(true);
}
Local<Value> ParticleSpawner::drawPoint(const Arguments& args) { return Boolean::newBoolean(true); }

Local<Value> ParticleSpawner::drawNumber(const Arguments& args) { return Boolean::newBoolean(true); }
Local<Value> ParticleSpawner::drawAxialLine(const Arguments& args) { return Boolean::newBoolean(true); }
Local<Value> ParticleSpawner::drawOrientedLine(const Arguments& args) { return Boolean::newBoolean(true); }
Local<Value> ParticleSpawner::drawCuboid(const Arguments& args) { return Boolean::newBoolean(true); }
Local<Value> ParticleSpawner::drawCircle(const Arguments& args) { return Boolean::newBoolean(true); }

ClassDefine<ParticleSpawner> ParticleSpawnerBuilder =
    defineClass<ParticleSpawner>("ParticleSpawner")
        .constructor(&ParticleSpawner::create)
        .instanceProperty("displayRadius", &ParticleSpawner::getDisplayRadius, &ParticleSpawner::setDisplayRadius)
        .instanceProperty("highDetial", &ParticleSpawner::getHighDetial, &ParticleSpawner::setHighDetial)
        .instanceProperty("doubleSide", &ParticleSpawner::getDoubleSide, &ParticleSpawner::setDoubleSide)

        .instanceFunction("spawnParticle", &ParticleSpawner::spawnParticle)
        .instanceFunction("drawPoint", &ParticleSpawner::drawPoint)
        .instanceFunction("drawNumber", &ParticleSpawner::drawNumber)
        .instanceFunction("drawAxialLine", &ParticleSpawner::drawAxialLine)
        .instanceFunction("drawOrientedLine", &ParticleSpawner::drawOrientedLine)
        .instanceFunction("drawCuboid", &ParticleSpawner::drawCuboid)
        .instanceFunction("drawCircle", &ParticleSpawner::drawCircle)
        .build();
// clang-format off
ClassDefine<void> ParticleColorBuilder =
    defineClass("ParticleColor")
        .property("Black",    []() { return String::newString("B"); })
        .property("Indigo",   []() { return String::newString("I"); })
        .property("Lavender", []() { return String::newString("L"); })
        .property("Teal",     []() { return String::newString("T"); })
        .property("Cocoa",    []() { return String::newString("C"); })
        .property("Dark",     []() { return String::newString("D"); })
        .property("Oatmeal",  []() { return String::newString("O"); })
        .property("White",    []() { return String::newString("W"); })
        .property("Red",      []() { return String::newString("R"); })
        .property("Apricot",  []() { return String::newString("A"); })
        .property("Yellow",   []() { return String::newString("Y"); })
        .property("Green",    []() { return String::newString("G"); })
        .property("Vatblue",  []() { return String::newString("V"); })
        .property("Slate",    []() { return String::newString("S"); })
        .property("Pink",     []() { return String::newString("P"); })
        .property("Fawn",     []() { return String::newString("F"); })
        .build();
// clang-format on

// clang-format off
ClassDefine<void> DirectionBuilder =
    defineClass("Direction")
        .property("NEG_Y", []() { return Number::newNumber(0); })
        .property("POS_Y", []() { return Number::newNumber(1); })
        .property("NEG_Z", []() { return Number::newNumber(2); })
        .property("POS_Z", []() { return Number::newNumber(3); })
        .property("NEG_X", []() { return Number::newNumber(4); })
        .property("POS_X", []() { return Number::newNumber(5); })
        .build();
// clang-format on
