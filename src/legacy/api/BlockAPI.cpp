#include "api/BlockAPI.h"

#include "ScriptX/ScriptX.h"
#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockEntityAPI.h"
#include "api/ContainerAPI.h"
#include "api/EntityAPI.h"
#include "api/McAPI.h"
#include "api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/string/HashedString.h"
#include "mc/deps/core/utility/optional_ref.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/ChunkBlockPos.h"
#include "mc/world/level/block/BedrockBlockNames.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/block_serialization_utils/BlockSerializationUtils.h"
#include "mc/world/level/block/components/BlockLiquidDetectionComponent.h"
#include "mc/world/level/chunk/LevelChunk.h"
#include "mc/world/level/dimension/Dimension.h"

#include <exception>

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

namespace lse::BlockAPI {
inline bool isValidHeight(WeakRef<Dimension> dim, std::variant<int, float> height) {
    if (dim) {
        if (std::holds_alternative<int>(height)) {
            int y = std::get<int>(height);
            return dim->getMinHeight() <= y && dim->getHeight() >= y;
        } else {
            float y = std::get<float>(height);
            return dim->getMinHeight() <= y && dim->getHeight() >= y;
        }
    }

    return false;
}
} // namespace lse::BlockAPI

using lse::BlockAPI::isValidHeight;

//////////////////// Classes ////////////////////

BlockClass::BlockClass(Block const& block) : ScriptClass(ScriptClass::ConstructFromCpp<BlockClass>{}), block(&block) {
    preloadData(BlockPos::ZERO(), -1);
}

BlockClass::BlockClass(Block const& block, BlockPos const& pos, DimensionType dim)
: ScriptClass(ScriptClass::ConstructFromCpp<BlockClass>{}),
  block(&block) {
    preloadData(pos, dim);
}

// generating function
Local<Object> BlockClass::newBlock(Block const& block, BlockPos const& pos, DimensionType dim) {
    auto newp = new BlockClass(block, pos, dim);
    return newp->getScriptObject();
}

