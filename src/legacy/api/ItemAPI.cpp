#include "legacy/api/ItemAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "lse/api/helper/ItemHelper.h"
#include "mc/common/SharedPtr.h"
#include "mc/safety/RedactableString.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/dimension/Dimension.h"

#include <string>
#include <variant>
#include <vector>

using lse::api::ItemHelper;

//////////////////// Class Definition ////////////////////

ClassDefine<ItemClass> ItemClassBuilder = defineClass<ItemClass>("LLSE_Item")
                                              .constructor(nullptr)
                                              .instanceProperty("name", &ItemClass::getName)
                                              .instanceProperty("type", &ItemClass::getType)
                                              .instanceProperty("id", &ItemClass::getId)
                                              .instanceProperty("count", &ItemClass::getCount)
                                              .instanceProperty("maxCount", &ItemClass::getMaxCount)
                                              .instanceProperty("aux", &ItemClass::getAux)
                                              .instanceProperty("damage", &ItemClass::getDamage)
                                              .instanceProperty("lore", &ItemClass::getLore)
                                              .instanceProperty("attackDamage", &ItemClass::getAttackDamage)
                                              .instanceProperty("maxDamage", &ItemClass::getMaxDamage)
                                              .instanceProperty("maxStackSize", &ItemClass::getMaxStackSize)
                                              .instanceProperty("isArmorItem", &ItemClass::isArmorItem)
                                              .instanceProperty("isBlock", &ItemClass::isBlock)
                                              .instanceProperty("isDamageableItem", &ItemClass::isDamageableItem)
                                              .instanceProperty("isDamaged", &ItemClass::isDamaged)
                                              .instanceProperty("isEnchanted", &ItemClass::isEnchanted)
                                              .instanceProperty("isEnchantingBook", &ItemClass::isEnchantingBook)
                                              .instanceProperty("isFireResistant", &ItemClass::isFireResistant)
                                              .instanceProperty("isFullStack", &ItemClass::isFullStack)
                                              .instanceProperty("isGlint", &ItemClass::isGlint)
                                              .instanceProperty("isHorseArmorItem", &ItemClass::isHorseArmorItem)
                                              .instanceProperty("isLiquidClipItem", &ItemClass::isLiquidClipItem)
                                              .instanceProperty("isMusicDiscItem", &ItemClass::isMusicDiscItem)
                                              .instanceProperty("isOffhandItem", &ItemClass::isOffhandItem)
                                              .instanceProperty("isPotionItem", &ItemClass::isPotionItem)
                                              .instanceProperty("isStackable", &ItemClass::isStackable)
                                              .instanceProperty("isWearableItem", &ItemClass::isWearableItem)

                                              .instanceFunction("set", &ItemClass::set)
                                              .instanceFunction("clone", &ItemClass::clone)
                                              .instanceFunction("isNull", &ItemClass::isNull)
                                              .instanceFunction("setNull", &ItemClass::setNull)
                                              .instanceFunction("setAux", &ItemClass::setAux)
                                              .instanceFunction("setLore", &ItemClass::setLore)
                                              .instanceFunction("setDisplayName", &ItemClass::setDisplayName)
                                              .instanceFunction("setDamage", &ItemClass::setDamage)
                                              .instanceFunction("setNbt", &ItemClass::setNbt)
                                              .instanceFunction("getNbt", &ItemClass::getNbt)

                                              .instanceFunction("match", &ItemClass::match)

                                              // For Compatibility
                                              .instanceFunction("setTag", &ItemClass::setNbt)
                                              .instanceFunction("getTag", &ItemClass::getNbt)
                                              .build();

//////////////////// Classes ////////////////////

ItemClass::ItemClass(std::variant<std::monostate, std::unique_ptr<ItemStack>, ItemStack*> itemStack)
: ScriptClass(ScriptClass::ConstructFromCpp<ItemClass>{}) {
    item = std::move(itemStack);
    preloadData();
}

// 生成函数
Local<Object> ItemClass::newItem(ItemStack* itemStack) {
    auto newp = new ItemClass(itemStack);
    return newp->getScriptObject();
}

Local<Object> ItemClass::newItem(std::unique_ptr<ItemStack> itemStack) {
    auto newp = new ItemClass(std::move(itemStack));
    return newp->getScriptObject();
}

