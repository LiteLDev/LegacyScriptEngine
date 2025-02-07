#include "api/ItemAPI.h"

#include "api/APIHelp.h"
#include "api/BaseAPI.h"
#include "api/EntityAPI.h"
#include "api/McAPI.h"
#include "api/NbtAPI.h"
#include "ll/api/service/Bedrock.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/safety/RedactableString.h"
#include "mc/world/actor/Actor.h"
#include "mc/world/actor/item/ItemActor.h"
#include "mc/world/item/Item.h"
#include "mc/world/item/ItemStack.h"
#include "mc/world/item/SaveContextFactory.h"
#include "mc/world/level/Spawner.h"
#include "mc/world/level/dimension/Dimension.h"

#include <string>
#include <variant>
#include <vector>

//////////////////// Class Definition ////////////////////

ClassDefine<ItemClass> ItemClassBuilder = defineClass<ItemClass>("LLSE_Item")
                                              .constructor(nullptr)
                                              .instanceProperty("name", &ItemClass::getName)
                                              .instanceProperty("type", &ItemClass::getType)
                                              .instanceProperty("id", &ItemClass::getId)
                                              .instanceProperty("count", &ItemClass::getCount)
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

ItemClass::ItemClass(ItemStack* itemStack, bool isManagedByBDS)
: ScriptClass(ScriptClass::ConstructFromCpp<ItemClass>{}) {
    if (isManagedByBDS) {
        item = itemStack;
    } else {
        item = std::unique_ptr<ItemStack>(itemStack);
    }
    preloadData();
}

// 生成函数
Local<Object> ItemClass::newItem(ItemStack* itemStack, bool isManagedByBDS) {
    auto newp = new ItemClass(itemStack, isManagedByBDS);
    return newp->getScriptObject();
}

ItemStack* ItemClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<ItemClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<ItemClass>(v)->get();
    else return nullptr;
}

// 成员函数
void ItemClass::preloadData() {
    ;
    name = get()->getCustomName();
    if (name.empty()) name = get()->getName();

    type  = get()->getTypeName();
    id    = get()->getId();
    count = get()->mCount;
    aux   = get()->getAuxValue();
}

Local<Value> ItemClass::getName() {
    try {
        // 已预加载
        return String::newString(name);
    }
    CATCH("Fail in GetItemName!");
}

Local<Value> ItemClass::getType() {
    try {
        // 已预加载
        return String::newString(type);
    }
    CATCH("Fail in GetType!");
}

Local<Value> ItemClass::getId() {
    try {
        // 已预加载
        return Number::newNumber(id);
    }
    CATCH("Fail in GetType!");
}

Local<Value> ItemClass::getCount() {
    try {
        // 已预加载
        return Number::newNumber(count);
    }
    CATCH("Fail in GetCount!");
}

Local<Value> ItemClass::getAux() {
    try {
        // 已预加载
        return Number::newNumber(aux);
    }
    CATCH("Fail in GetAux!");
}

Local<Value> ItemClass::getDamage() {
    try {
        return Number::newNumber(get()->getDamageValue());
    }
    CATCH("Fail in GetDamage!");
}

Local<Value> ItemClass::getAttackDamage() {
    try {
        return Number::newNumber(get()->getAttackDamage());
    }
    CATCH("Fail in GetAttackDamage!");
}

Local<Value> ItemClass::getMaxDamage() {
    try {
        return Number::newNumber(get()->getMaxDamage());
    }
    CATCH("Fail in GetMaxDamage!");
}

Local<Value> ItemClass::getMaxStackSize() {
    try {
        return Number::newNumber(get()->getMaxStackSize());
    }
    CATCH("Fail in GetMaxStackSize!");
}

Local<Value> ItemClass::getLore() {
    try {
        std::vector<std::string> loreArray = get()->getCustomLore();

        Local<Array> loreValueList = Array::newArray();

        for (std::string lore : loreArray) {
            loreValueList.add(String::newString(lore));
        }

        return loreValueList;
    }
    CATCH("Fail in GetLore!");
}

Local<Value> ItemClass::isArmorItem() {
    try {
        return Boolean::newBoolean(get()->isArmorItem());
    }
    CATCH("Fail in isArmorItem!");
}

