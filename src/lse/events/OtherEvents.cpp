#include "legacy/api/EventAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "ll/api/memory/Hook.h"
#include "ll/api/service/Bedrock.h"
#include "mc/common/ActorUniqueID.h"
#include "mc/world/scores/IdentityDefinition.h"
#include "mc/world/scores/Objective.h"
#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/ScoreInfo.h"
#include "mc/world/scores/ScoreboardId.h"
#include "mc/world/scores/ServerScoreboard.h"

namespace lse::events::other {
LL_TYPE_INSTANCE_HOOK(
    ScoreChangedHook,
    HookPriority::Normal,
    ServerScoreboard,
    &ServerScoreboard::$onScoreChanged,
    void,
    ScoreboardId const& id,
    Objective const&    obj
) {
    IF_LISTENED(EVENT_TYPES::onScoreChanged) {
        if (id.getIdentityDef().isPlayerType()) {
            if (!CallEvent(
                    EVENT_TYPES::onScoreChanged,
                    PlayerClass::newPlayer(
                        ll::service::getLevel()->getPlayer(
                            ActorUniqueID(id.getIdentityDef().getPlayerId().mActorUniqueId)
                        )
                    ),
                    Number::newNumber(obj.getPlayerScore(id).mValue),
                    String::newString(obj.getName()),
                    String::newString(obj.getDisplayName())
                )) {
                return;
            }
        }
    }
    IF_LISTENED_END(EVENT_TYPES::onScoreChanged);
    origin(id, obj);
}

void ScoreChangedEvent() { ScoreChangedHook::hook(); }
} // namespace lse::events::other