ItemStack* ItemClass::extract(Local<Value> const& v) {
    if (EngineScope::currentEngine()->isInstanceOf<ItemClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<ItemClass>(v)->get();
    return nullptr;
}

// 成员函数
void ItemClass::preloadData() {
    name = get()->getCustomName();
    if (name.empty()) name = get()->getName();

    type     = get()->getTypeName();
    id       = get()->getId();
    count    = get()->mCount;
    maxCount = get()->getMaxStackSize();
    aux      = get()->getAuxValue();
}

Local<Value> ItemClass::getName() const {
    try {
        // 已预加载
        return String::newString(name);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getType() const {
    try {
        // 已预加载
        return String::newString(type);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getId() const {
    try {
        // 已预加载
        return Number::newNumber(id);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getCount() const {
    try {
        // 已预加载
        return Number::newNumber(count);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getMaxCount() const {
    try {
        // 已预加载
        return Number::newNumber(maxCount);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getAux() const {
    try {
        // 已预加载
        return Number::newNumber(aux);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getDamage() const {
    try {
        return Number::newNumber(get()->getDamageValue());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getAttackDamage() const {
    try {
        if (auto mItem = get()->mItem) {
            return Number::newNumber(mItem->getAttackDamage());
        }
        return Number::newNumber(0);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getMaxDamage() const {
    try {
        return Number::newNumber(get()->getMaxDamage());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getMaxStackSize() const {
    try {
        return Number::newNumber(get()->getMaxStackSize());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getLore() const {
    try {
        std::vector<std::string> loreArray = get()->getCustomLore();

        Local<Array> loreValueList = Array::newArray();

        for (std::string lore : loreArray) {
            loreValueList.add(String::newString(lore));
        }

        return loreValueList;
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isArmorItem() const {
    try {
        return Boolean::newBoolean(get()->isArmorItem());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isBlock() const {
    try {
        return Boolean::newBoolean(get()->isBlock());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isDamageableItem() const {
    try {
        return Boolean::newBoolean(get()->isDamageableItem());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isDamaged() const {
    try {
        return Boolean::newBoolean(get()->getDamageValue() > 0);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isEnchanted() const {
    try {
        return Boolean::newBoolean(get()->isEnchanted());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isEnchantingBook() const {
    try {
        return Boolean::newBoolean(get()->isEnchantingBook());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isFireResistant() const {
    try {
        if (auto mItem = get()->mItem) {
            return Boolean::newBoolean(mItem->mFireResistant);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isFullStack() const {
    try {
        return Boolean::newBoolean(get()->mCount >= get()->getMaxStackSize());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isGlint() const {
    try {
        return Boolean::newBoolean(get()->mItem->mIsGlint);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isHorseArmorItem() const {
    try {
        return Boolean::newBoolean(get()->isHorseArmorItem());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isLiquidClipItem() const {
    try {
        if (auto mItem = get()->mItem) {
            return Boolean::newBoolean(mItem->isLiquidClipItem());
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isMusicDiscItem() const {
    try {
        return Boolean::newBoolean(get()->getItem()->isMusicDisk());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isOffhandItem() const {
    try {
        if (auto mItem = get()->mItem) {
            return Boolean::newBoolean(mItem->mAllowOffhand);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isPotionItem() const {
    try {
        return Boolean::newBoolean(get()->isPotionItem());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isStackable() const {
    try {
        if (get()->getMaxStackSize() > 1u && get()->getDamageValue() <= 0) {
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isWearableItem() const {
    try {
        if (get()->mItem) {
            return Boolean::newBoolean(get()->isHumanoidWearableBlockItem());
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::set(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto itemNew = ItemClass::extract(args[0]);
        if (!itemNew) return {}; // Null

        auto tag = itemNew->save(*SaveContextFactory::createCloneSaveContext());
        if (std::holds_alternative<std::unique_ptr<ItemStack>>(item)) {
            ItemHelper::load(*std::get<std::unique_ptr<ItemStack>>(item), *tag);
        } else {
            ItemHelper::load(*std::get<ItemStack*>(item), *tag);
        }
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::clone(Arguments const&) const {
    try {
        auto itemStack = get();
        if (!itemStack) return {};
        return ItemClass::newItem(std::make_unique<ItemStack>(*itemStack));
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::isNull(Arguments const&) const {
    try {
        return Boolean::newBoolean(get()->isNull());
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::setNull(Arguments const&) const {
    try {
        get()->setNull({});
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::setAux(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        get()->mAuxValue = args[0].asNumber().toInt32();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::setLore(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kArray);

    try {
        auto                     arr = args[0].asArray();
        std::vector<std::string> lores;
        for (int i = 0; i < arr.size(); ++i) {
            auto value = arr.get(i);
            if (value.getKind() == ValueKind::kString) lores.push_back(value.asString().toString());
        }
        if (lores.empty()) return Boolean::newBoolean(false);

        get()->setCustomLore(lores);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::setDisplayName(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        Bedrock::Safety::RedactableString redactableString;
        redactableString.mUnredactedString = args[0].asString().toString();
        get()->setCustomName(redactableString);
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::setDamage(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        if (get()->isDamageableItem() && args[0].asNumber().toInt32() <= 32767) {
            get()->setDamageValue(args[0].asNumber().toInt32());
            return Boolean::newBoolean(true);
        }
        return Boolean::newBoolean(false);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::getNbt(Arguments const&) const {
    try {
        return NbtCompoundClass::pack(get()->save(*SaveContextFactory::createCloneSaveContext()));
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::setNbt(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) return {}; // Null
        ItemHelper::load(*get(), *nbt);
        // update Pre Data
        preloadData();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::newItem(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        if (args[0].isString()) {
            // name & count
            if (args.size() >= 2 && args[1].isNumber()) {
                std::string type = args[0].asString().toString();
                int         cnt  = args[1].asNumber().toInt32();
                auto        item = std::make_unique<ItemStack>();
                item->reinit(type, cnt, 0);
                return ItemClass::newItem(std::move(item));
            }
            throw TooFewArgsException(__FUNCTION__);
        }
        if (auto nbt = NbtCompoundClass::extract(args[0])) {
            auto newItem = std::make_unique<ItemStack>(ItemStack::EMPTY_ITEM());
            ItemHelper::load(*newItem, *nbt);
            return ItemClass::newItem(std::move(newItem));
        }
        throw WrongArgTypeException(__FUNCTION__);
    }
    CATCH_AND_THROW
}

Local<Value> McClass::spawnItem(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);

    try {
        FloatVec4 pos;
        if (args.size() == 2) {
            if (IsInstanceOf<IntPos>(args[1])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = *posObj;
            } else if (IsInstanceOf<FloatPos>(args[1])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                pos = static_cast<FloatVec4>(*posObj);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        } else if (args.size() == 5) {
            // Number Pos
            CHECK_ARG_TYPE(args[1], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[2], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[3], ValueKind::kNumber);
            CHECK_ARG_TYPE(args[4], ValueKind::kNumber);
            pos = {
                args[1].asNumber().toFloat(),
                args[2].asNumber().toFloat(),
                args[3].asNumber().toFloat(),
                args[4].asNumber().toInt32()
            };
        } else {
            throw WrongArgsCountException(__FUNCTION__);
        }

        if (ItemStack* it = ItemClass::extract(args[0])) {
            // By Item
            ;
            ItemActor* entity = ll::service::getLevel()->getSpawner().spawnItem(
                ll::service::getLevel()->getDimension(pos.dim).lock()->getBlockSourceFromMainChunkSource(),
                *it,
                nullptr,
                pos.getVec3(),
                pos.dim
            );
            if (!entity) return {}; // Null
            return EntityClass::newEntity(entity);
        }
        throw WrongArgTypeException(__FUNCTION__);
    }
    CATCH_AND_THROW
}

Local<Value> ItemClass::match(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kObject)
    if (!IsInstanceOf<ItemClass>(args[0])) {
        throw WrongArgTypeException(__FUNCTION__);
    }

    try {
        ItemStackBase itemNew = static_cast<ItemStackBase>(*extract(args[0]));
        if (!itemNew) return Boolean::newBoolean(false);

        return Boolean::newBoolean(get()->matchesItem(itemNew));
    }
    CATCH_AND_THROW
}
