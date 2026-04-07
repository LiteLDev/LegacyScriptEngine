#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/nbt/CompoundTag.h"
#include "mc/world/level/dimension/Dimension.h"
#include "mc/world/level/levelgen/structure/BoundingBox.h"
#include "mc/world/level/levelgen/structure/StructureTemplate.h"

Local<Value> McClass::getStructure(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    if (!IsInstanceOf<IntPos>(args[0])) {
        throw WrongArgTypeException(__FUNCTION__);
    }

    if (!IsInstanceOf<IntPos>(args[1])) {
        throw WrongArgTypeException(__FUNCTION__);
    }
    auto argsSize       = args.size();
    bool ignoreBlocks   = false;
    bool ignoreEntities = false;
    if (argsSize > 2) {
        CHECK_ARG_TYPE(args[2], ValueKind::kBoolean);
        ignoreBlocks = args[2].asBoolean().value();
    }
    if (argsSize > 3) {
        CHECK_ARG_TYPE(args[3], ValueKind::kBoolean);
        ignoreEntities = args[3].asBoolean().value();
    }
    try {
        IntPos* pos1 = IntPos::extractPos(args[0]);
        IntPos* pos2 = IntPos::extractPos(args[1]);
        if (pos1->getDimensionId() != pos2->getDimensionId()) {
            throw CreateExceptionWithInfo(__FUNCTION__, "Pos should in the same dimension!");
        }

        auto structure = StructureTemplate::create(
            "",
            ll::service::getLevel()->getDimension(pos1->getDimensionId()).lock()->getBlockSourceFromMainChunkSource(),
            BoundingBox(pos1->getBlockPos(), pos2->getBlockPos()),
            ignoreBlocks,
            ignoreEntities
        );

        return NbtCompoundClass::pack(structure->save());
    }
    CATCH_AND_THROW
}

Local<Value> McClass::setStructure(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    auto nbt = NbtCompoundClass::extract(args[0]);
    if (!nbt) {
        throw WrongArgTypeException(__FUNCTION__);
    }
    if (!IsInstanceOf<IntPos>(args[1])) {
        throw WrongArgTypeException(__FUNCTION__);
    }
    auto     argsSize = args.size();
    Mirror   mirror   = Mirror::None;
    Rotation rotation = Rotation::None;
    if (argsSize > 2) {
        CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
        auto rawMirror = args[2].asNumber().toInt32();
        if (rawMirror > 3 || rawMirror < 0) {
            return Boolean::newBoolean(false);
        }
        mirror = static_cast<Mirror>(rawMirror);
    }
    if (argsSize > 3) {
        CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
        auto rawRotation = args[3].asNumber().toInt32();
        if (rawRotation > 4 || rawRotation < 0) {
            return Boolean::newBoolean(false);
        }
        rotation = static_cast<Rotation>(rawRotation);
    }
    try {
        IntPos* pos       = IntPos::extractPos(args[1]);
        auto    structure = StructureTemplate::create("", *nbt);
        structure->placeInWorld(
            ll::service::getLevel()->getDimension(pos->getDimensionId()).lock()->getBlockSourceFromMainChunkSource(),
            pos->getBlockPos() + BlockPos(0, 1, 0),
            mirror,
            rotation
        );
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}
