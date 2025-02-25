#include "lse/api/AttributeHelper.h"

#include "mc/world/attribute/AttributeInstance.h"
#include "mc/world/attribute/AttributeModificationContext.h"
#include "mc/world/attribute/BaseAttributeMap.h"

namespace lse::api {
inline void AttributeHelper::setDirty(MutableAttributeWithContext& attribute) {
    auto map = attribute.mContext->mAttributeMap;
    if (map) {
        map->_onAttributeModified(*attribute.mInstance);
    }
}

void AttributeHelper::setCurrentValue(MutableAttributeWithContext& attribute, float value) {
    auto& instance          = attribute.mInstance;
    instance->mCurrentValue = value;
    setDirty(attribute);
}

void AttributeHelper::setMaxValue(MutableAttributeWithContext& attribute, float value) {
    auto& instance             = attribute.mInstance;
    instance->mCurrentMaxValue = value;
    instance->mDefaultMaxValue = value;
    float& currentValue        = instance->mCurrentValue;
    currentValue               = std::max(currentValue, instance->mCurrentMinValue);
    setDirty(attribute);
}

void AttributeHelper::setDefaultValue(MutableAttributeWithContext& attribute, float value) {
    auto&  instance     = attribute.mInstance;
    float& defaultValue = instance->mDefaultValue;
    if (value != defaultValue) {
        defaultValue            = value;
        instance->mCurrentValue = value;
        setDirty(attribute);
    }
}
} // namespace lse::api::AttributeHelper