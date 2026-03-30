#include "api/BlockAPI.h"

#include "ScriptX/ScriptX.h"
#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockEntityAPI.h"
#include "api/ContainerAPI.h"
#include "api/McAPI.h"
#include "api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/helper/BlockHelper.h"
#include "mc/deps/core/utility/optional_ref.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/block/BedrockBlockNames.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/BlockChangeContext.h"
#include "mc/world/level/block/LiquidReaction.h"
#include "mc/world/level/block/VanillaBlockTags.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/block_serialization_utils/BlockSerializationUtils.h"
#include "mc/world/level/chunk/LevelChunk.h"

#include <exception>

using lse::api::BlockHelper;

//////////////////// Class Definition ////////////////////

ClassDefine<BlockClass> BlockClassBuilder =
    defineClass<BlockClass>("LLSE_Block")
        .constructor(nullptr)
        .instanceProperty("name", &BlockClass::getName)
        .instanceProperty("type", &BlockClass::getType)
        .instanceProperty("id", &BlockClass::getId)
        .instanceProperty("pos", &BlockClass::getPos)
        .instanceProperty("tileData", &BlockClass::getTileData)
        .instanceProperty("variant", &BlockClass::getVariant)
        .instanceProperty("translucency", &BlockClass::getTranslucency)
        .instanceProperty("thickness", &BlockClass::getThickness)

        .instanceProperty("isAir", &BlockClass::isAir)
        .instanceProperty("isBounceBlock", &BlockClass::isBounceBlock)
        .instanceProperty("isButtonBlock", &BlockClass::isButtonBlock)
        .instanceProperty("isCropBlock", &BlockClass::isCropBlock)
        .instanceProperty("isDoorBlock", &BlockClass::isDoorBlock)
        .instanceProperty("isFenceBlock", &BlockClass::isFenceBlock)
        .instanceProperty("isFenceGateBlock", &BlockClass::isFenceGateBlock)
        .instanceProperty("isThinFenceBlock", &BlockClass::isThinFenceBlock)
        .instanceProperty("isHeavyBlock", &BlockClass::isHeavyBlock)
        .instanceProperty("isStemBlock", &BlockClass::isStemBlock)
        .instanceProperty("isSlabBlock", &BlockClass::isSlabBlock)
        .instanceProperty("isUnbreakable", &BlockClass::isUnbreakable)
        .instanceProperty("isWaterBlockingBlock", &BlockClass::isWaterBlockingBlock)

        .instanceFunction("setNbt", &BlockClass::setNbt)
        .instanceFunction("getNbt", &BlockClass::getNbt)
        .instanceFunction("getBlockState", &BlockClass::getBlockState)
        .instanceFunction("hasContainer", &BlockClass::hasContainer)
        .instanceFunction("getContainer", &BlockClass::getContainer)
        .instanceFunction("hasBlockEntity", &BlockClass::hasBlockEntity)
        .instanceFunction("getBlockEntity", &BlockClass::getBlockEntity)
        .instanceFunction("removeBlockEntity", &BlockClass::removeBlockEntity)
        .instanceFunction("destroy", &BlockClass::destroyBlock)

        // For Compatibility
        .instanceFunction("setTag", &BlockClass::setNbt)
        .instanceFunction("getTag", &BlockClass::getNbt)
        .build();

//////////////////// Classes ////////////////////

BlockClass::BlockClass(Block const& block) : ScriptClass(ScriptClass::ConstructFromCpp<BlockClass>{}), block(&block) {
    preloadData(BlockPos::ZERO(), -1);
}

BlockClass::BlockClass(Block const& block, BlockPos const& pos, DimensionType dim)
: ScriptClass(ScriptClass::ConstructFromCpp<BlockClass>{}),
  block(&block) {
    preloadData(pos, dim);
}

// member function
void BlockClass::preloadData(BlockPos pos, DimensionType dim) {
    name     = block->buildDescriptionName();
    type     = block->getTypeName();
    id       = block->getBlockItemId();
    blockPos = {pos.x, pos.y, pos.z, dim};
}

// generating function
Local<Object> BlockClass::newBlock(Block const& block, BlockPos const& pos, DimensionType dim) {
    auto newp = new BlockClass(block, pos, dim);
    return newp->getScriptObject();
}

Local<Object> BlockClass::newBlock(BlockPos const& pos, DimensionType dim) {
    if (auto dimension = ll::service::getLevel()->getDimension(dim).lock()) {
        if (BlockHelper::isValidHeight(dimension, pos.y)) {
            auto& bl = dimension->getBlockSourceFromMainChunkSource().getBlock(pos);
            return BlockClass::newBlock(bl, pos, dim);
        }
    }
    auto block = Block::tryGetFromRegistry(BedrockBlockNames::Air());
    if (!block) {
        return Object::newObject();
    }
    return BlockClass::newBlock(block, pos, dim);
}

Local<Object> BlockClass::newBlock(Block const& block, BlockPos const& pos, BlockSource const& bs) {
    auto newp = new BlockClass(block, pos, bs.getDimensionId());
    return newp->getScriptObject();
}

