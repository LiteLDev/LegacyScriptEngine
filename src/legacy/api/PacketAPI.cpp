#include "legacy/api/PacketAPI.h"

#include "PacketAPI.h"
#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/NbtAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "lse/api/NetworkPacket.h"
#include "lse/api/helper/ItemStackSerializerHelpers.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/network/MinecraftPackets.h"
#include "mc/network/Packet.h"
#include "mc/world/item/NetworkItemStackDescriptor.h"

#include <limits>
#include <type_traits>

//////////////////// Class Definition ////////////////////

ClassDefine<PacketClass> PacketClassBuilder = defineClass<PacketClass>("LLSE_Packet")
                                                  .constructor(nullptr)
                                                  .instanceFunction("getName", &PacketClass::getName)
                                                  .instanceFunction("getId", &PacketClass::getId)
                                                  .instanceFunction("read", &PacketClass::read)
                                                  .instanceFunction("write", &PacketClass::write)
                                                  .instanceFunction("sendTo", &PacketClass::sendTo)
                                                  .instanceFunction("sendToClients", &PacketClass::sendToClients)
                                                  .instanceFunction("sendToServer", &PacketClass::sendToServer)
                                                  
                                                  .function("createPacket", &PacketClass::createPacket)

                                                  .build();

ClassDefine<BinaryStreamClass> BinaryStreamClassBuilder =
    defineClass<BinaryStreamClass>("BinaryStream")
        .constructor(&BinaryStreamClass::constructor)
        .instanceFunction("getReadPointer", &BinaryStreamClass::getReadPointer)
        .instanceFunction("setReadPointer", &BinaryStreamClass::setReadPointer)
        .instanceFunction("getData", &BinaryStreamClass::getData)
        .instanceFunction("setData", &BinaryStreamClass::setData)
        .instanceFunction("reset", &BinaryStreamClass::reset)
        .instanceFunction("reserve", &BinaryStreamClass::reserve)
        .instanceFunction("writeBool", &BinaryStreamClass::writeBool)
        .instanceFunction("writeByte", &BinaryStreamClass::writeByte)
        .instanceFunction("writeBytes", &BinaryStreamClass::writeBytes)
        .instanceFunction("writeDouble", &BinaryStreamClass::writeDouble)
        .instanceFunction("writeFloat", &BinaryStreamClass::writeFloat)
        .instanceFunction("writeSignedBigEndianInt", &BinaryStreamClass::writeSignedBigEndianInt)
        .instanceFunction("writeSignedInt", &BinaryStreamClass::writeSignedInt)
        .instanceFunction("writeSignedInt64", &BinaryStreamClass::writeSignedInt64)
        .instanceFunction("writeSignedShort", &BinaryStreamClass::writeSignedShort)
        .instanceFunction("writeString", &BinaryStreamClass::writeString)
        .instanceFunction("writeUnsignedChar", &BinaryStreamClass::writeByte)
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
        .instanceFunction("writeUuid", &BinaryStreamClass::writeUuid)
        .instanceFunction("readBool", &BinaryStreamClass::readBool)
        .instanceFunction("readByte", &BinaryStreamClass::readByte)
        .instanceFunction("readBytes", &BinaryStreamClass::readBytes)
        .instanceFunction("readUnsignedChar", &BinaryStreamClass::readByte)
        .instanceFunction("readDouble", &BinaryStreamClass::readDouble)
        .instanceFunction("readFloat", &BinaryStreamClass::readFloat)
        .instanceFunction("readSignedBigEndianInt", &BinaryStreamClass::readSignedBigEndianInt)
        .instanceFunction("readSignedInt", &BinaryStreamClass::readSignedInt)
        .instanceFunction("readSignedInt64", &BinaryStreamClass::readSignedInt64)
        .instanceFunction("readSignedShort", &BinaryStreamClass::readSignedShort)
        .instanceFunction("readString", &BinaryStreamClass::readString)
        .instanceFunction("readUnsignedInt", &BinaryStreamClass::readUnsignedInt)
        .instanceFunction("readUnsignedInt64", &BinaryStreamClass::readUnsignedInt64)
        .instanceFunction("readUnsignedShort", &BinaryStreamClass::readUnsignedShort)
        .instanceFunction("readUnsignedVarInt", &BinaryStreamClass::readUnsignedVarInt)
        .instanceFunction("readUnsignedVarInt64", &BinaryStreamClass::readUnsignedVarInt64)
        .instanceFunction("readVarInt", &BinaryStreamClass::readVarInt)
        .instanceFunction("readVarInt64", &BinaryStreamClass::readVarInt64)
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

