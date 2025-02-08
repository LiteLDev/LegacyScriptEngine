#include "api/BlockEntityAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/MoreGlobal.h"
#include "main/Global.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/BlockSource.h"
#include "mc/world/level/block/Block.h"
#include "mc/world/level/block/actor/BlockActor.h"
#include "mc/world/level/dimension/Dimension.h"

//////////////////// Class Definition ////////////////////

ClassDefine<BlockEntityClass> BlockEntityClassBuilder = defineClass<BlockEntityClass>("LLSE_BlockEntity")
                                                            .constructor(nullptr)
                                                            .instanceProperty("name", &BlockEntityClass::getName)
                                                            .instanceProperty("pos", &BlockEntityClass::getPos)
                                                            .instanceProperty("type", &BlockEntityClass::getType)

                                                            .instanceFunction("setNbt", &BlockEntityClass::setNbt)
                                                            .instanceFunction("getNbt", &BlockEntityClass::getNbt)
                                                            .instanceFunction("getBlock", &BlockEntityClass::getBlock)
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
BlockActor* BlockEntityClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<BlockEntityClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<BlockEntityClass>(v)->get();
    else return nullptr;
}

// 成员函数
Local<Value> BlockEntityClass::getPos() {
    try {
        return IntPos::newPos(blockEntity->getPosition(), dim);
    }
    CATCH("Fail in getBlockEntityPos!")
}

Local<Value> BlockEntityClass::getName() {
    try {
        return String::newString(blockEntity->getName());
    }
    CATCH("Fail in getName!")
}

Local<Value> BlockEntityClass::getType() {
    try {
        return Number::newNumber((int)blockEntity->getType());
    }
    CATCH("Fail in getBlockEntityType!")
}

Local<Value> BlockEntityClass::getNbt(const Arguments&) {
    try {
        CompoundTag* tag = new CompoundTag();
        blockEntity->save(*tag, *SaveContextFactory::createCloneSaveContext());
        return NbtCompoundClass::pack(std::move(tag)); // Not sure is that will get right value
    }
    CATCH("Fail in getNbt!")
}

Local<Value> BlockEntityClass::setNbt(const Arguments& args) {
    using namespace lse::api;
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            return Local<Value>();
        }
        blockEntity->load(*ll::service::getLevel(), *nbt, MoreGlobal::defaultDataLoadHelper());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setNbt!")
}

Local<Value> BlockEntityClass::getBlock(const Arguments&) {
    try {
        BlockPos bp = blockEntity->getPosition();
        auto&    bl = ll::service::getLevel()->getDimension(dim)->getBlockSourceFromMainChunkSource().getBlock(bp);
        return BlockClass::newBlock(bl, bp, dim);
    }
    CATCH("Fail in getBlock!")
}
