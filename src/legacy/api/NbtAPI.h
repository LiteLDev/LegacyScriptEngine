#pragma once
#include "api/APIHelp.h"
#include "mc/nbt/ByteArrayTag.h"
#include "mc/nbt/DoubleTag.h"
#include "mc/nbt/Tag.h"

#include <mc/nbt/CompoundTag.h>
#include <mc/nbt/FloatTag.h>
#include <mc/nbt/Int64Tag.h>
#include <mc/nbt/IntTag.h>
#include <mc/nbt/ListTag.h>
#include <mc/nbt/ShortTag.h>
#include <mc/nbt/StringTag.h>
#include <memory>

// NBT Static
class NbtStatic : public ScriptClass {
public:
    static Local<Value> newTag(const Arguments& args);
    static Local<Value> parseSNBT(const Arguments& args);
    static Local<Value> parseBinaryNBT(const Arguments& args);

    template <Tag::Type T>
    static Local<Value> getType() {
        return Number::newNumber((int)T);
    }
};
extern ClassDefine<void> NbtStaticBuilder;

// NBT Byte
class NbtByteClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ByteTag>, ByteTag*> nbt;

    explicit NbtByteClass(
        const Local<Object>&                                             scriptObj,
        std::variant<std::monostate, std::unique_ptr<ByteTag>, ByteTag*> nbt
    );
    explicit NbtByteClass(std::variant<std::monostate, std::unique_ptr<ByteTag>, ByteTag*> nbt);

    ByteTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ByteTag>>(nbt)) return std::get<std::unique_ptr<ByteTag>>(nbt).get();
        return std::get<ByteTag*>(nbt);
    }

    static NbtByteClass* constructor(const Arguments& args);
    static ByteTag*      extract(Local<Value> v);
    static Local<Value>  pack(ByteTag* tag);
    static Local<Value>  pack(std::unique_ptr<ByteTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtByteClass> NbtByteClassBuilder;

// NBT Short
class NbtShortClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ShortTag>, ShortTag*> nbt;

    explicit NbtShortClass(
        const Local<Object>&                                               scriptObj,
        std::variant<std::monostate, std::unique_ptr<ShortTag>, ShortTag*> nbt
    );
    explicit NbtShortClass(std::variant<std::monostate, std::unique_ptr<ShortTag>, ShortTag*> nbt);

    ShortTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ShortTag>>(nbt))
            return std::get<std::unique_ptr<ShortTag>>(nbt).get();
        return std::get<ShortTag*>(nbt);
    }

    static NbtShortClass* constructor(const Arguments& args);
    static ShortTag*      extract(Local<Value> v);
    static Local<Value>   pack(ShortTag* tag);
    static Local<Value>   pack(std::unique_ptr<ShortTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtShortClass> NbtShortClassBuilder;

// NBT Int
class NbtIntClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<IntTag>, IntTag*> nbt;

    explicit NbtIntClass(
        const Local<Object>&                                           scriptObj,
        std::variant<std::monostate, std::unique_ptr<IntTag>, IntTag*> nbt
    );
    explicit NbtIntClass(std::variant<std::monostate, std::unique_ptr<IntTag>, IntTag*> nbt);

    IntTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<IntTag>>(nbt)) return std::get<std::unique_ptr<IntTag>>(nbt).get();
        return std::get<IntTag*>(nbt);
    }

    static NbtIntClass* constructor(const Arguments& args);
    static IntTag*      extract(Local<Value> v);
    static Local<Value> pack(IntTag* tag);
    static Local<Value> pack(std::unique_ptr<IntTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtIntClass> NbtIntClassBuilder;

// NBT Long
class NbtLongClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<Int64Tag>, Int64Tag*> nbt;

    explicit NbtLongClass(
        const Local<Object>&                                               scriptObj,
        std::variant<std::monostate, std::unique_ptr<Int64Tag>, Int64Tag*> nbt
    );
    explicit NbtLongClass(std::variant<std::monostate, std::unique_ptr<Int64Tag>, Int64Tag*> nbt);

    Int64Tag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<Int64Tag>>(nbt))
            return std::get<std::unique_ptr<Int64Tag>>(nbt).get();
        return std::get<Int64Tag*>(nbt);
    }

    static NbtLongClass* constructor(const Arguments& args);
    static Int64Tag*     extract(Local<Value> v);
    static Local<Value>  pack(Int64Tag* tag);
    static Local<Value>  pack(std::unique_ptr<Int64Tag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtLongClass> NbtLongClassBuilder;

// NBT Float
class NbtFloatClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<FloatTag>, FloatTag*> nbt;

    explicit NbtFloatClass(
        const Local<Object>&                                               scriptObj,
        std::variant<std::monostate, std::unique_ptr<FloatTag>, FloatTag*> nbt
    );
    explicit NbtFloatClass(std::variant<std::monostate, std::unique_ptr<FloatTag>, FloatTag*> nbt);

    FloatTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<FloatTag>>(nbt))
            return std::get<std::unique_ptr<FloatTag>>(nbt).get();
        return std::get<FloatTag*>(nbt);
    }

    static NbtFloatClass* constructor(const Arguments& args);
    static FloatTag*      extract(Local<Value> v);
    static Local<Value>   pack(FloatTag* tag);
    static Local<Value>   pack(std::unique_ptr<FloatTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtFloatClass> NbtFloatClassBuilder;

// NBT Double
class NbtDoubleClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<DoubleTag>, DoubleTag*> nbt;

    explicit NbtDoubleClass(
        const Local<Object>&                                                 scriptObj,
        std::variant<std::monostate, std::unique_ptr<DoubleTag>, DoubleTag*> nbt
    );
    explicit NbtDoubleClass(std::variant<std::monostate, std::unique_ptr<DoubleTag>, DoubleTag*> nbt);

    DoubleTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<DoubleTag>>(nbt))
            return std::get<std::unique_ptr<DoubleTag>>(nbt).get();
        return std::get<DoubleTag*>(nbt);
    }

    static NbtDoubleClass* constructor(const Arguments& args);
    static DoubleTag*      extract(Local<Value> v);
    static Local<Value>    pack(DoubleTag* tag);
    static Local<Value>    pack(std::unique_ptr<DoubleTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtDoubleClass> NbtDoubleClassBuilder;

// NBT String
class NbtStringClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<StringTag>, StringTag*> nbt;

    explicit NbtStringClass(
        const Local<Object>&                                                 scriptObj,
        std::variant<std::monostate, std::unique_ptr<StringTag>, StringTag*> nbt
    );
    explicit NbtStringClass(std::variant<std::monostate, std::unique_ptr<StringTag>, StringTag*> nbt);

    StringTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<StringTag>>(nbt))
            return std::get<std::unique_ptr<StringTag>>(nbt).get();
        return std::get<StringTag*>(nbt);
    }

    static NbtStringClass* constructor(const Arguments& args);
    static StringTag*      extract(Local<Value> v);
    static Local<Value>    pack(StringTag* tag);
    static Local<Value>    pack(std::unique_ptr<StringTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtStringClass> NbtStringClassBuilder;

// NBT ByteArray
class NbtByteArrayClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ByteArrayTag>, ByteArrayTag*> nbt;

    explicit NbtByteArrayClass(
        const Local<Object>&                                                       scriptObj,
        std::variant<std::monostate, std::unique_ptr<ByteArrayTag>, ByteArrayTag*> nbt
    );
    explicit NbtByteArrayClass(std::variant<std::monostate, std::unique_ptr<ByteArrayTag>, ByteArrayTag*> nbt);

    ByteArrayTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ByteArrayTag>>(nbt))
            return std::get<std::unique_ptr<ByteArrayTag>>(nbt).get();
        return std::get<ByteArrayTag*>(nbt);
    }

    static NbtByteArrayClass* constructor(const Arguments& args);
    static ByteArrayTag*      extract(Local<Value> v);
    static Local<Value>       pack(ByteArrayTag* tag);
    static Local<Value>       pack(std::unique_ptr<ByteArrayTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> set(const Arguments& args);
    Local<Value> get(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtByteArrayClass> NbtByteArrayClassBuilder;

// NBT List
class NbtListClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ListTag>, ListTag*> nbt;

    explicit NbtListClass(
        const Local<Object>&                                             scriptObj,
        std::variant<std::monostate, std::unique_ptr<ListTag>, ListTag*> nbt
    );
    explicit NbtListClass(std::variant<std::monostate, std::unique_ptr<ListTag>, ListTag*> nbt);

    ListTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ListTag>>(nbt)) return std::get<std::unique_ptr<ListTag>>(nbt).get();
        return std::get<ListTag*>(nbt);
    }

    static NbtListClass* constructor(const Arguments& args);
    static ListTag*      extract(Local<Value> v);
    static Local<Value>  pack(ListTag* tag);
    static Local<Value>  pack(std::unique_ptr<ListTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> getSize(const Arguments& args);
    Local<Value> getTypeOf(const Arguments& args);

    Local<Value> setEnd(const Arguments& args);
    Local<Value> setByte(const Arguments& args);
    Local<Value> setInt(const Arguments& args);
    Local<Value> setShort(const Arguments& args);
    Local<Value> setLong(const Arguments& args);
    Local<Value> setFloat(const Arguments& args);
    Local<Value> setDouble(const Arguments& args);
    Local<Value> setString(const Arguments& args);
    Local<Value> setByteArray(const Arguments& args);
    Local<Value> setTag(const Arguments& args);
    Local<Value> addTag(const Arguments& args);
    Local<Value> removeTag(const Arguments& args);

    Local<Value> getData(const Arguments& args);
    Local<Value> getTag(const Arguments& args);

    Local<Value> toArray(const Arguments& args);
    Local<Value> toString(const Arguments& args);
};
extern ClassDefine<NbtListClass> NbtListClassBuilder;

