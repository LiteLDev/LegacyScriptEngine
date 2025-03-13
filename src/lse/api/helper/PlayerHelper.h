class Player;

namespace lse::api {
class PlayerHelper {
public:
    static unsigned int getPreviousLevelRequirement(Player* player);

    static unsigned int getXpEarnedAtCurrentLevel(Player* player);

    static bool setXpEarnedAtCurrentLevel(Player* player, unsigned int xp);
};
} // namespace lse::api
