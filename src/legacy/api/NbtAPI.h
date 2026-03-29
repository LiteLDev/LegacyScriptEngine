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
    static Local<Value> newTag(Arguments const& args);
    static Local<Value> parseSNBT(Arguments const& args);
    static Local<Value> parseBinaryNBT(Arguments const& args);

    template <Tag::Type T>
    static Local<Value> getType() {
        return Number::newNumber(static_cast<int>(T));
    }
};
extern ClassDefine<> NbtStaticBuilder;

// NBT Byte
class NbtByteClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ByteTag>, ByteTag*> nbt;

    explicit NbtByteClass(
        Local<Object> const&                                             scriptObj,
        std::variant<std::monostate, std::unique_ptr<ByteTag>, ByteTag*> nbt
    );
    explicit NbtByteClass(std::variant<std::monostate, std::unique_ptr<ByteTag>, ByteTag*> nbt);

    ByteTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ByteTag>>(nbt)) return std::get<std::unique_ptr<ByteTag>>(nbt).get();
        return std::get<ByteTag*>(nbt);
    }

    static NbtByteClass* constructor(Arguments const& args);
    static ByteTag*      extract(Local<Value> const& v);
    static Local<Value>  pack(ByteTag* tag);
    static Local<Value>  pack(std::unique_ptr<ByteTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtByteClass> NbtByteClassBuilder;

// NBT Short
class NbtShortClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ShortTag>, ShortTag*> nbt;

    explicit NbtShortClass(
        Local<Object> const&                                               scriptObj,
        std::variant<std::monostate, std::unique_ptr<ShortTag>, ShortTag*> nbt
    );
    explicit NbtShortClass(std::variant<std::monostate, std::unique_ptr<ShortTag>, ShortTag*> nbt);

    ShortTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ShortTag>>(nbt))
            return std::get<std::unique_ptr<ShortTag>>(nbt).get();
        return std::get<ShortTag*>(nbt);
    }

    static NbtShortClass* constructor(Arguments const& args);
    static ShortTag*      extract(Local<Value> const& v);
    static Local<Value>   pack(ShortTag* tag);
    static Local<Value>   pack(std::unique_ptr<ShortTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtShortClass> NbtShortClassBuilder;

// NBT Int
class NbtIntClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<IntTag>, IntTag*> nbt;

    explicit NbtIntClass(
        Local<Object> const&                                           scriptObj,
        std::variant<std::monostate, std::unique_ptr<IntTag>, IntTag*> nbt
    );
    explicit NbtIntClass(std::variant<std::monostate, std::unique_ptr<IntTag>, IntTag*> nbt);

    IntTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<IntTag>>(nbt)) return std::get<std::unique_ptr<IntTag>>(nbt).get();
        return std::get<IntTag*>(nbt);
    }

    static NbtIntClass* constructor(Arguments const& args);
    static IntTag*      extract(Local<Value> const& v);
    static Local<Value> pack(IntTag* tag);
    static Local<Value> pack(std::unique_ptr<IntTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtIntClass> NbtIntClassBuilder;

// NBT Long
class NbtLongClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<Int64Tag>, Int64Tag*> nbt;

    explicit NbtLongClass(
        Local<Object> const&                                               scriptObj,
        std::variant<std::monostate, std::unique_ptr<Int64Tag>, Int64Tag*> nbt
    );
    explicit NbtLongClass(std::variant<std::monostate, std::unique_ptr<Int64Tag>, Int64Tag*> nbt);

    Int64Tag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<Int64Tag>>(nbt))
            return std::get<std::unique_ptr<Int64Tag>>(nbt).get();
        return std::get<Int64Tag*>(nbt);
    }

    static NbtLongClass* constructor(Arguments const& args);
    static Int64Tag*     extract(Local<Value> const& v);
    static Local<Value>  pack(Int64Tag* tag);
    static Local<Value>  pack(std::unique_ptr<Int64Tag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtLongClass> NbtLongClassBuilder;

// NBT Float
class NbtFloatClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<FloatTag>, FloatTag*> nbt;

    explicit NbtFloatClass(
        Local<Object> const&                                               scriptObj,
        std::variant<std::monostate, std::unique_ptr<FloatTag>, FloatTag*> nbt
    );
    explicit NbtFloatClass(std::variant<std::monostate, std::unique_ptr<FloatTag>, FloatTag*> nbt);

    FloatTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<FloatTag>>(nbt))
            return std::get<std::unique_ptr<FloatTag>>(nbt).get();
        return std::get<FloatTag*>(nbt);
    }

    static NbtFloatClass* constructor(Arguments const& args);
    static FloatTag*      extract(Local<Value> const& v);
    static Local<Value>   pack(FloatTag* tag);
    static Local<Value>   pack(std::unique_ptr<FloatTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtFloatClass> NbtFloatClassBuilder;

// NBT Double
class NbtDoubleClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<DoubleTag>, DoubleTag*> nbt;

    explicit NbtDoubleClass(
        Local<Object> const&                                                 scriptObj,
        std::variant<std::monostate, std::unique_ptr<DoubleTag>, DoubleTag*> nbt
    );
    explicit NbtDoubleClass(std::variant<std::monostate, std::unique_ptr<DoubleTag>, DoubleTag*> nbt);

    DoubleTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<DoubleTag>>(nbt))
            return std::get<std::unique_ptr<DoubleTag>>(nbt).get();
        return std::get<DoubleTag*>(nbt);
    }

    static NbtDoubleClass* constructor(Arguments const& args);
    static DoubleTag*      extract(Local<Value> const& v);
    static Local<Value>    pack(DoubleTag* tag);
    static Local<Value>    pack(std::unique_ptr<DoubleTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtDoubleClass> NbtDoubleClassBuilder;

// NBT String
class NbtStringClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<StringTag>, StringTag*> nbt;

    explicit NbtStringClass(
        Local<Object> const&                                                 scriptObj,
        std::variant<std::monostate, std::unique_ptr<StringTag>, StringTag*> nbt
    );
    explicit NbtStringClass(std::variant<std::monostate, std::unique_ptr<StringTag>, StringTag*> nbt);

    StringTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<StringTag>>(nbt))
            return std::get<std::unique_ptr<StringTag>>(nbt).get();
        return std::get<StringTag*>(nbt);
    }

    static NbtStringClass* constructor(Arguments const& args);
    static StringTag*      extract(Local<Value> const& v);
    static Local<Value>    pack(StringTag* tag);
    static Local<Value>    pack(std::unique_ptr<StringTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtStringClass> NbtStringClassBuilder;

// NBT ByteArray
class NbtByteArrayClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ByteArrayTag>, ByteArrayTag*> nbt;

    explicit NbtByteArrayClass(
        Local<Object> const&                                                       scriptObj,
        std::variant<std::monostate, std::unique_ptr<ByteArrayTag>, ByteArrayTag*> nbt
    );
    explicit NbtByteArrayClass(std::variant<std::monostate, std::unique_ptr<ByteArrayTag>, ByteArrayTag*> nbt);

    ByteArrayTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ByteArrayTag>>(nbt))
            return std::get<std::unique_ptr<ByteArrayTag>>(nbt).get();
        return std::get<ByteArrayTag*>(nbt);
    }

    static NbtByteArrayClass* constructor(Arguments const& args);
    static ByteArrayTag*      extract(Local<Value> const& v);
    static Local<Value>       pack(ByteArrayTag* tag);
    static Local<Value>       pack(std::unique_ptr<ByteArrayTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> set(Arguments const& args) const;
    Local<Value> get(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtByteArrayClass> NbtByteArrayClassBuilder;

// NBT List
class NbtListClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<ListTag>, ListTag*> nbt;

    explicit NbtListClass(
        Local<Object> const&                                             scriptObj,
        std::variant<std::monostate, std::unique_ptr<ListTag>, ListTag*> nbt
    );
    explicit NbtListClass(std::variant<std::monostate, std::unique_ptr<ListTag>, ListTag*> nbt);

    ListTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<ListTag>>(nbt)) return std::get<std::unique_ptr<ListTag>>(nbt).get();
        return std::get<ListTag*>(nbt);
    }

    static NbtListClass* constructor(Arguments const& args);
    static ListTag*      extract(Local<Value> const& v);
    static Local<Value>  pack(ListTag* tag);
    static Local<Value>  pack(std::unique_ptr<ListTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> getSize(Arguments const& args) const;
    Local<Value> getTypeOf(Arguments const& args) const;

    Local<Value> setEnd(Arguments const& args) const;
    Local<Value> setByte(Arguments const& args) const;
    Local<Value> setInt(Arguments const& args) const;
    Local<Value> setShort(Arguments const& args) const;
    Local<Value> setLong(Arguments const& args) const;
    Local<Value> setFloat(Arguments const& args) const;
    Local<Value> setDouble(Arguments const& args) const;
    Local<Value> setString(Arguments const& args) const;
    Local<Value> setByteArray(Arguments const& args) const;
    Local<Value> setTag(Arguments const& args) const;
    Local<Value> addTag(Arguments const& args) const;
    Local<Value> removeTag(Arguments const& args) const;

    Local<Value> getData(Arguments const& args) const;
    Local<Value> getTag(Arguments const& args) const;

    Local<Value> toArray(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;
};
extern ClassDefine<NbtListClass> NbtListClassBuilder;

// NBT Compound
class NbtCompoundClass : public ScriptClass {
public:
    std::variant<std::monostate, std::unique_ptr<CompoundTag>, CompoundTag*> nbt;

    explicit NbtCompoundClass(
        Local<Object> const&                                                     scriptObj,
        std::variant<std::monostate, std::unique_ptr<CompoundTag>, CompoundTag*> nbt
    );
    explicit NbtCompoundClass(std::variant<std::monostate, std::unique_ptr<CompoundTag>, CompoundTag*> nbt);

    CompoundTag* getPtr() const {
        if (std::holds_alternative<std::monostate>(nbt)) return nullptr;
        if (std::holds_alternative<std::unique_ptr<CompoundTag>>(nbt))
            return std::get<std::unique_ptr<CompoundTag>>(nbt).get();
        return std::get<CompoundTag*>(nbt);
    }

    static NbtCompoundClass* constructor(Arguments const& args);
    static CompoundTag*      extract(Local<Value> const& v);
    static Local<Value>      pack(CompoundTag* tag);
    static Local<Value>      pack(std::unique_ptr<CompoundTag> tag);

    Local<Value> getType(Arguments const& args);
    Local<Value> getKeys(Arguments const& args) const;
    Local<Value> getTypeOf(Arguments const& args) const;

    Local<Value> setEnd(Arguments const& args) const;
    Local<Value> setByte(Arguments const& args) const;
    Local<Value> setInt(Arguments const& args) const;
    Local<Value> setShort(Arguments const& args) const;
    Local<Value> setLong(Arguments const& args) const;
    Local<Value> setFloat(Arguments const& args) const;
    Local<Value> setDouble(Arguments const& args) const;
    Local<Value> setString(Arguments const& args) const;
    Local<Value> setByteArray(Arguments const& args) const;
    Local<Value> setTag(Arguments const& args) const;
    Local<Value> removeTag(Arguments const& args) const;

    Local<Value> getData(Arguments const& args) const;
    Local<Value> getTag(Arguments const& args) const;

    Local<Value> toObject(Arguments const& args) const;
    Local<Value> toSNBT(Arguments const& args) const;
    Local<Value> toBinaryNBT(Arguments const& args) const;
    Local<Value> toString(Arguments const& args) const;

    Local<Value> destroy(Arguments const& args);
};
extern ClassDefine<NbtCompoundClass> NbtCompoundClassBuilder;

// Helper
bool         IsNbtClass(Local<Value> const& value);
Local<Value> Tag2Value(Tag* nbt, bool autoExpansion = false);
void         NbtCompoundClassAddHelper(CompoundTag* tag, Local<Object> const& obj);
void         NbtListClassAddHelper(ListTag* tag, Local<Array> const& arr);
