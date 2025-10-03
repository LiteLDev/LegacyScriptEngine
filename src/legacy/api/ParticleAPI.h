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

    explicit ParticleSpawner(const Local<Object>& scriptObj) : ScriptClass(scriptObj) {}

    static ParticleSpawner* create(const Arguments& args);

    Local<Value> getDisplayRadius() { return Number::newNumber(static_cast<long long>(1024)); }
    Local<Value> getHighDetial() { return Boolean::newBoolean(true); }
    Local<Value> getDoubleSide() { return Boolean::newBoolean(false); }
    void         setDisplayRadius(const Local<Value>& value) {}
    void         setHighDetial(const Local<Value>& value) {}
    void         setDoubleSide(const Local<Value>& value) {}

    Local<Value> spawnParticle(const Arguments& args);

    Local<Value> drawPoint(const Arguments& args);
    Local<Value> drawNumber(const Arguments& args);
    Local<Value> drawAxialLine(const Arguments& args);
    Local<Value> drawOrientedLine(const Arguments& args);
    Local<Value> drawCuboid(const Arguments& args);
    Local<Value> drawCircle(const Arguments& args);
};

extern ClassDefine<ParticleSpawner> ParticleSpawnerBuilder;
extern ClassDefine<void>            ParticleColorBuilder;
extern ClassDefine<void>            DirectionBuilder;
