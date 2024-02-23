#pragma once

#include <ll/api/memory/Memory.h>
#include <mc/deps/core/common/bedrock/Result.h>
#include <mc/enums/MinecraftPacketIds.h>
#include <mc/network/packet/Packet.h>
#include <string>
#include <string_view>

namespace lse::api {

constexpr auto PacketName  = "NetworkPacket";
constexpr auto WriteOffset = 96;

template <int packetId, bool batching = true, bool compress = true>
class NetworkPacket final : public Packet {
public:
    NetworkPacket(std::string_view data) : mData(data) {}

    NetworkPacket()                                   = default;
    NetworkPacket(NetworkPacket&&)                    = default;
    auto operator=(NetworkPacket&&) -> NetworkPacket& = default;
    ~NetworkPacket()                                  = default;

    NetworkPacket(const NetworkPacket&)                    = delete;
    auto operator=(const NetworkPacket&) -> NetworkPacket& = delete;

    [[nodiscard]] auto getId() const -> MinecraftPacketIds override {
        return static_cast<MinecraftPacketIds>(packetId);
    }

    [[nodiscard]] auto getName() const -> std::string override { return PacketName; }

    void write(BinaryStream& stream) const override {
        auto& target = *ll::memory::dAccess<std::string*>(&stream, WriteOffset);
        target.append(mData);
    }

    auto _read(class ReadOnlyBinaryStream& /*stream*/) -> Bedrock::Result<void> override {
        return Bedrock::Result<void>{};
    }

private:
    std::string_view mData;
};

} // namespace lse::api