Local<Value> PacketClass::read(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    if (!IsInstanceOf<BinaryStreamClass>(args[0])) {
        throw WrongArgTypeException(__FUNCTION__);
    }

    try {
        auto pkt = get();
        if (!pkt) {
            return Boolean::newBoolean(false);
        }

        auto stream = BinaryStreamClass::extract(args[0]);
        if (!stream) {
            return Boolean::newBoolean(false);
        }

        if (auto res = pkt->read(*stream); !res) {
            throw Exception(fmt::format("{}\nfunction: {}", res.error().code().message(), __FUNCTION__));
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PacketClass::write(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    if (!IsInstanceOf<BinaryStreamClass>(args[0])) {
        throw WrongArgTypeException(__FUNCTION__);
    }

    try {
        auto pkt = get();
        if (!pkt) {
            return Boolean::newBoolean(false);
        }

        auto stream = BinaryStreamClass::extract(args[0]);
        if (!stream) {
            return Boolean::newBoolean(false);
        }

        pkt->write(*stream);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PacketClass::createPacket(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    if (args.size() >= 2) {
        CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);
    }
    try {
        auto out = new PacketClass(
            args.size() < 2 || !args[1].asBoolean().value()
                ? MinecraftPackets::createPacket(static_cast<MinecraftPacketIds>(args[0].asNumber().toInt32()))
                : std::make_shared<lse::api::NetworkPacket>(
                      static_cast<MinecraftPacketIds>(args[0].asNumber().toInt32()),
                      ""
                  )
        );
        return out->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> PacketClass::sendTo(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return {};
        }

        if (IsInstanceOf<PlayerClass>(args[0])) {
            auto* player = PlayerClass::extract(args[0]);
            pkt->sendTo(*player);
        } else if (IsInstanceOf<EntityClass>(args[0])) {
            auto* entity = EntityClass::extract(args[0]);
            pkt->sendTo(*entity);
        } else if (IsInstanceOf<IntPos>(args[0])) {
            auto* pos = IntPos::extractPos(args[0]);
            pkt->sendTo(pos->getBlockPos(), pos->getDimensionId());
        } else if (IsInstanceOf<FloatPos>(args[0])) {
            auto* pos = FloatPos::extractPos(args[0]);
            pkt->sendTo(pos->getVec3(), pos->getDimensionId());
        } else if (args.size() >= 4 && std::ranges::all_of(std::views::iota(0ull, 4ull), [&](auto index) {
                       return args[index].getKind() == ValueKind::kNumber;
                   })) {
            pkt->sendTo(
                BlockPos{args[0].asNumber().toInt32(), args[1].asNumber().toInt32(), args[2].asNumber().toInt32()},
                args[3].asNumber().toInt32()
            );
        } else {
            throw WrongArgTypeException(__FUNCTION__);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PacketClass::sendToClients(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return {};
        }

        pkt->sendToClients();

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> PacketClass::sendToServer(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    try {
        std::shared_ptr<Packet> pkt = get();
        if (!pkt) {
            return {};
        }

        pkt->sendToServer();

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

//////////////////// BinaryStream Classes ////////////////////

template <typename T>
auto parseScalarArg(Arguments const& args, std::string_view func) {
    CHECK_ARGS_COUNT(args, 1);

    auto const& value = args[0];
    if constexpr (ll::traits::is_string_v<T>) {
        if (value.isString()) {
            return value.asString().toString();
        }
    } else {
        if (value.isBoolean()) {
            return static_cast<T>(value.asBoolean().value());
        }
        if (value.isNumber()) {
            if (auto num = value.asNumber(); num.isInteger()) {
                return static_cast<T>(value.asNumber().toInt64());
            } else {
                return static_cast<T>(value.asNumber().toDouble());
            }
        }
        if (value.isString()) {
            if constexpr (std::is_same_v<T, bool>) {
                if (auto res = ll::string_utils::svtobool(value.asString().toString()); res) {
                    return res.value();
                }
            } else if constexpr (std::is_floating_point_v<T>) {
                if (auto res =
                        ll::string_utils::svtonum<T>(value.asString().toString(), nullptr, std::chars_format::general);
                    res) {
                    return res.value();
                }
            } else {
                if (auto res = ll::string_utils::svtonum<T>(value.asString().toString(), nullptr, 10); res) {
                    return res.value();
                }
            }
        }
    }
    throw WrongArgTypeException(std::string{func});
}

template <typename T>
Local<Value> makeScalarResult(Bedrock::Result<T>&& res, char const* func, bool asString) {
    if (!res) {
        throw Exception(fmt::format("{}\nfunction: {}", res.error().code().message(), func));
    }
    auto& value = res.value();
    if (asString) return String::newString(fmt::to_string(value));

    if constexpr (std::is_same_v<T, bool>) {
        return Boolean::newBoolean(res.value());
    } else if constexpr (std::is_same_v<T, std::string>) {
        return String::newString(res.value());
    } else if constexpr (std::is_floating_point_v<T>) {
        return Number::newNumber(static_cast<double>(value));
    } else if constexpr (std::is_unsigned_v<T>) {
        if (value <= static_cast<T>(std::numeric_limits<int64_t>::max())) {
            return Number::newNumber(static_cast<int64_t>(value));
        }
        return Number::newNumber(static_cast<double>(value));
    } else {
        return Number::newNumber(static_cast<int64_t>(value));
    }
}

BinaryStreamClass::BinaryStreamClass(std::shared_ptr<BinaryStream> const& bs)
: ScriptClass(ConstructFromCpp<BinaryStreamClass>{}) {
    set(bs);
}

// generating function
Local<Object> BinaryStreamClass::newBinaryStream() {
    auto out = new BinaryStreamClass(std::make_shared<BinaryStream>());
    return out->getScriptObject();
}

std::shared_ptr<BinaryStream> BinaryStreamClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<BinaryStreamClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<BinaryStreamClass>(v)->get();
    return nullptr;
}

// member function

Local<Value> BinaryStreamClass::getData(Arguments const& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);
    try {

        auto stream = get();
        if (!stream) {
            return {};
        }

        auto result = ByteBuffer::newByteBuffer(stream->mBuffer.data(), stream->mBuffer.size());
        if (args.size() >= 1 && args[0].asBoolean().value()) {
            stream->mBuffer.clear();
            stream->mView        = stream->mBuffer;
            stream->mReadPointer = 0;
        }
        return result;
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::setData(Arguments const& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kByteBuffer);
    try {

        auto stream = get();
        if (!stream) {
            return {};
        }

        auto buffer          = args[0].asByteBuffer();
        stream->mBuffer      = std::string_view{reinterpret_cast<char*>(buffer.getRawBytes()), buffer.byteLength()};
        stream->mView        = stream->mBuffer;
        stream->mReadPointer = 0;

        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

BinaryStreamClass* BinaryStreamClass::constructor(Arguments const& args) {
    try {
        return new BinaryStreamClass(args.thiz());
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::getReadPointer(Arguments const& args) {
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        return Number::newNumber(static_cast<int64>(stream->mReadPointer));
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::setReadPointer(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->mReadPointer = args[0].asNumber().toInt64();
        return Boolean::newBoolean(true);
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
        stream->mView        = stream->mBuffer;
        stream->mReadPointer = 0;
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::reserve(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        stream->mBuffer.reserve(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::writeBytes(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kByteBuffer);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        auto buffer = args[0].asByteBuffer();
        stream->mBuffer.append(reinterpret_cast<char*>(buffer.getRawBytes()), buffer.byteLength());
        stream->mView = stream->mBuffer;
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::readBytes(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    auto length = args[0].asNumber().toInt32();
    if (length <= 0) throw WrongArgTypeException(__FUNCTION__);
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }

        std::string buffer(length, '\0');
        if (auto res = stream->read(buffer.data(), length); !res) {
            throw Exception(fmt::format("{}\nfunction: {}", res.error().code().message(), __func__));
        }
        return ByteBuffer::newByteBuffer(buffer.data(), length);
    }
    CATCH_AND_THROW
}

#define SCALAR_STREAM_MACRO(NAME, ...)                                                                                 \
    Local<Value> BinaryStreamClass::write##NAME(Arguments const& args) {                                               \
        try {                                                                                                          \
            auto stream = get();                                                                                       \
            if (!stream) {                                                                                             \
                return {};                                                                                             \
            }                                                                                                          \
            auto value = parseScalarArg<ll::traits::function_traits<decltype(&BinaryStream::write##NAME)>::arg<0>>(    \
                args,                                                                                                  \
                __FUNCTION__                                                                                           \
            );                                                                                                         \
            stream->write##NAME(value, nullptr, nullptr);                                                              \
            return Boolean::newBoolean(true);                                                                          \
        }                                                                                                              \
        CATCH_AND_THROW                                                                                                \
    }                                                                                                                  \
    Local<Value> BinaryStreamClass::read##NAME(Arguments const& args) {                                                \
        if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kBoolean);                                            \
        try {                                                                                                          \
            auto stream = get();                                                                                       \
            if (!stream) {                                                                                             \
                return {};                                                                                             \
            }                                                                                                          \
            return makeScalarResult(                                                                                   \
                stream->get##NAME(__VA_ARGS__),                                                                        \
                __FUNCTION__,                                                                                          \
                args.size() >= 1 ? args[0].asBoolean().value() : false                                                 \
            );                                                                                                         \
        }                                                                                                              \
        CATCH_AND_THROW                                                                                                \
    }

SCALAR_STREAM_MACRO(Bool);
SCALAR_STREAM_MACRO(Byte);
SCALAR_STREAM_MACRO(Double);
SCALAR_STREAM_MACRO(Float);
SCALAR_STREAM_MACRO(SignedBigEndianInt);
SCALAR_STREAM_MACRO(SignedInt);
SCALAR_STREAM_MACRO(SignedInt64);
SCALAR_STREAM_MACRO(SignedShort);
SCALAR_STREAM_MACRO(String, std::numeric_limits<uint64_t>::max());
SCALAR_STREAM_MACRO(UnsignedInt);
SCALAR_STREAM_MACRO(UnsignedInt64);
SCALAR_STREAM_MACRO(UnsignedShort);
SCALAR_STREAM_MACRO(UnsignedVarInt);
SCALAR_STREAM_MACRO(UnsignedVarInt64);
SCALAR_STREAM_MACRO(VarInt);
SCALAR_STREAM_MACRO(VarInt64);

#undef SCALAR_STREAM_MACRO

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
        stream->writeVarInt(posObj->getBlockPos().y, nullptr, nullptr);
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

Local<Value> BinaryStreamClass::writeUuid(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    auto uuidStr = args[0].asString().toString();
    if (!mce::UUID::canParse(uuidStr)) {
        throw Exception(fmt ::format("Invalid UUID: {}", uuidStr));
    }

    try {
        auto stream = get();
        if (!stream) {
            return {};
        }

        auto uuid = mce::UUID::fromString(uuidStr);
        stream->writeUnsignedInt64(uuid.a, nullptr, nullptr);
        stream->writeUnsignedInt64(uuid.b, nullptr, nullptr);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> BinaryStreamClass::createPacket(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    if (args.size() >= 2) {
        CHECK_ARG_TYPE(args[1], ValueKind::kBoolean);
    }
    try {
        auto stream = get();
        if (!stream) {
            return {};
        }
        std::shared_ptr<Packet> pkt =
            args.size() < 2 || !args[1].asBoolean().value()
                ? MinecraftPackets::createPacket(static_cast<MinecraftPacketIds>(args[0].asNumber().toInt32()))
                : std::make_shared<lse::api::NetworkPacket>(
                      static_cast<MinecraftPacketIds>(args[0].asNumber().toInt32()),
                      ""
                  );

        if (auto res = pkt->read(*stream); !res) {
            throw Exception(fmt::format("{}\nfunction: {}", res.error().code().message(), __func__));
        }
        return PacketClass::newPacket(pkt);
    }
    CATCH_AND_THROW
}
