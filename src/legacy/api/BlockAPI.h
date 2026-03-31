#pragma once
#include "legacy/api/APIHelp.h"
#include "legacy/main/Global.h"

#include <string>

//////////////////// Classes ////////////////////
class Block;
class BlockClass : public ScriptClass {
private:
    // Block is managed by BDS, so use raw pointer
    Block const* block = nullptr;

    // Pre data
    std::string   name, type;
    DimensionType id;
    IntVec4       blockPos;

public:
    explicit BlockClass(Block const& block);
    BlockClass(Block const& block, BlockPos const& pos, DimensionType dim);

    void         preloadData(BlockPos bp, DimensionType dim);
    Block const* get() const { return block; }

    static Local<Object> newBlock(Block const& block, BlockPos const& pos, DimensionType dim);
    static Local<Object> newBlock(BlockPos const& pos, DimensionType dim);
    static Local<Object> newBlock(Block const& block, BlockPos const& pos, BlockSource const& bs);
    static Local<Object> newBlock(IntVec4 pos);
    static Block const*  extract(Local<Value> const& v);

    Local<Value> getName() const;
    Local<Value> getType() const;
    Local<Value> getId() const;
    Local<Value> getPos() const;
    Local<Value> getTileData() const;
    Local<Value> getVariant() const;
    Local<Value> getTranslucency() const;
    Local<Value> getThickness() const;

    Local<Value> isAir() const;
    Local<Value> isBounceBlock() const;
    Local<Value> isButtonBlock() const;
    Local<Value> isCropBlock() const;
    Local<Value> isDoorBlock() const;
    Local<Value> isFenceBlock() const;
    Local<Value> isFenceGateBlock() const;
    Local<Value> isThinFenceBlock() const;
    Local<Value> isHeavyBlock() const;
    Local<Value> isStemBlock() const;
    Local<Value> isSlabBlock() const;
    Local<Value> isUnbreakable() const;
    Local<Value> isWaterBlockingBlock() const;

    Local<Value> getNbt(Arguments const& args) const;
    Local<Value> setNbt(Arguments const& args);
    Local<Value> getBlockState(Arguments const& args) const;
    Local<Value> hasContainer(Arguments const& args) const;
    Local<Value> getContainer(Arguments const& args) const;
    Local<Value> hasBlockEntity(Arguments const& args) const;
    Local<Value> getBlockEntity(Arguments const& args) const;
    Local<Value> removeBlockEntity(Arguments const& args) const;
    Local<Value> destroyBlock(Arguments const& args) const;
};
extern ClassDefine<BlockClass> BlockClassBuilder;