Local<Value> ItemClass::isBlock() {
    try {
        return Boolean::newBoolean(get()->isBlock());
    }
    CATCH("Fail in isBlock!");
}

Local<Value> ItemClass::isDamageableItem() {
    try {
        return Boolean::newBoolean(get()->isDamageableItem());
    }
    CATCH("Fail in isDamageableItem!");
}

Local<Value> ItemClass::isDamaged() {
    try {
        return Boolean::newBoolean(get()->isDamaged());
    }
    CATCH("Fail in isDamaged!");
}

Local<Value> ItemClass::isEnchanted() {
    try {
        return Boolean::newBoolean(get()->isEnchanted());
    }
    CATCH("Fail in isEnchanted!");
}

Local<Value> ItemClass::isEnchantingBook() {
    try {
        return Boolean::newBoolean(get()->isEnchantingBook());
    }
    CATCH("Fail in isEnchantingBook!");
}

Local<Value> ItemClass::isFireResistant() {
    try {
        return Boolean::newBoolean(get()->isFireResistant());
    }
    CATCH("Fail in isFireResistant!");
}

Local<Value> ItemClass::isFullStack() {
    try {
        return Boolean::newBoolean(get()->isFullStack());
    }
    CATCH("Fail in isFullStack!");
}

Local<Value> ItemClass::isGlint() {
    try {
        return Boolean::newBoolean(get()->isGlint());
    }
    CATCH("Fail in isGlint!");
}

Local<Value> ItemClass::isHorseArmorItem() {
    try {
        return Boolean::newBoolean(get()->isHorseArmorItem());
    }
    CATCH("Fail in isHorseArmorItem!");
}

Local<Value> ItemClass::isLiquidClipItem() {
    try {
        return Boolean::newBoolean(get()->isLiquidClipItem());
    }
    CATCH("Fail in isLiquidClipItem!");
}

Local<Value> ItemClass::isMusicDiscItem() {
    try {
        return Boolean::newBoolean(get()->getItem()->isMusicDisk());
    }
    CATCH("Fail in isMusicDiscItem!");
}

Local<Value> ItemClass::isOffhandItem() {
    try {
        return Boolean::newBoolean(get()->isOffhandItem());
    }
    CATCH("Fail in isOffhandItem!");
}

Local<Value> ItemClass::isPotionItem() {
    try {
        return Boolean::newBoolean(get()->isPotionItem());
    }
    CATCH("Fail in isPotionItem!");
}

Local<Value> ItemClass::isStackable() {
    try {
        return Boolean::newBoolean(get()->isStackable());
    }
    CATCH("Fail in isStackable!");
}

Local<Value> ItemClass::isWearableItem() {
    try {
        return Boolean::newBoolean(get()->isHumanoidWearableItem());
    }
    CATCH("Fail in isWearableItem!");
}

Local<Value> ItemClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto itemNew = ItemClass::extract(args[0]);
        if (!itemNew) return Local<Value>(); // Null

        auto tag = itemNew->save(*SaveContextFactory::createCloneSaveContext());
        if (std::holds_alternative<std::unique_ptr<ItemStack>>(item)) {
            std::get<std::unique_ptr<ItemStack>>(item)->load(*tag);
        } else {
            std::get<ItemStack*>(item)->load(*tag);
        }
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in set!");
}

Local<Value> ItemClass::clone(const Arguments&) {
    try {
        auto itemStack = get();
        if (!itemStack) return Local<Value>(); // Null
        auto itemNew = new ItemStack(*itemStack);
        return ItemClass::newItem(itemNew, false);
    }
    CATCH("Fail in cloneItem!");
}

Local<Value> ItemClass::isNull(const Arguments&) {
    try {
        return Boolean::newBoolean(get()->isNull());
    }
    CATCH("Fail in isNull!");
}

