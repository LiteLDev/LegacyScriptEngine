#include "PlayerHelper.h"

#include "AttributeHelper.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeModificationContext.h"
#include "mc/world/attribute/MutableAttributeWithContext.h"

namespace lse::api {

unsigned int PlayerHelper::getPreviousLevelRequirement(Player* player) {
    int prevLevelReq = player->mPreviousLevelRequirement;
    if (player->mPlayerLevelChanged) {
        int curLvl                        = player->getAttribute(Player::LEVEL()).mCurrentValue;
        int plus                          = (curLvl / 15 == 1) ? (curLvl * 4 - 38) : (curLvl * 8 - 158);
        prevLevelReq                      = (curLvl / 15) ? (curLvl + plus) : (curLvl * 2 + 7);
        player->mPlayerLevelChanged       = false;
        player->mPreviousLevelRequirement = prevLevelReq;
    }
    return prevLevelReq;
}

unsigned int PlayerHelper::getXpEarnedAtCurrentLevel(Player* player) {
    unsigned int prevLevelReq = PlayerHelper::getPreviousLevelRequirement(player);
    auto&        attribute    = player->getAttribute(Player::EXPERIENCE());
    return (unsigned int)roundf(attribute.mCurrentValue * (float)prevLevelReq);
}

bool PlayerHelper::setXpEarnedAtCurrentLevel(Player* player, unsigned int xp) {
    unsigned int prevLevelReq = PlayerHelper::getPreviousLevelRequirement(player);
    auto         attribute    = player->getMutableAttribute(Player::EXPERIENCE());
    AttributeHelper::setCurrentValue(attribute, (float)xp / (float)prevLevelReq);
    return true;
}

} // namespace lse::api