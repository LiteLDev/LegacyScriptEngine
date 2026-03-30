#pragma once
#include "mc/world/item/ItemStack.h"

namespace lse::api {
class ItemHelper {
public:
    static void load(ItemStack& itemStack, CompoundTag const& tag);
};
} // namespace lse::api
