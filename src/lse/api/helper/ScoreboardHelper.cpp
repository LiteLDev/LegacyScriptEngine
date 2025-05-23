#include "ScoreboardHelper.h"

#include "mc/world/scores/IdentityDictionary.h"

namespace lse::api {
ScoreboardId ScoreboardHelper::getId(Scoreboard const& scoreboard, PlayerScoreboardId const& playerId) {
    auto& dict  = scoreboard.mIdentityDict->mPlayers;
    auto found = dict->find(playerId);
    if (found != dict->end()) {
        return found->second;
    } else {
        return ScoreboardId::INVALID();
    }
}
} // namespace lse::api