#pragma once

class Player;

namespace lse::api {
class PlayerHelper {
public:
    static int getPreviousLevelRequirement(Player* player);

    static int getXpEarnedAtCurrentLevel(Player* player);

    static bool setXpEarnedAtCurrentLevel(Player* player, int xp);

    static long long getXpNeededForLevelRange(int startLevel, int endLevel);
};
} // namespace lse::api
