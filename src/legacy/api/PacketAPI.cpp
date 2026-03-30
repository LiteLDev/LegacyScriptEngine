#include "api/PacketAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/ItemAPI.h"
#include "api/NbtAPI.h"
#include "lse/api/helper/ItemStackSerializerHelpers.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/Packet.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"

//////////////////// Class Definition ////////////////////

ClassDefine<PacketClass> PacketClassBuilder = defineClass<PacketClass>("LLSE_Packet")
                                                  .constructor(nullptr)
                                                  .instanceFunction("getName", &PacketClass::getName)
                                                  .instanceFunction("getId", &PacketClass::getId)

                                                  .build();

ClassDefine<BinaryStreamClass> BinaryStreamClassBuilder =
    defineClass<BinaryStreamClass>("BinaryStream")
        .constructor(&BinaryStreamClass::constructor)
        .instanceFunction("getData", &BinaryStreamClass::getAndReleaseData)
        .instanceFunction("reset", &BinaryStreamClass::reset)
        .instanceFunction("reserve", &BinaryStreamClass::reserve)
        .instanceFunction("writeBool", &BinaryStreamClass::writeBool)
        .instanceFunction("writeByte", &BinaryStreamClass::writeByte)
        .instanceFunction("writeDouble", &BinaryStreamClass::writeDouble)
        .instanceFunction("writeFloat", &BinaryStreamClass::writeFloat)
        .instanceFunction("writeSignedBigEndianInt", &BinaryStreamClass::writeSignedBigEndianInt)
        .instanceFunction("writeSignedInt", &BinaryStreamClass::writeSignedInt)
        .instanceFunction("writeSignedInt64", &BinaryStreamClass::writeSignedInt64)
        .instanceFunction("writeSignedShort", &BinaryStreamClass::writeSignedShort)
        .instanceFunction("writeString", &BinaryStreamClass::writeString)
        .instanceFunction("writeUnsignedChar", &BinaryStreamClass::writeUnsignedChar)
        .instanceFunction("writeUnsignedInt", &BinaryStreamClass::writeUnsignedInt)
        .instanceFunction("writeUnsignedInt64", &BinaryStreamClass::writeUnsignedInt64)
        .instanceFunction("writeUnsignedShort", &BinaryStreamClass::writeUnsignedShort)
        .instanceFunction("writeUnsignedVarInt", &BinaryStreamClass::writeUnsignedVarInt)
        .instanceFunction("writeUnsignedVarInt64", &BinaryStreamClass::writeUnsignedVarInt64)
        .instanceFunction("writeVarInt", &BinaryStreamClass::writeVarInt)
        .instanceFunction("writeVarInt64", &BinaryStreamClass::writeVarInt64)
        .instanceFunction("writeVec3", &BinaryStreamClass::writeVec3)
        .instanceFunction("writeBlockPos", &BinaryStreamClass::writeBlockPos)
        .instanceFunction("writeCompoundTag", &BinaryStreamClass::writeCompoundTag)
        .instanceFunction("writeItem", &BinaryStreamClass::writeItem)
        .instanceFunction("createPacket", &BinaryStreamClass::createPacket)

        .build();

//////////////////// Packet Classes ////////////////////

PacketClass::PacketClass(std::shared_ptr<Packet> const& p) : ScriptClass(ConstructFromCpp<PacketClass>{}) { set(p); }

// generating function
Local<Object> PacketClass::newPacket(std::shared_ptr<Packet> const& pkt) {
    auto out = new PacketClass(pkt);
    return out->getScriptObject();
}

std::shared_ptr<Packet> PacketClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<PacketClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<PacketClass>(v)->get();
    return nullptr;
}

// member function
Local<Value> PacketClass::getName() {
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return {};
        }
        return String::newString(pkt->getName());
    }
    CATCH_AND_THROW
}

Local<Value> PacketClass::getId() {
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return {};
        }
        return Number::newNumber(static_cast<int>(pkt->getId()));
    }
    CATCH_AND_THROW
}

