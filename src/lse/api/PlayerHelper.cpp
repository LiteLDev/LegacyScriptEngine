#include "lse/api/PlayerHelper.h"

#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"

namespace lse::api {
unsigned int PlayerHelper::getXpEarnedAtCurrentLevel(Player* player) {
    int prevLevelReq = player->mPreviousLevelRequirement;
    if (player->mPlayerLevelChanged) {
        int curLvl                        = player->getAttribute(Player::LEVEL()).mCurrentValue;
        int plus                          = (curLvl / 15 == 1) ? (curLvl * 4 - 38) : (curLvl * 8 - 158);
        prevLevelReq                      = (curLvl / 15) ? (curLvl + plus) : (curLvl * 2 + 7);
        player->mPlayerLevelChanged       = false;
        player->mPreviousLevelRequirement = prevLevelReq;
    }
    auto& attribute = player->getAttribute(Player::EXPERIENCE());
    return (unsigned int)roundf(attribute.mCurrentValue * (float)prevLevelReq);
}
} // namespace lse::api