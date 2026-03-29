//
// Created by OEOTYAN on 2022/08/27.
//
#pragma once
#include "BaseAPI.h"

class ParticleSpawner : public ScriptClass {
public:
    enum Direction : char {
        NEG_Y = 0,
        POS_Y = 1,
        NEG_Z = 2,
        POS_Z = 3,
        NEG_X = 4,
        POS_X = 5,
    };

    enum PointSize : char {
        PX1  = 1,
        PX2  = 2,
        PX4  = 4,
        PX8  = 8,
        PX16 = 16,
    };

    enum NumType : char {
        NUM0  = 0,
        NUM1  = 1,
        NUM2  = 2,
        NUM3  = 3,
        NUM4  = 4,
        NUM5  = 5,
        NUM6  = 6,
        NUM7  = 7,
        NUM8  = 8,
        NUM9  = 9,
        NUMA  = 'A',
        NUMB  = 'B',
        NUMC  = 'C',
        NUMD  = 'D',
        NUME  = 'E',
        NUMF  = 'F',
        NUM10 = 10,
        NUM11 = 11,
        NUM12 = 12,
        NUM13 = 13,
        NUM14 = 14,
        NUM15 = 15,
        NUM16 = 16,
    };
    enum class ColorPalette {
        BLACK,
        INDIGO,
        LAVENDER,
        TEAL,
        COCOA,
        DARK,
        OATMEAL,
        WHITE,
        RED,
        APRICOT,
        YELLOW,
        GREEN,
        VATBLUE,
        SLATE,
        PINK,
        FAWN,
    };

    explicit ParticleSpawner(Local<Object> const& scriptObj) : ScriptClass(scriptObj) {}

    static ParticleSpawner* create(Arguments const& args);

    Local<Value> getDisplayRadius() { return Number::newNumber(static_cast<long long>(1024)); }
    Local<Value> getHighDetial() { return Boolean::newBoolean(true); }
    Local<Value> getDoubleSide() { return Boolean::newBoolean(false); }
    void         setDisplayRadius(Local<Value> const& value) {}
    void         setHighDetial(Local<Value> const& value) {}
    void         setDoubleSide(Local<Value> const& value) {}

    Local<Value> spawnParticle(Arguments const& args);

    Local<Value> drawPoint(Arguments const& args);
    Local<Value> drawNumber(Arguments const& args);
    Local<Value> drawAxialLine(Arguments const& args);
    Local<Value> drawOrientedLine(Arguments const& args);
    Local<Value> drawCuboid(Arguments const& args);
    Local<Value> drawCircle(Arguments const& args);
};

extern ClassDefine<ParticleSpawner> ParticleSpawnerBuilder;
extern ClassDefine<void>            ParticleColorBuilder;
extern ClassDefine<void>            DirectionBuilder;