Local<Object> BlockClass::newBlock(BlockPos const& pos, DimensionType dim) {
    if (auto dimension = ll::service::getLevel()->getDimension(dim)) {
        if (isValidHeight(dimension, pos.y)) {
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
    BlockPos bp = {(float)pos.x, (float)pos.y, (float)pos.z};
    if (auto dimension = ll::service::getLevel()->getDimension(pos.dim)) {
        if (isValidHeight(dimension, pos.y)) {
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

Block const* BlockClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<BlockClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<BlockClass>(v)->get();
    else return nullptr;
}

// member function
void BlockClass::preloadData(BlockPos pos, DimensionType dim) {
    name     = block->buildDescriptionName();
    type     = block->getTypeName();
    id       = block->getBlockItemId();
    blockPos = {pos.x, pos.y, pos.z, dim};
}

Local<Value> BlockClass::getName() {
    try {
        // preloaded
        return String::newString(name);
    }
    CATCH("Fail in getBlockName!");
}

Local<Value> BlockClass::getType() {
    try {
        // preloaded
        return String::newString(type);
    }
    CATCH("Fail in getBlockType!");
}

Local<Value> BlockClass::getId() {
    try {
        // preloaded
        return Number::newNumber(id);
    }
    CATCH("Fail in getBlockId!");
}

Local<Value> BlockClass::getPos() {
    try {
        // preloaded
        return IntPos::newPos(blockPos);
    }
    CATCH("Fail in getBlockPos!");
}

Local<Value> BlockClass::getTileData() {
    try {
        // preloaded
        return Number::newNumber(block->getVariant());
    }
    CATCH("Fail in getTileData!");
}

Local<Value> BlockClass::getVariant() {
    try {
        return Number::newNumber(block->getVariant());
    }
    CATCH("Fail in getVariant!");
}

Local<Value> BlockClass::getTranslucency() {
    try {
        return Number::newNumber(block->getTranslucency());
    }
    CATCH("Fail in getTranslucency!");
}

Local<Value> BlockClass::getThickness() {
    try {
        return Number::newNumber(block->getThickness());
    }
    CATCH("Fail in getThickness!");
}

Local<Value> BlockClass::isAir() {
    try {
        return Boolean::newBoolean(block->isAir());
    }
    CATCH("Fail in isAir!");
}

Local<Value> BlockClass::isBounceBlock() {
    try {
        return Boolean::newBoolean(block->isBounceBlock());
    }
    CATCH("Fail in isBounceBlock!");
}

Local<Value> BlockClass::isButtonBlock() {
    try {
        return Boolean::newBoolean(block->isButtonBlock());
    }
    CATCH("Fail in isButtonBlock!");
}

Local<Value> BlockClass::isCropBlock() {
    try {
        return Boolean::newBoolean(block->isCropBlock());
    }
    CATCH("Fail in isCropBlock!");
}

Local<Value> BlockClass::isDoorBlock() {
    try {
        return Boolean::newBoolean(block->isDoorBlock());
    }
    CATCH("Fail in isDoorBlock!");
}

Local<Value> BlockClass::isFenceBlock() {
    try {
        return Boolean::newBoolean(block->isFenceBlock());
    }
    CATCH("Fail in isFenceBlock!");
}

Local<Value> BlockClass::isFenceGateBlock() {
    try {
        return Boolean::newBoolean(block->isFenceGateBlock());
    }
    CATCH("Fail in isFenceGateBlock!");
}

Local<Value> BlockClass::isThinFenceBlock() {
    try {
        return Boolean::newBoolean(block->isThinFenceBlock());
    }
    CATCH("Fail in isThinFenceBlock!");
}

Local<Value> BlockClass::isHeavyBlock() {
    try {
        return Boolean::newBoolean(block->isFallingBlock());
    }
    CATCH("Fail in isHeavyBlock!");
}

Local<Value> BlockClass::isStemBlock() {
    try {
        return Boolean::newBoolean(block->isStemBlock());
    }
    CATCH("Fail in isStemBlock!");
}

Local<Value> BlockClass::isSlabBlock() {
    try {
        return Boolean::newBoolean(block->isSlabBlock());
    }
    CATCH("Fail in isSlabBlock!");
}

Local<Value> BlockClass::isUnbreakable() {
    try {
        return Boolean::newBoolean(block->isUnbreakable());
    }
    CATCH("Fail in isUnbreakable!");
}

Local<Value> BlockClass::isWaterBlockingBlock() {
    try {
        return Boolean::newBoolean(BlockLiquidDetectionComponent::isLiquidBlocking(*block));
    }
    CATCH("Fail in isWaterBlockingBlock!");
}

Local<Value> BlockClass::destroyBlock(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);

    try {
        // same as `Level::getBlockInstance(pos.getBlockPos(),
        // pos.dim).breakNaturally()` when drop
        BlockSource& bl = ll::service::getLevel()->getDimension(blockPos.dim)->getBlockSourceFromMainChunkSource();
        return Boolean::newBoolean(
            ll::service::getLevel()->destroyBlock(bl, blockPos.getBlockPos(), args[0].asBoolean().value())
        );
    }
    CATCH("Fail in destroyBlock!");
}

Local<Value> BlockClass::getNbt(const Arguments&) {
    try {
        return NbtCompoundClass::pack(block->getSerializationId().clone());
    }
    CATCH("Fail in getNbt!");
}

Local<Value> BlockClass::setNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) return Local<Value>(); // Null

        // update Pre Data
        auto         result = BlockSerializationUtils::tryGetBlockFromNBT(*nbt, nullptr);
        const Block* bl     = result.second;
        if (bl) {
            ll::service::getLevel()
                ->getDimension(blockPos.dim)
                ->getBlockSourceFromMainChunkSource()
                .setBlock(blockPos.getBlockPos(), *bl, 3, nullptr, nullptr);
        }
        preloadData(blockPos.getBlockPos(), blockPos.getDimensionId());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setNbt!")
}

Local<Value> BlockClass::getBlockState(const Arguments&) {
    return Local<Value>();
    try {
        auto list = block->getSerializationId();
        try {
            return Tag2Value(&list.at("states").get(), true);
        } catch (...) {
            return Array::newArray();
        }
    } catch (const std::out_of_range&) {
        return Object::newObject();
    }
    CATCH("Fail in getBlockState!")
}

Local<Value> BlockClass::hasContainer(const Arguments&) {
    try {
        auto& bl = ll::service::getLevel()
                       ->getDimension(blockPos.dim)
                       ->getBlockSourceFromMainChunkSource()
                       .getBlock(blockPos.getBlockPos());
        return Boolean::newBoolean(bl.isContainerBlock());
    }
    CATCH("Fail in hasContainer!");
}

Local<Value> BlockClass::getContainer(const Arguments&) {
    try {
        Container* container = ll::service::getLevel()
                                   ->getDimension(blockPos.dim)
                                   ->getBlockSourceFromMainChunkSource()
                                   .getBlockEntity(blockPos.getBlockPos())
                                   ->getContainer();
        return container ? ContainerClass::newContainer(container) : Local<Value>();
    }
    CATCH("Fail in getContainer!");
}

Local<Value> BlockClass::hasBlockEntity(const Arguments&) {
    try {
        return Boolean::newBoolean(block->hasBlockEntity());
    }
    CATCH("Fail in hasBlockEntity!");
}

Local<Value> BlockClass::getBlockEntity(const Arguments&) {
    try {
        BlockActor* be = ll::service::getLevel()
                             ->getDimension(blockPos.dim)
                             ->getBlockSourceFromMainChunkSource()
                             .getBlockEntity(blockPos.getBlockPos());
        return be ? BlockEntityClass::newBlockEntity(be, blockPos.dim) : Local<Value>();
    }
    CATCH("Fail in getBlockEntity!");
}

Local<Value> BlockClass::removeBlockEntity(const Arguments&) {
    try {
        ll::service::getLevel()
            ->getDimension(blockPos.dim)
            ->getBlockSourceFromMainChunkSource()
            .removeBlockEntity(blockPos.getBlockPos());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in removeBlockEntity!");
}

// public API
Local<Value> McClass::getBlock(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        IntVec4 pos;
        if (args.size() == 1) {
            // IntPos
            if (IsInstanceOf<IntPos>(args[0])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = posObj->toIntVec4();
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        auto dimPtr = ll::service::getLevel()->getDimension(pos.dim).get();
        if (!dimPtr) {
            return {};
        }
        BlockSource& bs        = dimPtr->getBlockSourceFromMainChunkSource();
        short        minHeight = dimPtr->getMinHeight();
        if (pos.y < minHeight || pos.y > dimPtr->getHeight()) {
            return {};
        }
        auto lc = bs.getChunkAt(pos.getBlockPos());
        if (!lc) {
            return {};
        }
        ChunkBlockPos cbpos = ChunkBlockPos(pos.getBlockPos(), minHeight);
        auto&         block = lc->getBlock(cbpos);
        BlockPos      bp{pos.x, pos.y, pos.z};
        return BlockClass::newBlock(block, bp, pos.dim);
    }
    CATCH("Fail in GetBlock!")
}

Local<Value> McClass::setBlock(const Arguments& args) {
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
                else {
                    pos   = *posObj;
                    block = args[1];
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos   = posObj->toIntVec4();
                    block = args[1];
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        if (block.isString()) {
            optional_ref<const Block> bl = Block::tryGetFromRegistry(block.asString().toString(), tileData);
            if (!bl.has_value()) {
                return Boolean::newBoolean(false);
            }
            BlockSource& bs = ll::service::getLevel()->getDimension(pos.dim)->getBlockSourceFromMainChunkSource();
            return Boolean::newBoolean(bs.setBlock(pos.getBlockPos(), bl, 3, nullptr, nullptr));
        } else if (IsInstanceOf<NbtCompoundClass>(block)) {
            // Nbt
            auto                      nbt = NbtCompoundClass::extract(block);
            optional_ref<const Block> bl  = Block::tryGetFromRegistry(*nbt);
            if (!bl.has_value()) {
                return Boolean::newBoolean(false);
            }
            BlockSource& bs = ll::service::getLevel()->getDimension(pos.dim)->getBlockSourceFromMainChunkSource();
            return Boolean::newBoolean(bs.setBlock(pos.getBlockPos(), bl, 3, nullptr, nullptr));
        } else {
            // other block object
            Block const* bl = BlockClass::extract(block);
            if (!bl) {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
            }
            BlockSource& bs = ll::service::getLevel()->getDimension(pos.dim)->getBlockSourceFromMainChunkSource();
            return Boolean::newBoolean(bs.setBlock(pos.getBlockPos(), *bl, 3, nullptr, nullptr));
        }
    }
    CATCH("Fail in SetBlock!")
}

Local<Value> McClass::spawnParticle(const Arguments& args) {
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
                else {
                    pos.x   = posObj->x;
                    pos.y   = posObj->y;
                    pos.z   = posObj->z;
                    pos.dim = posObj->dim;
                    type    = args[1];
                }
            } else if (IsInstanceOf<FloatPos>(args[0])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[0]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos  = *posObj;
                    type = args[1];
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        ll::service::getLevel()->spawnParticleEffect(
            type.asString().toString(),
            pos.getVec3(),
            ll::service::getLevel()->getDimension(pos.dim).get()
        );
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in SpawnParticle!")
}
