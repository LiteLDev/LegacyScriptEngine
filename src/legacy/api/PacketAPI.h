#pragma once
#include "legacy/api/APIHelp.h"
#include "mc/deps/core/utility/BinaryStream.h"

//////////////////// Classes ////////////////////
class Packet;
class BinaryStream;

class PacketClass : public ScriptClass {
private:
    std::shared_ptr<Packet> packet = nullptr;

public:
    explicit PacketClass(std::shared_ptr<Packet> const& p);
    static std::shared_ptr<Packet> extract(Local<Value> const& v);

    std::shared_ptr<Packet> get() { return packet; }

    void set(std::shared_ptr<Packet> const& pkt) { packet = pkt; };

    static Local<Object> newPacket(std::shared_ptr<Packet> const& pkt);

    Local<Value> getId();
    Local<Value> getName();
};
extern ClassDefine<PacketClass> PacketClassBuilder;

class BinaryStreamClass : public ScriptClass {
private:
    std::shared_ptr<BinaryStream> binaryStream = nullptr;

public:
    explicit BinaryStreamClass(std::shared_ptr<BinaryStream> const& bs);

    BinaryStreamClass(Local<Object> const& scriptObj)
    : ScriptClass(scriptObj),
      binaryStream(std::make_shared<BinaryStream>()) {}

    std::shared_ptr<BinaryStream> get() { return binaryStream; }
    void                          set(std::shared_ptr<BinaryStream> const& bs) { binaryStream = bs; };

    static Local<Object>      newBinaryStream();
    static BinaryStreamClass* constructor(Arguments const& args);

    Local<Value> getAndReleaseData();
    Local<Value> reset();

    Local<Value> reserve(Arguments const& args);
    Local<Value> writeBool(Arguments const& args);
    Local<Value> writeByte(Arguments const& args);
    Local<Value> writeDouble(Arguments const& args);
    Local<Value> writeFloat(Arguments const& args);
    Local<Value> writeSignedBigEndianInt(Arguments const& args);
    Local<Value> writeSignedInt(Arguments const& args);
    Local<Value> writeSignedInt64(Arguments const& args);
    Local<Value> writeSignedShort(Arguments const& args);
    Local<Value> writeString(Arguments const& args);
    Local<Value> writeUnsignedChar(Arguments const& args);
    Local<Value> writeUnsignedInt(Arguments const& args);
    Local<Value> writeUnsignedInt64(Arguments const& args);
    Local<Value> writeUnsignedShort(Arguments const& args);
    Local<Value> writeUnsignedVarInt(Arguments const& args);
    Local<Value> writeUnsignedVarInt64(Arguments const& args);
    Local<Value> writeVarInt(Arguments const& args);
    Local<Value> writeVarInt64(Arguments const& args);
    Local<Value> writeVec3(Arguments const& args);
    Local<Value> writeBlockPos(Arguments const& args);
    Local<Value> writeCompoundTag(Arguments const& args);
    Local<Value> writeItem(Arguments const& args);

    Local<Value> createPacket(Arguments const& args);
};
extern ClassDefine<BinaryStreamClass> BinaryStreamClassBuilder;
