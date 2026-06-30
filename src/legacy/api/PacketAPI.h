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
    static Local<Object>           newPacket(std::shared_ptr<Packet> const& pkt);
    static Local<Value>            createPacket(Arguments const& args);

    std::shared_ptr<Packet> get() { return packet; }
    void                    set(std::shared_ptr<Packet> const& pkt) { packet = pkt; };

    Local<Value> getId();
    Local<Value> getName();
    Local<Value> read(Arguments const& args);
    Local<Value> write(Arguments const& args);

    Local<Value> sendTo(Arguments const& args);
    Local<Value> sendToClients(Arguments const& args);
    Local<Value> sendToServer(Arguments const& args);
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

    static Local<Object>                 newBinaryStream();
    static BinaryStreamClass*            constructor(Arguments const& args);
    static std::shared_ptr<BinaryStream> extract(Local<Value> const& v);

    Local<Value> getReadPointer(Arguments const& args);
    Local<Value> setReadPointer(Arguments const& args);
    Local<Value> getData(Arguments const& args);
    Local<Value> setData(Arguments const& args);
    Local<Value> reserve(Arguments const& args);
    Local<Value> reset();

    Local<Value> writeBool(Arguments const& args);
    Local<Value> writeByte(Arguments const& args);
    Local<Value> writeBytes(Arguments const& args);
    Local<Value> writeDouble(Arguments const& args);
    Local<Value> writeFloat(Arguments const& args);
    Local<Value> writeSignedBigEndianInt(Arguments const& args);
    Local<Value> writeSignedInt(Arguments const& args);
    Local<Value> writeSignedInt64(Arguments const& args);
    Local<Value> writeSignedShort(Arguments const& args);
    Local<Value> writeString(Arguments const& args);
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
    Local<Value> writeUuid(Arguments const& args);

    Local<Value> readBool(Arguments const& args);
    Local<Value> readByte(Arguments const& args);
    Local<Value> readBytes(Arguments const& args);
    Local<Value> readDouble(Arguments const& args);
    Local<Value> readFloat(Arguments const& args);
    Local<Value> readSignedBigEndianInt(Arguments const& args);
    Local<Value> readSignedInt(Arguments const& args);
    Local<Value> readSignedInt64(Arguments const& args);
    Local<Value> readSignedShort(Arguments const& args);
    Local<Value> readString(Arguments const& args);
    Local<Value> readUnsignedInt(Arguments const& args);
    Local<Value> readUnsignedInt64(Arguments const& args);
    Local<Value> readUnsignedShort(Arguments const& args);
    Local<Value> readUnsignedVarInt(Arguments const& args);
    Local<Value> readUnsignedVarInt64(Arguments const& args);
    Local<Value> readVarInt(Arguments const& args);
    Local<Value> readVarInt64(Arguments const& args);

    Local<Value> createPacket(Arguments const& args);
};
extern ClassDefine<BinaryStreamClass> BinaryStreamClassBuilder;
