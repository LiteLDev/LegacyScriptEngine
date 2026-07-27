#include "PlayerHelper.h"

#include "AttributeHelper.h"
#include "mc/entity/components/AttributesComponent.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeInstanceConstRef.h"
#include "mc/world/attribute/AttributeInstanceHandle.h" // IWYU pragma: keep

namespace lse::api {

int PlayerHelper::getPreviousLevelRequirement(Player* player) {
    if (player->mPlayerLevelChanged) {
        int level = player->getAttribute(Player::LEVEL()).mPtr->mCurrentValue;
        if (level >= 30) {
            player->mPreviousLevelRequirement = 9 * level - 158;
        } else if (level >= 15) {
            player->mPreviousLevelRequirement = 5 * level - 38;
        } else {
            player->mPreviousLevelRequirement = 2 * level + 7;
        }
        player->mPlayerLevelChanged = false;
    }
    return player->mPreviousLevelRequirement;
}

int PlayerHelper::getXpEarnedAtCurrentLevel(Player* player) {
    int  prevLevelReq = getPreviousLevelRequirement(player);
    auto attribute    = player->getAttribute(Player::EXPERIENCE());
    return static_cast<int>(roundf(attribute.mPtr->mCurrentValue * static_cast<float>(prevLevelReq)));
}

bool PlayerHelper::setXpEarnedAtCurrentLevel(Player* player, int xp) {
    if (auto component = player->getEntityContext().tryGetComponent<AttributesComponent>()) {
        int prevLevelReq = getPreviousLevelRequirement(player);
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
