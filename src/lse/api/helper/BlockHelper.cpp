#include "BlockHelper.h"

#include "mc/world/level/dimension/DimensionHeightRange.h"

namespace lse::api {
bool BlockHelper::isValidHeight(WeakRef<Dimension> dimension, std::variant<int, float> height) {
    auto dim = dimension.lock();
    if (dim) {
        if (std::holds_alternative<int>(height)) {
            int y = std::get<int>(height);
            return dim->mHeightRange->mMin <= y && dim->mHeightRange->mMax >= y;
        } else {
            float y = std::get<float>(height);
            return dim->mHeightRange->mMin <= y && dim->mHeightRange->mMax >= y;
        }
    }

    return false;
}
} // namespace lse::api