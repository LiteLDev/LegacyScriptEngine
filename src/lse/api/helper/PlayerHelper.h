#pragma once

class Player;

namespace lse::api {
class PlayerHelper {
public:
    static unsigned int getPreviousLevelRequirement(Player* player);

    static unsigned int getXpEarnedAtCurrentLevel(Player* player);

    static bool setXpEarnedAtCurrentLevel(Player* player, unsigned int xp);

    static long long getXpNeededForLevelRange(int startLevel, int endLevel);
};
} // namespace lse::api
