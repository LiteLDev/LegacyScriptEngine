#pragma once

#include "ll/api/memory/Memory.h"
#include "mc/deps/core/platform/Result.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/Packet.h"

#include <string>

namespace lse::api {

template <MinecraftPacketIds packetId>
class NetworkPacket final : public Packet {
public:
    NetworkPacket(std::string data) : mData(std::move(data)) {}

    NetworkPacket()                           = default;
    NetworkPacket(NetworkPacket&&)            = default;
    NetworkPacket& operator=(NetworkPacket&&) = default;
    ~NetworkPacket() override                 = default;

    NetworkPacket(NetworkPacket const&)            = delete;
    NetworkPacket& operator=(NetworkPacket const&) = delete;

    [[nodiscard]] MinecraftPacketIds getId() const override { return packetId; }

    [[nodiscard]] std::string_view getName() const override { return "NetworkPacket"; }

    void write(BinaryStream& stream) const override { stream.mBuffer.append(mData); }

    Bedrock::Result<void> _read(class ReadOnlyBinaryStream& /*stream*/) override { return Bedrock::Result<void>{}; }

private:
    std::string mData;
};

} // namespace lse::api
