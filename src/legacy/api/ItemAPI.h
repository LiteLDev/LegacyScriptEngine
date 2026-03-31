#pragma once
#include "legacy/api/APIHelp.h"

#include <memory>
#include <string>
#include <variant>

//////////////////// Classes ////////////////////
class ItemStack;

class ItemClass : public ScriptClass {
private:
    // ItemStack* is managed by BDS
    std::variant<std::monostate, std::unique_ptr<ItemStack>, ItemStack*> item;

    // Pre data
    std::string name, type;
    int         id, maxCount, count, aux;

public:
    explicit ItemClass(std::variant<std::monostate, std::unique_ptr<ItemStack>, ItemStack*> itemStack);
    void preloadData();

    ItemStack* get() const {
        if (std::holds_alternative<std::monostate>(item)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ItemStack>>(item))
            return std::get<std::unique_ptr<ItemStack>>(item).get();
        return std::get<ItemStack*>(item);
    }

    static Local<Object> newItem(ItemStack* itemStack);
    static Local<Object> newItem(std::unique_ptr<ItemStack> itemStack);
    static ItemStack*    extract(Local<Value> const& v);

    Local<Value> getName() const;
    Local<Value> getType() const;
    Local<Value> getId() const;
    Local<Value> getCount() const;
    Local<Value> getMaxCount() const;
    Local<Value> getAux() const;
    Local<Value> getDamage() const;
    Local<Value> getAttackDamage() const;
    Local<Value> getMaxDamage() const;
    Local<Value> getMaxStackSize() const;
    Local<Value> getLore() const;

    Local<Value> isArmorItem() const;
    Local<Value> isBlock() const;
    Local<Value> isDamageableItem() const;
    Local<Value> isDamaged() const;
    Local<Value> isEnchanted() const;
    Local<Value> isEnchantingBook() const;
    Local<Value> isFireResistant() const;
    Local<Value> isFullStack() const;
    Local<Value> isGlint() const;
    Local<Value> isHorseArmorItem() const;
    Local<Value> isLiquidClipItem() const;
    Local<Value> isMusicDiscItem() const;
    Local<Value> isOffhandItem() const;
    Local<Value> isPotionItem() const;
    Local<Value> isStackable() const;
    Local<Value> isWearableItem() const;

    Local<Value> set(Arguments const& args) const;
    Local<Value> clone(Arguments const& args) const;
    Local<Value> isNull(Arguments const& args) const;
    Local<Value> setNull(Arguments const& args) const;
    Local<Value> setAux(Arguments const& args) const;
    Local<Value> setLore(Arguments const& args) const;
    Local<Value> setDisplayName(Arguments const& args) const;
    Local<Value> setDamage(Arguments const& args) const;
    Local<Value> getNbt(Arguments const& args) const;
    Local<Value> setNbt(Arguments const& args);

    Local<Value> match(Arguments const& args) const;
};

extern ClassDefine<ItemClass> ItemClassBuilder;