Local<Value> ItemClass::setNull(const Arguments&) {
    try {
        get()->setNull({});
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setNull!");
}

Local<Value> ItemClass::setAux(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        get()->setAuxValue(args[0].asNumber().toInt32());
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setAux!");
}

Local<Value> ItemClass::setLore(const Arguments& args) {
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
    CATCH("Fail in SetLore!");
}

Local<Value> ItemClass::setDisplayName(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        get()->setCustomName(Bedrock::Safety::RedactableString(args[0].asString().toString()));
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setDisplayName!");
}

Local<Value> ItemClass::setDamage(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        if (get()->isDamageableItem() && args[0].asNumber().toInt32() <= 32767) {
            get()->setDamageValue(args[0].asNumber().toInt32());
            return Boolean::newBoolean(true);
        } else {
            return Boolean::newBoolean(false);
        }
    }
    CATCH("Fail in setDamage!");
}

Local<Value> ItemClass::getNbt(const Arguments&) {
    try {
        return NbtCompoundClass::pack(get()->save(*SaveContextFactory::createCloneSaveContext()));
    }
    CATCH("Fail in getNbt!");
}

Local<Value> ItemClass::setNbt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        auto nbt = NbtCompoundClass::extract(args[0]);
        if (!nbt) return Local<Value>(); // Null
        auto itemStack = get();
        itemStack->load(*nbt);
        // update Pre Data
        preloadData();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in setNbt!");
}

Local<Value> McClass::newItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        if (args[0].isString()) {
            // name & count
            if (args.size() >= 2 && args[1].isNumber()) {
                std::string type = args[0].asString().toString();
                int         cnt  = args[1].asNumber().toInt32();

                ItemStack* item = new ItemStack{type, cnt, 0, nullptr};
                if (!item) return Local<Value>();            // Null
                else return ItemClass::newItem(item, false); // Not managed by BDS, pointer will be saved as unique_ptr
            } else {
                LOG_TOO_FEW_ARGS(__FUNCTION__);
                return Local<Value>();
            }
        } else {
            auto nbt = NbtCompoundClass::extract(args[0]);
            if (nbt) {
                auto newItem = new ItemStack{ItemStack::EMPTY_ITEM()};
                newItem->load(*nbt);
                if (!newItem) return Local<Value>(); // Null
                else
                    return ItemClass::newItem(
                        newItem,
                        false
                    ); // Not managed by BDS, pointer will be saved as unique_ptr
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
            }
        }
    }
    CATCH("Fail in NewItem!");
}

Local<Value> McClass::spawnItem(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);

    try {
        FloatVec4 pos;
        if (args.size() == 2) {
            if (IsInstanceOf<IntPos>(args[1])) {
                // IntPos
                IntPos* posObj = IntPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos.x   = posObj->x;
                    pos.y   = posObj->y;
                    pos.z   = posObj->z;
                    pos.dim = posObj->dim;
                }
            } else if (IsInstanceOf<FloatPos>(args[1])) {
                // FloatPos
                FloatPos* posObj = FloatPos::extractPos(args[1]);
                if (posObj->dim < 0) return Boolean::newBoolean(false);
                else {
                    pos = *posObj;
                }
            } else {
                LOG_WRONG_ARG_TYPE(__FUNCTION__);
                return Local<Value>();
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
            LOG_WRONG_ARGS_COUNT(__FUNCTION__);
            return Local<Value>();
        }

        ItemStack* it = ItemClass::extract(args[0]);
        if (it) {
            // By Item
            ;
            ItemActor* entity = ll::service::getLevel()->getSpawner().spawnItem(
                ll::service::getLevel()->getDimension(pos.dim)->getBlockSourceFromMainChunkSource(),
                *it,
                0,
                pos.getVec3(),
                pos.dim
            );
            if (!entity) return Local<Value>(); // Null
            else return EntityClass::newEntity(entity);
        } else {
            LOG_WRONG_ARG_TYPE(__FUNCTION__);
            return Local<Value>();
        }
    }
    CATCH("Fail in SpawnItem!");
}

Local<Value> ItemClass::match(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kObject)
    if (!IsInstanceOf<ItemClass>(args[0])) {
        LOG_WRONG_ARG_TYPE(__FUNCTION__);
        return Boolean::newBoolean(false);
    }

    try {
        ItemStackBase itemNew = *ItemClass::extract(args[0]);
        if (!itemNew) return Boolean::newBoolean(false);

        return Boolean::newBoolean(get()->matchesItem(itemNew));
    }
    CATCH("Fail in MatchItem!");
}
