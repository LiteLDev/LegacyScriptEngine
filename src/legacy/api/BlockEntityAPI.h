#pragma once
#include "legacy/api/APIHelp.h"

//////////////////// Classes ////////////////////
class BlockActor;
class BlockEntityClass : public ScriptClass {
private:
    // BlockActor is managed by BDS, so use raw pointer
    BlockActor* blockEntity = nullptr;
    int         dim;

public:
    explicit BlockEntityClass(BlockActor* be, int dim);

    BlockActor* get() const { return blockEntity; }

    static Local<Object> newBlockEntity(BlockActor* be, int dim);
    static BlockActor*   extract(Local<Value> const& v);

    Local<Value> getName() const;
    Local<Value> getPos() const;
    Local<Value> getType() const;

    Local<Value> getNbt(Arguments const& args) const;
    Local<Value> setNbt(Arguments const& args) const;
    Local<Value> getBlock(Arguments const& args) const;
};
extern ClassDefine<BlockEntityClass> BlockEntityClassBuilder;
