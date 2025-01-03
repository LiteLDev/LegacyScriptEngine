#pragma once
#include "api/APIHelp.h"
#include "main/Global.h"

#include <string>

//////////////////// Classes ////////////////////
class Block;
class BlockClass : public ScriptClass {
private:
    Block const* block;

    // Pre data
    std::string   name, type;
    DimensionType id;
    IntVec4       blockPos;

public:
    explicit BlockClass(Block const& block);
    BlockClass(Block const& block, BlockPos const& pos, DimensionType dim);

    void         preloadData(BlockPos bp, DimensionType dim);
    Block const* get() { return block; }

    static Local<Object> newBlock(Block const& block, BlockPos const& pos, DimensionType dim);
    static Local<Object> newBlock(BlockPos const& pos, DimensionType dim);
    static Local<Object> newBlock(Block const& block, BlockPos const& pos, BlockSource const& bs);
    static Local<Object> newBlock(IntVec4 pos);
    static Block const*  extract(Local<Value> v);

    Local<Value> getName();
    Local<Value> getType();
    Local<Value> getId();
    Local<Value> getPos();
    Local<Value> getTileData();
    Local<Value> getVariant();
    Local<Value> getTranslucency();
    Local<Value> getThickness();

    Local<Value> isAir();
    Local<Value> isBounceBlock();
    Local<Value> isButtonBlock();
    Local<Value> isCropBlock();
    Local<Value> isDoorBlock();
    Local<Value> isFenceBlock();
    Local<Value> isFenceGateBlock();
    Local<Value> isThinFenceBlock();
    Local<Value> isHeavyBlock();
    Local<Value> isStemBlock();
    Local<Value> isSlabBlock();
    Local<Value> isUnbreakable();
    Local<Value> isWaterBlockingBlock();

    Local<Value> getNbt(const Arguments& args);
    Local<Value> setNbt(const Arguments& args);
    Local<Value> getBlockState(const Arguments& args);
    Local<Value> hasContainer(const Arguments& args);
    Local<Value> getContainer(const Arguments& args);
    Local<Value> hasBlockEntity(const Arguments& args);
    Local<Value> getBlockEntity(const Arguments& args);
    Local<Value> removeBlockEntity(const Arguments& args);
    Local<Value> destroyBlock(const Arguments& args);
};
extern ClassDefine<BlockClass> BlockClassBuilder;
