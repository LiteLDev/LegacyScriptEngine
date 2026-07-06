#pragma once

#include "ll/api/memory/Memory.h"
#include "mc/deps/core/platform/Result.h"
#include "mc/deps/core/utility/BinaryStream.h"
#include "mc/network/MinecraftPacketIds.h"
#include "mc/network/Packet.h"

#include <string>

namespace lse::api {

class NetworkPacket final : public Packet {
public:
    NetworkPacket(MinecraftPacketIds id, std::string data) : Packet(), mPacketId(id), mData(std::move(data)) {}

    NetworkPacket()                           = default;
    NetworkPacket(NetworkPacket&&)            = default;
    NetworkPacket& operator=(NetworkPacket&&) = default;
    ~NetworkPacket() override                 = default;

    NetworkPacket(NetworkPacket const&)            = delete;
    NetworkPacket& operator=(NetworkPacket const&) = delete;

    [[nodiscard]] MinecraftPacketIds getId() const override { return mPacketId; }

    [[nodiscard]] std::string_view getName() const override { return "NetworkPacket"; }

    void write(BinaryStream& stream) const override { stream.mBuffer.append(mData); }

    Bedrock::Result<void> _read(ReadOnlyBinaryStream& stream) override {
        mData = stream.mView.substr(stream.mReadPointer);
        return {};
    }

private:
    MinecraftPacketIds mPacketId;
    std::string        mData;
};

} // namespace lse::api