// NBT Compound
class NbtCompoundClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<CompoundTag>, CompoundTag*> nbt;

    explicit NbtCompoundClass(
        const Local<Object>&                                                     scriptObj,
        std::variant<std::monostate, std::unique_ptr<CompoundTag>, CompoundTag*> nbt
    );
    explicit NbtCompoundClass(std::variant<std::monostate, std::unique_ptr<CompoundTag>, CompoundTag*> nbt);

    CompoundTag* getPtr() {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<CompoundTag>>(nbt))
            return std::get<std::unique_ptr<CompoundTag>>(nbt).get();
        return std::get<CompoundTag*>(nbt);
    }

    static NbtCompoundClass* constructor(const Arguments& args);
    static CompoundTag*      extract(Local<Value> v);
    static Local<Value>      pack(CompoundTag* tag);
    static Local<Value>      pack(std::unique_ptr<CompoundTag> tag);

    Local<Value> getType(const Arguments& args);
    Local<Value> getKeys(const Arguments& args);
    Local<Value> getTypeOf(const Arguments& args);

    Local<Value> setEnd(const Arguments& args);
    Local<Value> setByte(const Arguments& args);
    Local<Value> setInt(const Arguments& args);
    Local<Value> setShort(const Arguments& args);
    Local<Value> setLong(const Arguments& args);
    Local<Value> setFloat(const Arguments& args);
    Local<Value> setDouble(const Arguments& args);
    Local<Value> setString(const Arguments& args);
    Local<Value> setByteArray(const Arguments& args);
    Local<Value> setTag(const Arguments& args);
    Local<Value> removeTag(const Arguments& args);

    Local<Value> getData(const Arguments& args);
    Local<Value> getTag(const Arguments& args);

    Local<Value> toObject(const Arguments& args);
    Local<Value> toSNBT(const Arguments& args);
    Local<Value> toBinaryNBT(const Arguments& args);
    Local<Value> toString(const Arguments& args);

    Local<Value> destroy(const Arguments& args);
};
extern ClassDefine<NbtCompoundClass> NbtCompoundClassBuilder;

// Helper
bool         IsNbtClass(Local<Value> value);
Local<Value> Tag2Value(Tag* nbt, bool autoExpansion = false);
void         NbtCompoundClassAddHelper(CompoundTag* tag, Local<Object>& obj);
void         NbtListClassAddHelper(ListTag* tag, Local<Array>& arr);