Local<Object> BlockClass::newBlock(IntVec4 pos) {
    BlockPos bp = {static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z)};
    if (auto dimension = ll::service::getLevel()->getDimension(pos.dim).lock()) {
        if (BlockHelper::isValidHeight(dimension, pos.y)) {
            auto& bl = dimension->getBlockSourceFromMainChunkSource().getBlock(bp);
            return BlockClass::newBlock(bl, bp, pos.dim);
        }
    }
    auto block = Block::tryGetFromRegistry(BedrockBlockNames::Air());
    if (!block) {
        return Object::newObject();
    }
    return BlockClass::newBlock(block, bp, pos.dim);
}

Block const* BlockClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<BlockClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<BlockClass>(v)->get();
    return nullptr;
}

Local<Value> BlockClass::getName() const {
    try {
        // preloaded
        return String::newString(name);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getType() const {
    try {
        // preloaded
        return String::newString(type);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getId() const {
    try {
        // preloaded
        return Number::newNumber(id);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getPos() const {
    try {
        // preloaded
        return IntPos::newPos(blockPos);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getTileData() const {
    try {
        return Number::newNumber(block->getBlockType().getVariant(*block));
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getVariant() const {
    try {
        return Number::newNumber(block->getBlockType().getVariant(*block));
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getTranslucency() const {
    try {
        return Number::newNumber(block->getBlockType().mTranslucency);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getThickness() const {
    try {
        return Number::newNumber(block->getBlockType().mThickness);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isAir() const {
    try {
        return Boolean::newBoolean(block->isAir());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isBounceBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isBounceBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isButtonBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isButtonBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isCropBlock() const {
    try {
        return Boolean::newBoolean(block->hasTag(VanillaBlockTags::Crop()));
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isDoorBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isDoorBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isFenceBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isFenceBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isFenceGateBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isFenceGateBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isThinFenceBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isThinFenceBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isHeavyBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().mFalling);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isStemBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isStemBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isSlabBlock() const {
    try {
        return Boolean::newBoolean(block->getBlockType().isSlabBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isUnbreakable() const {
    try {
        return Boolean::newBoolean(block->mDirectData->mDestroySpeed < 0.0f);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::isWaterBlockingBlock() const {
    try {
        return Boolean::newBoolean(
            block->mDirectData->mWaterDetectionRule->mOnLiquidTouches == LiquidReaction::Blocking
        );
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getNbt(Arguments const&) const {
    try {
        return NbtCompoundClass::pack(block->mSerializationId->clone());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::setNbt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) return {}; // Null

        // update Pre Data
        auto result = BlockSerializationUtils::tryGetBlockFromNBT(*nbt, nullptr);
        if (Block const* bl = result.second) {
            ll::service::getLevel()
                ->getDimension(blockPos.dim)
                .lock()
                ->getBlockSourceFromMainChunkSource()
                .setBlock(blockPos.getBlockPos(), *bl, 3, nullptr, nullptr, BlockChangeContext(false));
        }
        preloadData(blockPos.getBlockPos(), blockPos.getDimensionId());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getBlockState(Arguments const&) const {
    try {
        auto list = block->mSerializationId;
        try {
            return Tag2Value(&list->at("states").get(), true);
        } catch (...) {
            return Array::newArray();
        }
    } catch (std::out_of_range const&) {
        return Object::newObject();
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::hasContainer(Arguments const&) const {
    try {
        auto& bl = ll::service::getLevel()
                       ->getDimension(blockPos.dim)
                       .lock()
                       ->getBlockSourceFromMainChunkSource()
                       .getBlock(blockPos.getBlockPos());
        return Boolean::newBoolean(bl.getBlockType().isContainerBlock());
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getContainer(Arguments const&) const {
    try {
        Container* container = ll::service::getLevel()
                                   ->getDimension(blockPos.dim)
                                   .lock()
                                   ->getBlockSourceFromMainChunkSource()
                                   .getBlockEntity(blockPos.getBlockPos())
                                   ->getContainer();
        return container ? ContainerClass::newContainer(container) : Local<Value>();
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::hasBlockEntity(Arguments const&) const {
    try {
        return Boolean::newBoolean(block->getBlockType().mBlockEntityType != BlockActorType::Undefined);
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::getBlockEntity(Arguments const&) const {
    try {
        BlockActor* be = ll::service::getLevel()
                             ->getDimension(blockPos.dim)
                             .lock()
                             ->getBlockSourceFromMainChunkSource()
                             .getBlockEntity(blockPos.getBlockPos());
        return be ? BlockEntityClass::newBlockEntity(be, blockPos.dim) : Local<Value>();
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::removeBlockEntity(Arguments const&) const {
    try {
        return Boolean::newBoolean(
            ll::service::getLevel()
                ->getDimension(blockPos.dim)
                .lock()
                ->getBlockSourceFromMainChunkSource()
                .removeBlockEntity(blockPos.getBlockPos())
            != nullptr
        );
    }
    CATCH_AND_THROW
}

Local<Value> BlockClass::destroyBlock(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);

    try {
        // same as `Level::getBlockInstance(pos.getBlockPos(),
        // pos.dim).breakNaturally()` when drop
        BlockSource& bl =
            ll::service::getLevel()->getDimension(blockPos.dim).lock()->getBlockSourceFromMainChunkSource();
        return Boolean::newBoolean(
            ll::service::getLevel()
                ->destroyBlock(bl, blockPos.getBlockPos(), args[0].asBoolean().value(), BlockChangeContext(false))
        );
    }
    CATCH_AND_THROW
}

// public API
Local<Value> McClass::getBlock(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        IntVec4 pos;
        if (args.size() == 1) {
            // IntPos
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = static_cast<IntVec4>(*posObj);
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return {};
                pos = posObj->toIntVec4();
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 4) {
            // Number Pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            pos = {
                args[0].asNumber().toInt32(),
                args[1].asNumber().toInt32(),
                args[2].asNumber().toInt32(),
                args[3].asNumber().toInt32()
            };
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        auto dimPtr = ll::service::getLevel()->getDimension(pos.dim).lock();
        if (!dimPtr) {
            return {};
        }
        BlockSource& bs        = dimPtr->getBlockSourceFromMainChunkSource();
        short        minHeight = dimPtr->mHeightRange->mMin;
        if (pos.y < minHeight || pos.y > dimPtr->mHeightRange->mMax) {
            return {};
        }
        auto lc = bs.getChunkAt(pos.getBlockPos());
        if (!lc) {
            return {};
        }
        auto& block = lc->getBlock(
            ChunkBlockPos{
                static_cast<uchar>(pos.x & 0xf),
                ChunkLocalHeight{static_cast<short>(pos.y - minHeight)},
                static_cast<uchar>(pos.z & 0xf)
            }
        );
        return BlockClass::newBlock(block, pos.getBlockPos(), pos.dim);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::setBlock(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);

    try {
        IntVec4        pos;
        Local<Value>   block;
        unsigned short tileData = 0;
        if (args.size() == 2 || args.size() == 3) {
            if (args.size() == 3) {
                CHECK_ARG_TYPE(args[1], ValueKind::kString);
                CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
                tileData = args[2].asNumber().toInt32();
            }
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos   = static_cast<IntVec4>(*posObj);
                block = args[1];
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos   = posObj->toIntVec4();
                block = args[1];
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 5 || args.size() == 6) {
            // Number Pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            pos = {
                args[0].asNumber().toInt32(),
                args[1].asNumber().toInt32(),
                args[2].asNumber().toInt32(),
                args[3].asNumber().toInt32()
            };
            block = args[4];
            if (args.size() == 6) {
                CHECK_ARG_TYPE(args[4], ValueKind::kString);
                CHECK_ARG_TYPE(args[5], ValueKind::kNumber);
                tileData = args[5].asNumber().toInt32();
            }
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        if (block.isString()) {
            optional_ref<Block const> bl =
                Block::tryGetFromRegistry(HashedString(block.asString().toString()), tileData);
            if (!bl.has_value()) {
                return Boolean::newBoolean(false);
            }
            BlockSource& bs =
                ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource();
            return Boolean::newBoolean(
                bs.setBlock(pos.getBlockPos(), bl, 3, nullptr, nullptr, BlockChangeContext(false))
            );
        }
        if (IsInstanceOf<NbtCompoundClass>(block)) {
            // Nbt
            auto                      nbt = NbtCompoundClass::extract(block);
            optional_ref<Block const> bl  = Block::tryGetFromRegistry(*nbt);
            if (!bl.has_value()) {
                return Boolean::newBoolean(false);
            }
            BlockSource& bs =
                ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource();
            return Boolean::newBoolean(
                bs.setBlock(pos.getBlockPos(), bl, 3, nullptr, nullptr, BlockChangeContext(false))
            );
        }
        // other block object
        Block const* bl = BlockClass::extract(block);
        if (!bl) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        BlockSource& bs = ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource();
        return Boolean::newBoolean(bs.setBlock(pos.getBlockPos(), *bl, 3, nullptr, nullptr, BlockChangeContext(false)));
    }
    CATCH_AND_THROW
}

Local<Value> McClass::spawnParticle(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2)

    try {
        FloatVec4    pos;
        Local<Value> type;

        if (args.size() == 2) {
            // IntPos
            CHECK_ARG_TYPE(args[1], ValueKind::kString);

            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos  = *posObj;
                type = args[1];
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos  = static_cast<FloatVec4>(*posObj);
                type = args[1];
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 5) {
            // Number Pos
            CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kString);

            pos = {
                args[0].asNumber().toFloat(),
                args[1].asNumber().toFloat(),
                args[2].asNumber().toFloat(),
                args[3].asNumber().toInt32()
            };
            type = args[4];
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        ll::service::getLevel()->spawnParticleEffect(
            type.asString().toString(),
            pos.getVec3(),
            ll::service::getLevel()->getDimension(pos.dim).lock().get()
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}
