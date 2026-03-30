#include "ItemHelper.h"

#include "mc/world/item/Item.h"

namespace lse::api {
void ItemHelper::load(ItemStack& itemStack, CompoundTag const& tag) {
    itemStack._loadItem(tag);
    if (auto mItem = itemStack.mItem) {
        mItem->fixupCommon(itemStack);
        if (itemStack.getAuxValue() == 0x7FFF) {
            itemStack.mAuxValue = 0;
        }
    }
};
} // namespace lse::api
