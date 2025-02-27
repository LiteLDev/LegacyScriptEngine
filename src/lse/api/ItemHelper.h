#include "mc/world/item/ItemStack.h"

namespace lse::api {
class ItemHelper {
public:
    static void load(ItemStack& itemStack, CompoundTag& tag);
};
} // namespace lse::api