// This file includes code from GMLIB, which is licensed under the GNU Lesser General Public License
// version 3.0 (LGPL-3.0). The source code for GMLIB can be obtained at https://github.com/GroupMountain/GMLIB.
// Modifications to the original code are licensed under the GNU General Public License version 3.0 (GPL-3.0).

#pragma once

#include <mc/network/packet/Packet.h>

namespace lse::api {

template <int packetId, bool batching = true, bool compress = true>
class NetworkPacket : public Packet {
public:
    NetworkPacket() { mCompressible = compress ? Compressibility::Incompressible : Compressibility::Compressible; }

    NetworkPacket(std::string_view binaryStreamData) : mData(binaryStreamData) {
        mCompressible = compress ? Compressibility::Incompressible : Compressibility::Compressible;
    }

    virtual ~NetworkPacket() {}

    virtual ::MinecraftPacketIds getId() const { return (MinecraftPacketIds)packetId; }

    virtual std::string getName() const { return "NetworkPacket"; }

    virtual void write(BinaryStream& bs) const { (*ll::memory::dAccess<std::string*>(&bs, 96)).append(mData); }

    virtual Bedrock::Result<void> _read(class ReadOnlyBinaryStream&) override { return {}; }

    virtual void dummyread() {}

    virtual bool disallowBatching() const { return !batching; }

private:
    std::string_view mData;
};

} // namespace lse::api
