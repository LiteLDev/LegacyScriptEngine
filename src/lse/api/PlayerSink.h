#pragma once

#include "ll/api/io/Sink.h"
#include "mc/platform/UUID.h"

#include <mutex>

namespace lse::io {
class PlayerSink : public ll::io::Sink {
    std::mutex mutex;
    mce::UUID  playerUuid;

public:
    PlayerSink(mce::UUID const& uuid);

    ~PlayerSink() override;

    void setFormatter(ll::Polymorphic<ll::io::Formatter> fmter) override;

    void append(ll::io::LogMessageView const& view) override;

    void setUUID(mce::UUID const& uuid);
};
} // namespace lse::io