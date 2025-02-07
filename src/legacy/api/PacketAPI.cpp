#include "api/PacketAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/BlockEntityAPI.h"
#include "api/ContainerAPI.h"
#include "api/EntityAPI.h"
#include "api/ItemAPI.h"
#include "api/McAPI.h"
#include "api/NbtAPI.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/packet/Packet.h"
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

PacketClass::PacketClass(std::shared_ptr<Packet> p) : ScriptClass(ScriptClass::ConstructFromCpp<PacketClass>{}) {
    set(p);
}

// generating function
Local<Object> PacketClass::newPacket(std::shared_ptr<class Packet> pkt) {
    auto out = new PacketClass(pkt);
    return out->getScriptObject();
}

std::shared_ptr<Packet> PacketClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<PacketClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<PacketClass>(v)->get();
    else return nullptr;
}

// member function
Local<Value> PacketClass::getName() {
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        return String::newString(pkt->getName());
    }
    CATCH("Fail in getPacketName!");
}

Local<Value> PacketClass::getId() {
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        return Number::newNumber((int)pkt->getId());
    }
    CATCH("Fail in getPacketId!");
}

//////////////////// BinaryStream Classes ////////////////////

BinaryStreamClass::BinaryStreamClass(BinaryStream* p)
: ScriptClass(ScriptClass::ConstructFromCpp<BinaryStreamClass>{}) {
    set(p);
}

// generating function
Local<Object> BinaryStreamClass::newBinaryStream() {
    auto out = new BinaryStreamClass(new BinaryStream());
    return out->getScriptObject();
}

// member function

Local<Value> BinaryStreamClass::getAndReleaseData() {
    try {
        BinaryStream* stream = get();
        if (!bs) {
            return Local<Value>();
        }
        return String::newString(stream->getAndReleaseData());
    }
    CATCH("Fail in BinaryStream getData!");
}

BinaryStreamClass* BinaryStreamClass::constructor(const Arguments& args) {
    try {
        return new BinaryStreamClass(args.thiz());
    }
    CATCH_C("Fail in Create BinaryStreamClass!");
}

Local<Value> BinaryStreamClass::reset() {
    try {
        BinaryStream* stream = get();
        if (!stream) {
            return Local<Value>();
        }
        stream->reset();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream reset!");
}

Local<Value> BinaryStreamClass::reserve(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream reserve!");
}

Local<Value> BinaryStreamClass::writeBool(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeBool(args[0].asBoolean().value());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeBool!");
}

Local<Value> BinaryStreamClass::writeByte(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeByte(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeByte!");
}

Local<Value> BinaryStreamClass::writeDouble(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeDouble(args[0].asNumber().toDouble());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeDouble!");
}

Local<Value> BinaryStreamClass::writeFloat(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeFloat(args[0].asNumber().toFloat());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeFloat!");
}

Local<Value> BinaryStreamClass::writeSignedBigEndianInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeSignedBigEndianInt(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeSignedBigEndianInt!");
}

Local<Value> BinaryStreamClass::writeSignedInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeSignedInt(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeSignedInt!");
}

Local<Value> BinaryStreamClass::writeSignedInt64(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeSignedInt64(args[0].asNumber().toInt64());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeSignedInt64!");
}

Local<Value> BinaryStreamClass::writeSignedShort(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeSignedShort(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeSignedShort!");
}

Local<Value> BinaryStreamClass::writeString(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeString(args[0].asString().toString());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeString!");
}

Local<Value> BinaryStreamClass::writeUnsignedChar(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeUnsignedChar(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeUnsignedChar!");
}

Local<Value> BinaryStreamClass::writeUnsignedInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeUnsignedInt((uint32_t)args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeUnsignedInt!");
}

Local<Value> BinaryStreamClass::writeUnsignedInt64(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeUnsignedInt64((uint64_t)args[0].asNumber().toInt64());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeUnsignedInt64!");
}

Local<Value> BinaryStreamClass::writeUnsignedShort(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeUnsignedShort((uint16_t)args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeUnsignedShort!");
}

Local<Value> BinaryStreamClass::writeUnsignedVarInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeUnsignedVarInt((uint32_t)args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeUnsignedVarInt!");
}

Local<Value> BinaryStreamClass::writeUnsignedVarInt64(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeUnsignedVarInt64((uint64_t)args[0].asNumber().toInt64());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeUnsignedVarInt64!");
}

Local<Value> BinaryStreamClass::writeVarInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeVarInt(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeVarInt!");
}

Local<Value> BinaryStreamClass::writeVarInt64(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        pkt->writeVarInt64(args[0].asNumber().toInt64());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeVarInt64!");
}

Local<Value> BinaryStreamClass::writeVec3(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        if (!IsInstanceOf<FloatPos>(args[0])) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        FloatPos* posObj = FloatPos::extractPos(args[0]);
        pkt->writeFloat(posObj->getVec3().x);
        pkt->writeFloat(posObj->getVec3().y);
        pkt->writeFloat(posObj->getVec3().z);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeVec3!");
}

Local<Value> BinaryStreamClass::writeBlockPos(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        if (!IsInstanceOf<IntPos>(args[0])) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        IntPos* posObj = IntPos::extractPos(args[0]);
        pkt->writeVarInt(posObj->getBlockPos().x);
        pkt->writeUnsignedVarInt(posObj->getBlockPos().y);
        pkt->writeVarInt(posObj->getBlockPos().z);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeVec3!");
}

Local<Value> BinaryStreamClass::writeCompoundTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        pkt->writeType(*nbt);
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeCompoundTag!");
}

Local<Value> BinaryStreamClass::writeItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        BinaryStream* pkt = get();
        if (!pkt) {
            return Local<Value>();
        }
        auto item = ItemClass::extract(args[0]);
        if (!item) {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
        pkt->writeType(NetworkItemStackDescriptor(*item));
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in BinaryStream writeCompoundTag!");
}

Local<Value> BinaryStreamClass::createPacket(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        BinaryStream* stream = get();
        if (!bs) {
            return Local<Value>();
        }
        auto pkt = MinecraftPackets::createPacket((MinecraftPacketIds)args[0].asNumber().toInt32());
        pkt->read(*stream);
        return PacketClass::newPacket(pkt);
    }
    CATCH("Fail in BinaryStream createPacket!");
}
