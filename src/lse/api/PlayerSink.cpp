#include "lse/api/PlayerSink.h"

#include "ll/api/io/FileUtils.h"
#include "ll/api/io/PatternFormatter.h"
#include "ll/api/service/Bedrock.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"

namespace lse::io {

PlayerSink::PlayerSink(mce::UUID const& uuid)
: Sink(ll::makePolymorphic<ll::io::PatternFormatter>("<{tit}|{lvl}> [{tm:%T}] {msg}", false)),
  playerUuid(uuid) {}

PlayerSink::~PlayerSink() = default;

void PlayerSink::setFormatter(ll::Polymorphic<ll::io::Formatter> fmter) {
    std::lock_guard lock(mutex);
    formatter = std::move(fmter);
}
void PlayerSink::append(ll::io::LogMessageView const& view) {
    std::lock_guard lock(mutex);
    std::string     buffer;
    formatter->format(view, buffer);
    if (auto* player = ll::service::getLevel()->getPlayer(playerUuid)) {
        player->sendMessage(buffer);
    }
}

void PlayerSink::setUUID(mce::UUID const& uuid) { playerUuid = uuid; }
} // namespace lse::io