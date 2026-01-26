#include "AttributeHelper.h"

#include "mc/world/attribute/Attribute.h"
#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeInstanceRef.h"

namespace lse::api {
inline void AttributeHelper::setDirty(BaseAttributeMap& map, AttributeInstance const* attribute) {
    map._onAttributeModified(*attribute);
}

bool AttributeHelper::setCurrentValue(BaseAttributeMap& map, Attribute const& attribute, float value) {
    if (auto ptr = map.getMutableInstance(attribute.mIDValue).mPtr) {
        ptr->mCurrentValue = value;
        setDirty(map, ptr);
        return true;
    }
    return false;
}

bool AttributeHelper::setMaxValue(BaseAttributeMap& map, Attribute const& attribute, float value) {
    if (auto ptr = map.getMutableInstance(attribute.mIDValue).mPtr) {
        ptr->mCurrentMaxValue = value;
        ptr->mDefaultMaxValue = value;
        float& currentValue   = ptr->mCurrentValue;
        currentValue          = std::max(currentValue, ptr->mCurrentMinValue);
        setDirty(map, ptr);
        return true;
    }
    return false;
}

bool AttributeHelper::setDefaultValue(BaseAttributeMap& map, Attribute const& attribute, float value) {
    if (auto ptr = map.getMutableInstance(attribute.mIDValue).mPtr) {
        float& defaultValue = ptr->mDefaultValue;
        if (value != defaultValue) {
            defaultValue       = value;
            ptr->mCurrentValue = value;
            setDirty(map, ptr);
            return true;
        }
    }
    return false;
}
} // namespace lse::api
