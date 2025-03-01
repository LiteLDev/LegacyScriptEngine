#include "mc/world/scores/PlayerScoreboardId.h"
#include "mc/world/scores/Scoreboard.h"
#include "mc/world/scores/ScoreboardId.h"

namespace lse::api {
class ScoreboardHelper {
public:
    static ScoreboardId getId(Scoreboard const& scoreboard, PlayerScoreboardId const& playerId);
};
} // namespace lse::api