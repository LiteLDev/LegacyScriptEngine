#include "ItemHelper.h"

#include "mc/nbt/CompoundTag.h"
#include "mc/world/item/Item.h"

namespace lse::api {
void ItemHelper::load(ItemStack& itemStack, CompoundTag& tag) {
    itemStack._loadItem(tag);
    auto mItem = itemStack.mItem;
    if (mItem) {
        mItem->fixupCommon(itemStack);
        if (itemStack.getAuxValue() == 0x7FFF) {
            itemStack.mAuxValue = 0;
        }
    }
};
} // namespace lse::api