//////////////////// BinaryStream Classes ////////////////////

BinaryStreamClass::BinaryStreamClass(std::shared_ptr<BinaryStream> const& bs)
: ScriptClass(ConstructFromCpp<BinaryStreamClass>{}) {
    set(bs);
}

// generating function
Local<Object> BinaryStreamClass::newBinaryStream() {
    auto out = new BinaryStreamClass(std::make_shared<BinaryStream>());
    return out->getScriptObject();
}

// member function

Local<Value> BinaryStreamClass::getAndReleaseData() {
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        std::string data;
        stream->mBuffer.swap(data);
        return String::newString(data);
    }
    CATCH_AND_THROW
}

BinaryStreamClass* BinaryStreamClass::constructor(Arguments const& args) {
    try {
        return new BinaryStreamClass(args.thiz());
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::reset() {
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->mBuffer.clear();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::reserve(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeBool(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeBool(args[0].asBoolean().value(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeByte(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeByte(args[0].asNumber().toInt32(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeDouble(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeDouble(args[0].asNumber().toDouble(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeFloat(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeFloat(args[0].asNumber().toFloat(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeSignedBigEndianInt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeSignedBigEndianInt(args[0].asNumber().toInt32(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeSignedInt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeSignedInt(args[0].asNumber().toInt32(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeSignedInt64(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeSignedInt64(args[0].asNumber().toInt64(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeSignedShort(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeSignedShort(args[0].asNumber().toInt32(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeString(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeString(args[0].asString().toString(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeUnsignedChar(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeByte(args[0].asNumber().toInt32(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeUnsignedInt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeUnsignedInt(static_cast<uint32_t>(args[0].asNumber().toInt32()), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeUnsignedInt64(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeUnsignedInt64(static_cast<uint64_t>(args[0].asNumber().toInt64()), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeUnsignedShort(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeUnsignedShort(static_cast<uint16_t>(args[0].asNumber().toInt32()), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeUnsignedVarInt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeUnsignedVarInt(static_cast<uint32_t>(args[0].asNumber().toInt32()), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeUnsignedVarInt64(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeUnsignedVarInt64(static_cast<uint64_t>(args[0].asNumber().toInt64()), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeVarInt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeVarInt(args[0].asNumber().toInt32(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeVarInt64(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->writeVarInt64(args[0].asNumber().toInt64(), nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeVec3(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto stream = get();
        if (!stream) {
            return Boolean::newBoolean(false);
        }
        if (!IsInstanceOf<FloatPos>(args[0])) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        FloatPos* posObj = FloatPos::extractPos(args[0]);
        stream->writeFloat(posObj->getVec3().x, nullptr, nullptr);
        stream->writeFloat(posObj->getVec3().y, nullptr, nullptr);
        stream->writeFloat(posObj->getVec3().z, nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeBlockPos(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto stream = get();
        if (!stream) {
            return Boolean::newBoolean(false);
        }
        if (!IsInstanceOf<IntPos>(args[0])) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        IntPos* posObj = IntPos::extractPos(args[0]);
        stream->writeVarInt(posObj->getBlockPos().x, nullptr, nullptr);
        stream->writeUnsignedVarInt(posObj->getBlockPos().y, nullptr, nullptr);
        stream->writeVarInt(posObj->getBlockPos().z, nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeCompoundTag(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto stream = get();
        if (!stream) {
            return Boolean::newBoolean(false);
        }
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        stream->writeType(*nbt);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeItem(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto stream = get();
        if (!stream) {
            return Boolean::newBoolean(false);
        }
        auto item = ItemClass::extract(args[0]);
        if (!item) {
            throw WrongArgTypeException(__FUNCTION__);
        }
        ItemStackSerializerHelpers::write(NetworkItemStackDescriptor(*item), *stream);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::createPacket(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        auto pkt = MinecraftPackets::createPacket(static_cast<MinecraftPacketIds>(args[0].asNumber().toInt32()));
        pkt->read(*stream);
        return PacketClass::newPacket(pkt);
    }
    CATCH_AND_THROW
}
