// From https://github.com/GroupMountain/GMLIB
#pragma once
#include "mc/network/packet/Packet.h"

template <int packetId, bool batching = true, bool compress = true>
class NetworkPacket : public Packet {
public:
    std::string_view mData;

public:
    NetworkPacket() { mCompressible = compress ? Compressibility::Incompressible : Compressibility::Compressible; }

    NetworkPacket(std::string_view binaryStreamData) : mData(binaryStreamData) {
        mCompressible = compress ? Compressibility::Incompressible : Compressibility::Compressible;
    }

public:
    virtual ~NetworkPacket() {}

    virtual ::MinecraftPacketIds getId() const { return (MinecraftPacketIds)packetId; }

    virtual std::string getName() const { return "NetworkPacket"; }

    virtual void write(BinaryStream& bs) const { (*ll::memory::dAccess<std::string*>(&bs, 96)).append(mData); }

    virtual Bedrock::Result<void> _read(class ReadOnlyBinaryStream&) override { return {}; }

    virtual void dummyread() {}

    virtual bool disallowBatching() const { return !batching; }
};
