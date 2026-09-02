#include "legacy/api/BlockEntityAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/MoreGlobal.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/deps/nbt/CompoundTag.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/block/actor/BlockActorType.h"
#include "mc/world/level/block/actor/VanillaBlockActor.h"
#include "mc/world/level/dimension/Dimension.h"

//////////////////// Class Definition ////////////////////

ClassDefine<BlockEntityClass> BlockEntityClassBuilder =
    defineClass<BlockEntityClass>("LLSE_BlockEntity")
        .constructor(nullptr)
        .instanceProperty("name", &BlockEntityClass::getName)
        .instanceProperty("pos", &BlockEntityClass::getPos)
        .instanceProperty("type", &BlockEntityClass::getType)

        .instanceFunction("toString", &BlockEntityClass::toString)
        .instanceFunction("setNbt", &BlockEntityClass::setNbt)
        .instanceFunction("getNbt", &BlockEntityClass::getNbt)
        .instanceFunction("getBlock", &BlockEntityClass::getBlock)
        .instanceFunction("setCustomName", &BlockEntityClass::setCustomName)
        .instanceFunction("getCustomName", &BlockEntityClass::getCustomName)
        .build();

//////////////////// Classes ////////////////////

BlockEntityClass::BlockEntityClass(BlockActor* be, int dim)
: ScriptClass(ScriptClass::ConstructFromCpp<BlockEntityClass>{}),
  blockEntity(be),
  dim(dim) {}

Local<Object> BlockEntityClass::newBlockEntity(BlockActor* be, int dim) {
    auto newp = new BlockEntityClass(be, dim);
    return newp->getScriptObject();
}

// 生成函数
BlockActor* BlockEntityClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<BlockEntityClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<BlockEntityClass>(v)->get();
    return nullptr;
}

// 成员函数
Local<Value> BlockEntityClass::getPos() const {
    try {
        return IntPos::newPos(blockEntity->mPosition, dim);
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::getName() const {
    try {
        if (blockEntity->mType != BlockActorType::DataDriven) {
            auto blockActor = static_cast<VanillaBlockActor*>(blockEntity);
            return String::newString(blockActor->getName());
        }
        return String::newString("");
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::getType() const {
    try {
        return Number::newNumber(static_cast<int>(blockEntity->mType));
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::toString() const {
    try {
        return String::newString(
            fmt::format(
                "{},{},{}({})",
                getName().asString().toString(),
                blockEntity->mType,
                dim,
                blockEntity->mPosition->toString()
            )
        );
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::getNbt(Arguments const&) const {
    try {
        auto tag = std::make_unique<CompoundTag>();
        blockEntity->save(*tag, *SaveContextFactory::createCloneSaveContext());
        return NbtCompoundClass::pack(std::move(tag)); // Not sure is that will get right value
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::setNbt(Arguments const& args) const {
    using namespace lse::api;
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            return {};
        }
        blockEntity->load(*ll::service::getLevel(), *nbt, MoreGlobal::defaultDataLoadHelper());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::getBlock(Arguments const&) const {
    try {
        BlockPos blockPos = blockEntity->mPosition;
        auto&    block =
            ll::service::getLevel()->getDimension(dim).lock()->getBlockSourceFromMainChunkSource().getBlock(blockPos);
        return BlockClass::newBlock(block, blockPos, dim);
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::setCustomName(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (blockEntity->mType != BlockActorType::DataDriven) {
            auto blockActor         = static_cast<VanillaBlockActor*>(blockEntity);
            blockActor->mCustomName = args[0].asString().toString();
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> BlockEntityClass::getCustomName() const {
    try {
        if (blockEntity->mType != BlockActorType::DataDriven) {
            auto blockActor = static_cast<VanillaBlockActor*>(blockEntity);
            return String::newString(blockActor->mCustomName->mUnredactedString);
        }
        return String::newString("");
    }
    CATCH_AND_THROW
}
