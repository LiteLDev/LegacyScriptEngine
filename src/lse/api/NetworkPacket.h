#pragma once

#include <ll/api/memory/Memory.h>
#include <mc/deps/core/common/bedrock/Result.h>
#include <mc/deps/core/utility/BinaryStream.h>
#include <mc/enums/MinecraftPacketIds.h>
#include <mc/network/packet/Packet.h>
#include <string>
#include <string_view>

namespace lse::api {

template <MinecraftPacketIds packetId>
class NetworkPacket final : public Packet {
public:
    NetworkPacket(std::string data) : mData(std::move(data)) {}

    NetworkPacket()                                   = default;
    NetworkPacket(NetworkPacket&&)                    = default;
    auto operator=(NetworkPacket&&) -> NetworkPacket& = default;
    ~NetworkPacket()                                  = default;

    NetworkPacket(const NetworkPacket&)                    = delete;
    auto operator=(const NetworkPacket&) -> NetworkPacket& = delete;

    [[nodiscard]] auto getId() const -> MinecraftPacketIds override { return packetId; }

    [[nodiscard]] auto getName() const -> std::string override { return "NetworkPacket"; }

    void write(BinaryStream& stream) const override { stream.mBuffer->append(mData); }

    auto _read(class ReadOnlyBinaryStream& /*stream*/) -> Bedrock::Result<void> override {
        return Bedrock::Result<void>{};
    }

private:
    std::string mData;
};

} // namespace lse::api
