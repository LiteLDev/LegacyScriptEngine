#include "PlayerHelper.h"

#include "AttributeHelper.h"
#include "mc/entity/components/AttributesComponent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeInstanceConstRef.h"
#include "mc/world/attribute/AttributeInstanceHandle.h" // IWYU pragma: keep

namespace lse::api {

unsigned int PlayerHelper::getPreviousLevelRequirement(Player* player) {
    int prevLevelReq = player->mPreviousLevelRequirement;
    if (player->mPlayerLevelChanged) {
        int curLvl                        = player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue;
        int plus                          = (curLvl / 15 == 1) ? (curLvl * 4 - 38) : (curLvl * 8 - 158);
        prevLevelReq                      = (curLvl / 15) ? (curLvl + plus) : (curLvl * 2 + 7);
        player->mPlayerLevelChanged       = false;
        player->mPreviousLevelRequirement = prevLevelReq;
    }
    return prevLevelReq;
}

unsigned int PlayerHelper::getXpEarnedAtCurrentLevel(Player* player) {
    unsigned int prevLevelReq = getPreviousLevelRequirement(player);
    auto         attribute    = player->getAttribute(Player::EXPERIENCE());
    return static_cast<unsigned int>(roundf(attribute.mPtr->mCurrentValue * static_cast<float>(prevLevelReq)));
}

bool PlayerHelper::setXpEarnedAtCurrentLevel(Player* player, unsigned int xp) {
    if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
        unsigned int prevLevelReq = getPreviousLevelRequirement(player);
        AttributeHelper::setCurrentValue(
            component->mAttributes,
            Player::EXPERIENCE(),
            static_cast<float>(xp) / static_cast<float>(prevLevelReq)
        );
        return true;
    }
    return false;
}

long long PlayerHelper::getXpNeededForLevelRange(int startLevel, int endLevel) {
    long long result = 0;

    for (; startLevel < endLevel; ++startLevel) {
        if (startLevel < 15) {
            result += 2 * startLevel + 7;
        } else if (startLevel < 30) {
            result += 5 * startLevel - 38;
        } else {
            result += 9 * startLevel - 158;
        }
    }

    return result;
}

} // namespace lse::api
