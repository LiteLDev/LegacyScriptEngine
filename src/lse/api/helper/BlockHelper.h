#pragma once
#include "mc/world/level/dimension/Dimension.h"

namespace lse::api {
class BlockHelper {
public:
    static bool isValidHeight(WeakRef<Dimension> const& dimension, std::variant<int, float> height);
};
} // namespace lse::api
