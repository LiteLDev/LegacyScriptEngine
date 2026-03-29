#pragma once

#include "api/APIHelp.h" // IWYU pragma: keep
#include "main/Global.h"

class BlockSource;

///////////////////// Enum //////////////////////
extern ClassDefine<void> DamageCauseEnumBuilder;
extern ClassDefine<void> ActorDamageCauseEnumBuilder;

//////////////////// Classes ////////////////////
class IntPos : public IntVec4, public ScriptClass {
public:
    explicit IntPos(Local<Object> const& scriptObj) : IntVec4(), ScriptClass(scriptObj) {}
    static IntPos* create(Arguments const& args);

    static Local<Object> newPos(int x, int y, int z, int dim = -1);
    static Local<Object> newPos(BlockPos const& b, int dim = -1);
    static Local<Object> newPos(BlockPos const* b, int dim = -1);
    static Local<Object> newPos(BlockPos const* b, BlockSource const* bs);
    static Local<Object> newPos(IntVec4 const& v);
    static IntPos*       extractPos(Local<Value> const& v);

    Local<Value> getX() const { return Number::newNumber(x); }
    Local<Value> getY() const { return Number::newNumber(y); }
    Local<Value> getZ() const { return Number::newNumber(z); }
    Local<Value> getDim() const;
    Local<Value> getDimId() const { return Number::newNumber(dim); }
    void         setX(Local<Value> const& value) { x = value.asNumber().toInt32(); }
    void         setY(Local<Value> const& value) { y = value.asNumber().toInt32(); }
    void         setZ(Local<Value> const& value) { z = value.asNumber().toInt32(); }
    void         setDimId(Local<Value> const& value) { dim = value.asNumber().toInt32(); }
    Local<Value> toString();
};
extern ClassDefine<IntPos> IntPosBuilder;

class FloatPos : public FloatVec4, public ScriptClass {
public:
    explicit FloatPos(Local<Object> const& scriptObj) : FloatVec4(), ScriptClass(scriptObj) {}
    static FloatPos* create(Arguments const& args);

    static Local<Object> newPos(double x, double y, double z, int dim = -1);
    static Local<Object> newPos(Vec3 const& v, int dim = -1);
    static Local<Object> newPos(FloatVec4 const& v);
    static FloatPos*     extractPos(Local<Value> const& v);

    Local<Value> getX() const { return Number::newNumber(x); }
    Local<Value> getY() const { return Number::newNumber(y); }
    Local<Value> getZ() const { return Number::newNumber(z); }
    Local<Value> getDim() const;
    Local<Value> getDimId() const { return Number::newNumber(dim); }
    Local<Value> toString();

    void setX(Local<Value> const& value) { x = value.asNumber().toFloat(); }
    void setY(Local<Value> const& value) { y = value.asNumber().toFloat(); }
    void setZ(Local<Value> const& value) { z = value.asNumber().toFloat(); }
    void setDimId(Local<Value> const& value) { dim = value.asNumber().toInt32(); }
};
extern ClassDefine<FloatPos> FloatPosBuilder;

class DirectionAngle : public ScriptClass {
public:
    float pitch = 0, yaw = 0;

    explicit DirectionAngle(Local<Object> const& scriptObj) : ScriptClass(scriptObj) {}
    static DirectionAngle* create(Arguments const& args);
    static DirectionAngle* extract(Local<Value> const& value);

    static Local<Object>       newAngle(float pitch, float yaw);
    [[nodiscard]] Local<Value> getPitch() const { return Number::newNumber(pitch); }
    [[nodiscard]] Local<Value> getYaw() const { return Number::newNumber(yaw); }
    void                       setPitch(Local<Value> const& value) { pitch = value.asNumber().toFloat(); }
    void                       setYaw(Local<Value> const& value) { yaw = value.asNumber().toFloat(); }
    Local<Value>               toString();

    Local<Value> toFacing() const;
};
extern ClassDefine<DirectionAngle> DirectionAngleBuilder;
