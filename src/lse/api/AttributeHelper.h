#pragma once
#include "mc/world/attribute/MutableAttributeWithContext.h"

namespace lse::api {
class AttributeHelper {
    static void setDirty(MutableAttributeWithContext& attribute);

public:
    static void setCurrentValue(MutableAttributeWithContext& attribute, float value);
    static void setMaxValue(MutableAttributeWithContext& attribute, float value);
    static void setDefaultValue(MutableAttributeWithContext& attribute, float value);
};
} // namespace lse::api