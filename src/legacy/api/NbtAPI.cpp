#include "api/NbtAPI.h"

#include "api/APIHelp.h"
#include "api/BlockAPI.h"
#include "api/ItemAPI.h"
#include "legacyapi/Base64.h"
#include "mc/nbt/ByteArrayTag.h"
#include "mc/nbt/ByteTag.h"
#include "mc/nbt/CompoundTag.h"
#include "mc/nbt/CompoundTagVariant.h"
#include "mc/nbt/DoubleTag.h"
#include "mc/nbt/EndTag.h"
#include "mc/nbt/FloatTag.h"
#include "mc/nbt/Int64Tag.h"
#include "mc/nbt/IntTag.h"
#include "mc/nbt/ListTag.h"
#include "mc/nbt/ShortTag.h"
#include "mc/nbt/StringTag.h"

#include <magic_enum.hpp>
#include <mc/nbt/CompoundTag.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

using magic_enum::enum_cast;

//////////////////// Class Definition ////////////////////

ClassDefine<void> NbtStaticBuilder = defineClass("NBT")
                                         .function("parseSNBT", &NbtStatic::parseSNBT)
                                         .function("parseBinaryNBT", &NbtStatic::parseBinaryNBT)
                                         .property("End", &NbtStatic::getType<Tag::Type::End>)
                                         .property("Byte", &NbtStatic::getType<Tag::Type::Byte>)
                                         .property("Short", &NbtStatic::getType<Tag::Type::Short>)
                                         .property("Int", &NbtStatic::getType<Tag::Type::Int>)
                                         .property("Long", &NbtStatic::getType<Tag::Type::Int64>)
                                         .property("Float", &NbtStatic::getType<Tag::Type::Float>)
                                         .property("Double", &NbtStatic::getType<Tag::Type::Double>)
                                         .property("ByteArray", &NbtStatic::getType<Tag::Type::ByteArray>)
                                         .property("String", &NbtStatic::getType<Tag::Type::String>)
                                         .property("List", &NbtStatic::getType<Tag::Type::List>)
                                         .property("Compound", &NbtStatic::getType<Tag::Type::Compound>)

                                         // For Compatibility
                                         .function("createTag", &NbtStatic::newTag)
                                         .function("newTag", &NbtStatic::newTag)
                                         .build();

ClassDefine<NbtByteClass> NbtByteClassBuilder = defineClass<NbtByteClass>("NbtByte")
                                                    .constructor(&NbtByteClass::constructor)
                                                    .instanceFunction("getType", &NbtByteClass::getType)
                                                    .instanceFunction("toString", &NbtByteClass::toString)
                                                    .instanceFunction("set", &NbtByteClass::set)
                                                    .instanceFunction("get", &NbtByteClass::get)
                                                    .build();

ClassDefine<NbtShortClass> NbtShortClassBuilder = defineClass<NbtShortClass>("NbtShort")
                                                      .constructor(&NbtShortClass::constructor)
                                                      .instanceFunction("getType", &NbtShortClass::getType)
                                                      .instanceFunction("toString", &NbtShortClass::toString)
                                                      .instanceFunction("set", &NbtShortClass::set)
                                                      .instanceFunction("get", &NbtShortClass::get)
                                                      .build();

ClassDefine<NbtIntClass> NbtIntClassBuilder = defineClass<NbtIntClass>("NbtInt")
                                                  .constructor(&NbtIntClass::constructor)
                                                  .instanceFunction("getType", &NbtIntClass::getType)
                                                  .instanceFunction("toString", &NbtIntClass::toString)
                                                  .instanceFunction("set", &NbtIntClass::set)
                                                  .instanceFunction("get", &NbtIntClass::get)
                                                  .build();

ClassDefine<NbtLongClass> NbtLongClassBuilder = defineClass<NbtLongClass>("NbtLong")
                                                    .constructor(&NbtLongClass::constructor)
                                                    .instanceFunction("getType", &NbtLongClass::getType)
                                                    .instanceFunction("toString", &NbtLongClass::toString)
                                                    .instanceFunction("set", &NbtLongClass::set)
                                                    .instanceFunction("get", &NbtLongClass::get)
                                                    .build();

ClassDefine<NbtFloatClass> NbtFloatClassBuilder = defineClass<NbtFloatClass>("NbtFloat")
                                                      .constructor(&NbtFloatClass::constructor)
                                                      .instanceFunction("getType", &NbtFloatClass::getType)
                                                      .instanceFunction("toString", &NbtFloatClass::toString)
                                                      .instanceFunction("set", &NbtFloatClass::set)
                                                      .instanceFunction("get", &NbtFloatClass::get)
                                                      .build();

ClassDefine<NbtDoubleClass> NbtDoubleClassBuilder = defineClass<NbtDoubleClass>("NbtDouble")
                                                        .constructor(&NbtDoubleClass::constructor)
                                                        .instanceFunction("getType", &NbtDoubleClass::getType)
                                                        .instanceFunction("toString", &NbtDoubleClass::toString)
                                                        .instanceFunction("set", &NbtDoubleClass::set)
                                                        .instanceFunction("get", &NbtDoubleClass::get)
                                                        .build();

ClassDefine<NbtStringClass> NbtStringClassBuilder = defineClass<NbtStringClass>("NbtString")
                                                        .constructor(&NbtStringClass::constructor)
                                                        .instanceFunction("getType", &NbtStringClass::getType)
                                                        .instanceFunction("toString", &NbtStringClass::toString)
                                                        .instanceFunction("set", &NbtStringClass::set)
                                                        .instanceFunction("get", &NbtStringClass::get)
                                                        .build();

ClassDefine<NbtByteArrayClass> NbtByteArrayClassBuilder =
    defineClass<NbtByteArrayClass>("NbtByteArray")
        .constructor(&NbtByteArrayClass::constructor)
        .instanceFunction("getType", &NbtByteArrayClass::getType)
        .instanceFunction("toString", &NbtByteArrayClass::toString)
        .instanceFunction("set", &NbtByteArrayClass::set)
        .instanceFunction("get", &NbtByteArrayClass::get)
        .build();

ClassDefine<NbtListClass> NbtListClassBuilder = defineClass<NbtListClass>("NbtList")
                                                    .constructor(&NbtListClass::constructor)
                                                    .instanceFunction("getType", &NbtListClass::getType)
                                                    .instanceFunction("toString", &NbtListClass::toString)
                                                    .instanceFunction("getSize", &NbtListClass::getSize)
                                                    .instanceFunction("getTypeOf", &NbtListClass::getTypeOf)
                                                    .instanceFunction("setEnd", &NbtListClass::setEnd)
                                                    .instanceFunction("setByte", &NbtListClass::setByte)
                                                    .instanceFunction("setInt", &NbtListClass::setInt)
                                                    .instanceFunction("setShort", &NbtListClass::setShort)
                                                    .instanceFunction("setLong", &NbtListClass::setLong)
                                                    .instanceFunction("setFloat", &NbtListClass::setFloat)
                                                    .instanceFunction("setDouble", &NbtListClass::setDouble)
                                                    .instanceFunction("setString", &NbtListClass::setString)
                                                    .instanceFunction("setByteArray", &NbtListClass::setByteArray)
                                                    .instanceFunction("setTag", &NbtListClass::setTag)
                                                    .instanceFunction("addTag", &NbtListClass::addTag)
                                                    .instanceFunction("removeTag", &NbtListClass::removeTag)
                                                    .instanceFunction("getData", &NbtListClass::getData)
                                                    .instanceFunction("getTag", &NbtListClass::getTag)
                                                    .instanceFunction("toArray", &NbtListClass::toArray)
                                                    .build();

ClassDefine<NbtCompoundClass> NbtCompoundClassBuilder =
    defineClass<NbtCompoundClass>("NbtCompound")
        .constructor(&NbtCompoundClass::constructor)
        .instanceFunction("getType", &NbtCompoundClass::getType)
        .instanceFunction("toString", &NbtCompoundClass::toString)
        .instanceFunction("getKeys", &NbtCompoundClass::getKeys)
        .instanceFunction("getTypeOf", &NbtCompoundClass::getTypeOf)
        .instanceFunction("setEnd", &NbtCompoundClass::setEnd)
        .instanceFunction("setByte", &NbtCompoundClass::setByte)
        .instanceFunction("setInt", &NbtCompoundClass::setInt)
        .instanceFunction("setShort", &NbtCompoundClass::setShort)
        .instanceFunction("setLong", &NbtCompoundClass::setLong)
        .instanceFunction("setFloat", &NbtCompoundClass::setFloat)
        .instanceFunction("setDouble", &NbtCompoundClass::setDouble)
        .instanceFunction("setString", &NbtCompoundClass::setString)
        .instanceFunction("setByteArray", &NbtCompoundClass::setByteArray)
        .instanceFunction("setTag", &NbtCompoundClass::setTag)
        .instanceFunction("removeTag", &NbtCompoundClass::removeTag)
        .instanceFunction("getData", &NbtCompoundClass::getData)
        .instanceFunction("getTag", &NbtCompoundClass::getTag)
        .instanceFunction("toObject", &NbtCompoundClass::toObject)
        .instanceFunction("toSNBT", &NbtCompoundClass::toSNBT)
        .instanceFunction("toBinaryNBT", &NbtCompoundClass::toBinaryNBT)
        .instanceFunction("destroy", &NbtCompoundClass::destroy)
        .build();

void TagToJson_Compound_Helper(ordered_json& res, CompoundTag* nbt);

void TagToJson_List_Helper(ordered_json& res, ListTag* nbt) {
    auto& list = nbt->mList;
    for (auto& tag : list) {
        switch (tag->getId()) {
        case Tag::Type::Byte:
            tag->as<ByteTag>() = 4;
            res.push_back(tag->as<ByteTag>().data);
            break;
        case Tag::Type::Short:
            res.push_back(tag->as<ShortTag>().data);
            break;
        case Tag::Type::Int:
            res.push_back(tag->as<IntTag>().data);
            break;
        case Tag::Type::Int64:
            res.push_back(tag->as<Int64Tag>().data);
            break;
        case Tag::Type::Float:
            res.push_back(tag->as<FloatTag>().data);
            break;
        case Tag::Type::Double:
            res.push_back(tag->as<DoubleTag>().data);
            break;
        case Tag::Type::String:
            res.push_back(tag->as<StringTag>().data);
            break;
        case Tag::Type::ByteArray: {
            auto& bytes = tag->as<ByteArrayTag>().data;
            char  tmpData[1024];
            for (unsigned int i = 0; i < bytes.size(); ++i) {
                tmpData[i] = bytes[i];
            }
            res.push_back(Base64::Encode(tmpData));
            break;
        }
        case Tag::Type::List: {
            ordered_json arrJson = ordered_json::array();
            TagToJson_List_Helper(arrJson, tag->as_ptr<ListTag>());
            res.push_back(arrJson);
            break;
        }
        case Tag::Type::Compound: {
            ordered_json arrObj = ordered_json::object();
            TagToJson_Compound_Helper(arrObj, tag->as_ptr<CompoundTag>());
            res.push_back(arrObj);
            break;
        }
        case Tag::Type::End:
        default:
            res.push_back(nullptr);
            break;
        }
    }
}

void TagToJson_Compound_Helper(ordered_json& res, CompoundTag* nbt) {
    auto& list = nbt->mTags;
    for (auto& [key, tmp] : list) {
        auto& tag = tmp.get();
        switch (tag.getId()) {
        case Tag::Type::Byte:
            res.push_back({key, tag.as<ByteTag>().data});
            break;
        case Tag::Type::Short:
            res.push_back({key, tag.as<ShortTag>().data});
            break;
        case Tag::Type::Int:
            res.push_back({key, tag.as<IntTag>().data});
            break;
        case Tag::Type::Int64:
            res.push_back({key, tag.as<Int64Tag>().data});
            break;
        case Tag::Type::Float:
            res.push_back({key, tag.as<FloatTag>().data});
            break;
        case Tag::Type::Double:
            res.push_back({key, tag.as<DoubleTag>().data});
            break;
        case Tag::Type::String:
            res.push_back({key, tag.as<StringTag>().data});
            break;
        case Tag::Type::ByteArray: {
            auto& bytes = tag.as<ByteArrayTag>().data;
            char  tmpData[1024];
            for (unsigned int i = 0; i < bytes.size(); ++i) {
                tmpData[i] = bytes[i];
            }
            res.push_back(Base64::Encode(tmpData));
            break;
        }
        case Tag::Type::List: {
            ordered_json arrJson = ordered_json::array();
            TagToJson_List_Helper(arrJson, &tag.as<ListTag>());
            res.push_back({key, arrJson});
            break;
        }
        case Tag::Type::Compound: {
            ordered_json arrObj = ordered_json::object();
            TagToJson_Compound_Helper(arrObj, &tag.as<CompoundTag>());
            res.push_back({key, arrObj});
            break;
        }
        case Tag::Type::End:
        default:
            res.push_back({key, nullptr});
            break;
        }
    }
}

std::string TagToJson(Tag* nbt, int formatIndent) {
    std::string result;
    switch (nbt->getId()) {
    case Tag::Type::End:
        result = "";
        break;
    case Tag::Type::Byte:
        result = std::to_string(nbt->as<ByteTag>().data);
        break;
    case Tag::Type::Short:
        result = std::to_string(nbt->as<ShortTag>().data);
        break;
    case Tag::Type::Int:
        result = std::to_string(nbt->as<IntTag>().data);
        break;
    case Tag::Type::Int64:
        result = std::to_string(nbt->as<Int64Tag>().data);
        break;
    case Tag::Type::Float:
        result = std::to_string(nbt->as<FloatTag>().data);
        break;
    case Tag::Type::Double:
        result = std::to_string(nbt->as<DoubleTag>().data);
        break;
    case Tag::Type::String:
        result = nbt->as<StringTag>().data;
        break;
    case Tag::Type::ByteArray: {
        auto&       bytes = nbt->as<ByteArrayTag>().data;
        std::string tmpData;
        for (uchar data : bytes) {
            tmpData.push_back(data);
        }
        result = Base64::Encode(tmpData);
        break;
    }
    case Tag::Type::List: {
        ordered_json jsonRes = ordered_json::array();
        TagToJson_List_Helper(jsonRes, nbt->as_ptr<ListTag>());
        result = jsonRes.dump(formatIndent);
        break;
    }
    case Tag::Type::Compound: {
        ordered_json jsonRes = ordered_json::object();
        TagToJson_Compound_Helper(jsonRes, nbt->as_ptr<CompoundTag>());
        result = jsonRes.dump(formatIndent);
        break;
    }
    default:
        result = "";
        break;
    }
    return result;
}

//////////////////// Classes NbtByte ////////////////////

NbtByteClass::NbtByteClass(const Local<Object>& scriptObj, std::unique_ptr<ByteTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtByteClass::NbtByteClass(std::unique_ptr<ByteTag> p) : ScriptClass(ScriptClass::ConstructFromCpp<NbtByteClass>{}) {
    this->nbt = std::move(p);
}

NbtByteClass* NbtByteClass::constructor(const Arguments& args) {
    try {
        auto tag = ByteTag((char)args[0].toInt());
        return new NbtByteClass(args.thiz(), std::move(std::make_unique<ByteTag>(tag)));
    }
    CATCH_C("Fail in Create ByteTag!");
}

ByteTag* NbtByteClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtByteClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtByteClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtByteClass::pack(ByteTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<ByteTag> nbt(tag);
            auto*                    nbtObj = new NbtByteClass(std::move(nbt));
            nbtObj->canDelete               = false;
            return nbtObj->getScriptObject();
        } else return (new NbtByteClass(std::make_unique<ByteTag>(tag->as<ByteTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtByte!");
}

Local<Value> NbtByteClass::pack(std::unique_ptr<ByteTag> tag) {
    try {
        return (new NbtByteClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtByte!");
}

Local<Value> NbtByteClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Byte); }

Local<Value> NbtByteClass::get(const Arguments& args) {
    try {
        return Number::newNumber(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtByteClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtByteClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        nbt->data = (char)args[0].toInt();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtInt ////////////////////

NbtIntClass::NbtIntClass(const Local<Object>& scriptObj, std::unique_ptr<IntTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtIntClass::NbtIntClass(std::unique_ptr<IntTag> p) : ScriptClass(ScriptClass::ConstructFromCpp<NbtIntClass>{}) {
    this->nbt = std::move(p);
}

NbtIntClass* NbtIntClass::constructor(const Arguments& args) {
    try {
        auto tag = IntTag(args[0].toInt());
        return new NbtIntClass(args.thiz(), std::make_unique<IntTag>(tag));
    }
    CATCH_C("Fail in Create IntTag!");
}

IntTag* NbtIntClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtIntClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtIntClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtIntClass::pack(IntTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<IntTag> nbt(tag);
            auto*                   nbtObj = new NbtIntClass(std::move(nbt));
            nbtObj->canDelete              = false;
            return nbtObj->getScriptObject();
        } else return (new NbtIntClass(std::make_unique<IntTag>(tag->as<IntTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtInt!");
}

Local<Value> NbtIntClass::pack(std::unique_ptr<IntTag> tag) {
    try {
        return (new NbtIntClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtInt!");
}

Local<Value> NbtIntClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Int); }

Local<Value> NbtIntClass::get(const Arguments& args) {
    try {
        return Number::newNumber(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtIntClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtIntClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1)

    try {
        nbt->data = args[0].toInt();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtShort ////////////////////

NbtShortClass::NbtShortClass(const Local<Object>& scriptObj, std::unique_ptr<ShortTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtShortClass::NbtShortClass(std::unique_ptr<ShortTag> p)
: ScriptClass(ScriptClass::ConstructFromCpp<NbtShortClass>{}) {
    this->nbt = std::move(p);
}

NbtShortClass* NbtShortClass::constructor(const Arguments& args) {
    try {
        auto tag = ShortTag(args[0].toInt());
        return new NbtShortClass(args.thiz(), std::make_unique<ShortTag>(tag));
    }
    CATCH_C("Fail in Create ShortTag!");
}

ShortTag* NbtShortClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtShortClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtShortClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtShortClass::pack(ShortTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<ShortTag> nbt(tag);
            auto*                     nbtObj = new NbtShortClass(std::move(nbt));
            nbtObj->canDelete                = false;
            return nbtObj->getScriptObject();
        } else return (new NbtShortClass(std::make_unique<ShortTag>(tag->as<ShortTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtShort!");
}

Local<Value> NbtShortClass::pack(std::unique_ptr<ShortTag> tag) {
    try {
        return (new NbtShortClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtShort!");
}

Local<Value> NbtShortClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Short); }

Local<Value> NbtShortClass::get(const Arguments& args) {
    try {
        return Number::newNumber(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtShortClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtShortClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        nbt->data = args[0].toInt();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtLong ////////////////////

NbtLongClass::NbtLongClass(const Local<Object>& scriptObj, std::unique_ptr<Int64Tag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtLongClass::NbtLongClass(std::unique_ptr<Int64Tag> p) : ScriptClass(ScriptClass::ConstructFromCpp<NbtLongClass>{}) {
    this->nbt = std::move(p);
}

NbtLongClass* NbtLongClass::constructor(const Arguments& args) {
    try {
        auto tag = Int64Tag(args[0].asNumber().toInt64());
        return new NbtLongClass(args.thiz(), std::make_unique<Int64Tag>(tag));
    }
    CATCH_C("Fail in Create LongTag!");
}

Int64Tag* NbtLongClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtLongClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtLongClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtLongClass::pack(Int64Tag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<Int64Tag> nbt(tag);
            auto*                     nbtObj = new NbtLongClass(std::move(nbt));
            nbtObj->canDelete                = false;
            return nbtObj->getScriptObject();
        } else return (new NbtLongClass(std::make_unique<Int64Tag>(tag->as<Int64Tag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtLong!");
}

Local<Value> NbtLongClass::pack(std::unique_ptr<Int64Tag> tag) {
    try {
        return (new NbtLongClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtLong!");
}

Local<Value> NbtLongClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Int64); }

Local<Value> NbtLongClass::get(const Arguments& args) {
    try {
        return Number::newNumber(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtLongClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtLongClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        nbt->data = args[0].asNumber().toInt64();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtFloat ////////////////////

NbtFloatClass::NbtFloatClass(const Local<Object>& scriptObj, std::unique_ptr<FloatTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtFloatClass::NbtFloatClass(std::unique_ptr<FloatTag> p)
: ScriptClass(ScriptClass::ConstructFromCpp<NbtFloatClass>{}) {
    this->nbt = std::move(p);
}

NbtFloatClass* NbtFloatClass::constructor(const Arguments& args) {
    try {
        auto tag = FloatTag(args[0].asNumber().toFloat());
        return new NbtFloatClass(args.thiz(), std::make_unique<FloatTag>(tag));
    }
    CATCH_C("Fail in Create FloatTag!");
}

FloatTag* NbtFloatClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtFloatClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtFloatClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtFloatClass::pack(FloatTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<FloatTag> nbt(tag);
            auto*                     nbtObj = new NbtFloatClass(std::move(nbt));
            nbtObj->canDelete                = false;
            return nbtObj->getScriptObject();
        } else return (new NbtFloatClass(std::make_unique<FloatTag>(tag->as<FloatTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtFloat!");
}

Local<Value> NbtFloatClass::pack(std::unique_ptr<FloatTag> tag) {
    try {
        return (new NbtFloatClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtFloat!");
}

Local<Value> NbtFloatClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Float); }

Local<Value> NbtFloatClass::get(const Arguments& args) {
    try {
        return Number::newNumber(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtFloatClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtFloatClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        nbt->data = args[0].asNumber().toFloat();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtDouble ////////////////////

NbtDoubleClass::NbtDoubleClass(const Local<Object>& scriptObj, std::unique_ptr<DoubleTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtDoubleClass::NbtDoubleClass(std::unique_ptr<DoubleTag> p)
: ScriptClass(ScriptClass::ConstructFromCpp<NbtDoubleClass>{}) {
    this->nbt = std::move(p);
}

NbtDoubleClass* NbtDoubleClass::constructor(const Arguments& args) {
    try {
        auto tag = DoubleTag(args[0].asNumber().toDouble());
        return new NbtDoubleClass(args.thiz(), std::make_unique<DoubleTag>(tag));
    }
    CATCH_C("Fail in Create DoubleTag!");
}

DoubleTag* NbtDoubleClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtDoubleClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtDoubleClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtDoubleClass::pack(DoubleTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<DoubleTag> nbt(tag);
            auto*                      nbtObj = new NbtDoubleClass(std::move(nbt));
            nbtObj->canDelete                 = false;
            return nbtObj->getScriptObject();
        } else return (new NbtDoubleClass(std::make_unique<DoubleTag>(tag->as<DoubleTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtDouble!");
}

Local<Value> NbtDoubleClass::pack(std::unique_ptr<DoubleTag> tag) {
    try {
        return (new NbtDoubleClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtDouble!");
}

Local<Value> NbtDoubleClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Double); }

Local<Value> NbtDoubleClass::get(const Arguments& args) {
    try {
        return Number::newNumber(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtDoubleClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtDoubleClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        nbt->data = args[0].asNumber().toDouble();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtString ////////////////////

NbtStringClass::NbtStringClass(const Local<Object>& scriptObj, std::unique_ptr<StringTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtStringClass::NbtStringClass(std::unique_ptr<StringTag> p)
: ScriptClass(ScriptClass::ConstructFromCpp<NbtStringClass>{}) {
    this->nbt = std::move(p);
}

NbtStringClass* NbtStringClass::constructor(const Arguments& args) {
    try {
        auto tag = StringTag(args[0].toStr());
        return new NbtStringClass(args.thiz(), std::make_unique<StringTag>(tag));
    }
    CATCH_C("Fail in Create StringTag!");
}

StringTag* NbtStringClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtStringClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtStringClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtStringClass::pack(StringTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<StringTag> nbt(tag);
            auto*                      nbtObj = new NbtStringClass(std::move(nbt));
            nbtObj->canDelete                 = false;
            return nbtObj->getScriptObject();
        } else return (new NbtStringClass(std::make_unique<StringTag>(tag->as<StringTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtString!");
}

Local<Value> NbtStringClass::pack(std::unique_ptr<StringTag> tag) {
    try {
        return (new NbtStringClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtString!");
}

Local<Value> NbtStringClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::String); }

Local<Value> NbtStringClass::get(const Arguments& args) {
    try {
        return String::newString(nbt->data);
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtStringClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtStringClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        nbt->data = args[0].toStr();
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtByteArray ////////////////////

NbtByteArrayClass::NbtByteArrayClass(const Local<Object>& scriptObj, std::unique_ptr<ByteArrayTag> p)
: ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtByteArrayClass::NbtByteArrayClass(std::unique_ptr<ByteArrayTag> p)
: ScriptClass(ScriptClass::ConstructFromCpp<NbtByteArrayClass>{}) {
    this->nbt = std::move(p);
}

NbtByteArrayClass* NbtByteArrayClass::constructor(const Arguments& args) {
    try {
        auto buf = args[0].asByteBuffer();

        std::unique_ptr<ByteArrayTag> arrayTag = std::make_unique<ByteArrayTag>(ByteArrayTag());
        for (char c : buf.describeUtf8()) {
            arrayTag->data.push_back(c);
        }
        return new NbtByteArrayClass(args.thiz(), std::move(arrayTag));
    }
    CATCH_C("Fail in Create ByteArrayTag!");
}

ByteArrayTag* NbtByteArrayClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtByteArrayClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtByteArrayClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtByteArrayClass::pack(ByteArrayTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<ByteArrayTag> nbt(tag);
            auto*                         nbtObj = new NbtByteArrayClass(std::move(nbt));
            nbtObj->canDelete                    = false;
            return nbtObj->getScriptObject();
        } else
            return (new NbtByteArrayClass(std::make_unique<ByteArrayTag>(tag->as<ByteArrayTag>())))->getScriptObject();
    }
    CATCH("Fail in construct NbtByteArray!");
}

Local<Value> NbtByteArrayClass::pack(std::unique_ptr<ByteArrayTag> tag) {
    try {
        return (new NbtByteArrayClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtByteArray!");
}

Local<Value> NbtByteArrayClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::ByteArray); }

Local<Value> NbtByteArrayClass::get(const Arguments& args) {
    try {
        auto& data = nbt->data;
        char  buf[1024];
        for (unsigned int i = 0; i < data.size(); ++i) {
            buf[i] = data[i];
        }
        return ByteBuffer::newByteBuffer(buf, data.size());
    }
    CATCH("Fail in NbtValueGet!")
}

Local<Value> NbtByteArrayClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtByteArrayClass::set(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        Local<ByteBuffer> buf = args[0].asByteBuffer();
        for (char c : buf.describeUtf8()) {
            nbt->data.push_back(c);
        }
        return Boolean::newBoolean(true);
    }
    CATCH("Fail in NbtValueSet!")
}

//////////////////// Classes NbtList ////////////////////

NbtListClass::NbtListClass(const Local<Object>& scriptObj, std::unique_ptr<ListTag> p) : ScriptClass(scriptObj) {
    this->nbt = std::move(p);
}

NbtListClass::NbtListClass(std::unique_ptr<ListTag> p) : ScriptClass(ScriptClass::ConstructFromCpp<NbtListClass>{}) {
    this->nbt = std::move(p);
}

////////////////// Helper //////////////////
void NbtListClassAddHelper(ListTag* tag, Local<Array>& arr) {
    if (arr.size() > 0) {
        Local<Value> t = arr.get(0);
        if (IsInstanceOf<NbtByteClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtByteClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtShortClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtShortClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtIntClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtIntClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtLongClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtLongClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtFloatClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtFloatClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtDoubleClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtDoubleClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtStringClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtStringClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtByteArrayClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtByteArrayClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtListClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtListClass::extract(arr.get(i)));
        else if (IsInstanceOf<NbtCompoundClass>(t))
            for (int i = 0; i < arr.size(); ++i) tag->add(*NbtCompoundClass::extract(arr.get(i)));
        else if (t.isArray()) {
            for (int i = 0; i < arr.size(); ++i) {
                auto arrTag = ListTag();
                auto data   = arr.get(i).asArray();
                NbtListClassAddHelper(&arrTag, data);
                tag->add(std::move(arrTag));
            }
        } else if (t.isObject()) {
            for (int i = 0; i < arr.size(); ++i) {
                auto objTag = CompoundTag();
                auto data   = arr.get(i).asObject();
                NbtCompoundClassAddHelper(&objTag, data);
                tag->add(std::move(objTag));
            }
        } else {
            throw script::Exception("Wrong Type of data to set into NBT List!");
        }
    }
}

NbtListClass* NbtListClass::constructor(const Arguments& args) {
    try {
        auto tag = ListTag();

        if (args.size() >= 1 && args[0].isArray()) {
            auto arr = args[0].asArray();
            NbtListClassAddHelper(&tag, arr);
        }

        return new NbtListClass(args.thiz(), std::make_unique<ListTag>(tag));
    }
    CATCH_C("Fail in Create ListTag!");
}

ListTag* NbtListClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtListClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtListClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtListClass::pack(ListTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<ListTag> nbt(tag);
            auto*                    nbtObj = new NbtListClass(std::move(nbt));
            nbtObj->canDelete               = false;
            return nbtObj->getScriptObject();
        } else return (new NbtListClass(std::unique_ptr<ListTag>(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtList!");
}

Local<Value> NbtListClass::pack(std::unique_ptr<ListTag> tag) {
    try {
        return (new NbtListClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtList!");
}

Local<Value> NbtListClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::List); }

Local<Value> NbtListClass::getSize(const Arguments& args) {
    try {
        return Number::newNumber((int)nbt->size());
    }
    CATCH("Fail in NBT GetSize!");
}

Local<Value> NbtListClass::getTypeOf(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            return Local<Value>();
        }

        return Number::newNumber(int(list[index].getId()));
    }
    CATCH("Fail in NBT GetTypeOf!");
}

Local<Value> NbtListClass::setEnd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::End) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<EndTag>();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetEnd!");
}

Local<Value> NbtListClass::setByte(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::Byte) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<ByteTag>()->data = args[1].toInt();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetByte!");
}

Local<Value> NbtListClass::setInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::Int) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<IntTag>()->data = args[1].toInt();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetInt!");
}

Local<Value> NbtListClass::setShort(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::Short) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<ShortTag>()->data = args[1].toInt();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetShort!");
}

Local<Value> NbtListClass::setLong(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::Int64) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<Int64Tag>()->data = args[1].asNumber().toInt64();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetLong!");
}

Local<Value> NbtListClass::setFloat(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::Float) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<FloatTag>()->data = args[1].asNumber().toFloat();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetFloat!");
}

Local<Value> NbtListClass::setDouble(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::Double) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<DoubleTag>()->data = args[1].asNumber().toDouble();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetDouble!");
}

Local<Value> NbtListClass::setString(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::String) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            list[index].as_ptr<StringTag>()->data = args[1].toStr();
        }

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetString!");
}

Local<Value> NbtListClass::setByteArray(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);
    CHECK_ARG_TYPE(args[1], ValueKind::kByteBuffer);

    try {
        auto list  = nbt.get();
        auto index = args[0].toInt();

        if (index >= list->size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
        } else if (list[0].getId() != Tag::Type::ByteArray) {
            LOG_ERROR_WITH_SCRIPT_INFO("Set wrong type of element into NBT List!");
        } else {
            auto data = args[1].asByteBuffer();
            for (char c : data.describeUtf8()) {
                list[index].as_ptr<ByteArrayTag>()->data.push_back(c);
            }
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetByteArray!");
}

Local<Value> NbtListClass::setTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto& list  = nbt->mList;
        auto  index = args[0].toInt();

        if (index >= list.size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
            return Local<Value>();
        }

        if (IsInstanceOf<NbtByteClass>(args[1])) {
            list[index] = std::unique_ptr<ByteTag>(NbtByteClass::extract(args[1]));
        } else if (IsInstanceOf<NbtShortClass>(args[1])) {
            list[index] = std::unique_ptr<ShortTag>(NbtShortClass::extract(args[1]));
        } else if (IsInstanceOf<NbtIntClass>(args[1])) {
            list[index] = std::unique_ptr<IntTag>(NbtIntClass::extract(args[1]));
        } else if (IsInstanceOf<NbtLongClass>(args[1])) {
            list[index] = std::unique_ptr<Int64Tag>(NbtLongClass::extract(args[1]));
        } else if (IsInstanceOf<NbtFloatClass>(args[1])) {
            list[index] = std::unique_ptr<FloatTag>(NbtFloatClass::extract(args[1]));
        } else if (IsInstanceOf<NbtDoubleClass>(args[1])) {
            list[index] = std::unique_ptr<DoubleTag>(NbtDoubleClass::extract(args[1]));
        } else if (IsInstanceOf<NbtStringClass>(args[1])) {
            list[index] = std::unique_ptr<StringTag>(NbtStringClass::extract(args[1]));
        } else if (IsInstanceOf<NbtByteArrayClass>(args[1])) {
            list[index] = std::unique_ptr<ByteArrayTag>(NbtByteArrayClass::extract(args[1]));
        } else if (IsInstanceOf<NbtListClass>(args[1])) {
            list[index] = std::unique_ptr<ListTag>(NbtListClass::extract(args[1]));
        } else if (IsInstanceOf<NbtCompoundClass>(args[1])) {
            list[index] = std::unique_ptr<CompoundTag>(NbtCompoundClass::extract(args[1]));
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO("Unknown type! Cannot set Tag into List");
            return Local<Value>();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetTag!");
}

Local<Value> NbtListClass::addTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);

    try {
        if (IsInstanceOf<NbtByteClass>(args[0])) {
            nbt->add(*NbtByteClass::extract(args[0]));
        } else if (IsInstanceOf<NbtShortClass>(args[0])) {
            nbt->add(*NbtShortClass::extract(args[0]));
        } else if (IsInstanceOf<NbtIntClass>(args[0])) {
            nbt->add(*NbtIntClass::extract(args[0]));
        } else if (IsInstanceOf<NbtLongClass>(args[0])) {
            nbt->add(*NbtLongClass::extract(args[0]));
        } else if (IsInstanceOf<NbtFloatClass>(args[0])) {
            nbt->add(*NbtFloatClass::extract(args[0]));
        } else if (IsInstanceOf<NbtDoubleClass>(args[0])) {
            nbt->add(*NbtDoubleClass::extract(args[0]));
        } else if (IsInstanceOf<NbtStringClass>(args[0])) {
            nbt->add(*NbtStringClass::extract(args[0]));
        } else if (IsInstanceOf<NbtByteArrayClass>(args[0])) {
            nbt->add(*NbtByteArrayClass::extract(args[0]));
        } else if (IsInstanceOf<NbtListClass>(args[0])) {
            nbt->add(*NbtListClass::extract(args[0]));
        } else if (IsInstanceOf<NbtCompoundClass>(args[0])) {
            nbt->add(*NbtCompoundClass::extract(args[0]));
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO("Unknown type! Cannot add Tag into List");
            return Local<Value>();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT AddTag!");
}

Local<Value> NbtListClass::removeTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto& list  = nbt->mList;
        auto  index = args[0].toInt();

        if (index >= list.size() || index < 0) {
            LOG_ERROR_WITH_SCRIPT_INFO("Bad Index of NBT List!");
            return Local<Value>();
        }

        list.erase(list.begin() + index); //===== delete?
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetTag!");
}

Local<Value> NbtListClass::getData(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto index = args[0].toInt();

        if (index >= nbt->size() || index < 0) {
            return Local<Value>();
        }

        return Tag2Value(nbt->at(index).get());
    }
    CATCH("Fail in NBTgetData!")
}

Local<Value> NbtListClass::getTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto index = args[0].toInt();

        if (index >= nbt->size() || index < 0) {
            return Local<Value>();
        }

        Local<Value> res;
        // lse::getSelfPluginInstance().getLogger().info(
        //     "getListTag Type {}",
        //     magic_enum::enum_name(nbt->at(index)->getId())
        // );
        switch (nbt->at(index)->getId()) {
        case Tag::Type::Byte:
            res = NbtByteClass::pack(nbt->at(index)->as_ptr<ByteTag>(),
                                     true); // share ptr
            break;
        case Tag::Type::Short:
            res = NbtShortClass::pack(nbt->at(index)->as_ptr<ShortTag>(),
                                      true); // share ptr
            break;
        case Tag::Type::Int:
            res = NbtIntClass::pack(nbt->at(index)->as_ptr<IntTag>(),
                                    true); // share ptr
            break;
        case Tag::Type::Int64:
            res = NbtLongClass::pack(nbt->at(index)->as_ptr<Int64Tag>(),
                                     true); // share ptr
            break;
        case Tag::Type::Float:
            res = NbtFloatClass::pack(nbt->at(index)->as_ptr<FloatTag>(),
                                      true); // share ptr
            break;
        case Tag::Type::Double:
            res = NbtDoubleClass::pack(nbt->at(index)->as_ptr<DoubleTag>(), true);
            // share ptr
            break;
        case Tag::Type::String:
            res = NbtStringClass::pack(nbt->at(index)->as_ptr<StringTag>(),
                                       true); // share ptr
            break;
        case Tag::Type::ByteArray:
            res = NbtByteArrayClass::pack(nbt->at(index)->as_ptr<ByteArrayTag>(),
                                          true); // share ptr
            break;
        case Tag::Type::List:
            res = NbtListClass::pack(nbt->at(index)->as_ptr<ListTag>(), true); // share ptr
            break;
        case Tag::Type::Compound:
            res = NbtCompoundClass::pack(nbt->at(index)->as_ptr<CompoundTag>(),
                                         true); // share ptr
            break;
        case Tag::Type::End:
        default:
            res = Local<Value>();
            break;
        }
        return res;
    }
    CATCH("Fail in NBT GetTag!");
}

Local<Value> NbtListClass::toArray(const Arguments& args) {
    try {
        auto&        list = nbt->mList;
        Local<Array> arr  = Array::newArray();

        for (auto& tag : list) {
            arr.add(Tag2Value(tag.get(), true));
        }
        return arr;
    }
    CATCH("Fail in NBTtoArray!");
}

Local<Value> NbtListClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

//////////////////// Classes NbtCompound ////////////////////

NbtCompoundClass::NbtCompoundClass(const Local<Object>& scriptObj, std::unique_ptr<CompoundTag> p)
: ScriptClass(scriptObj) {
    nbt = std::move(p);
}

NbtCompoundClass::NbtCompoundClass(std::unique_ptr<CompoundTag> p)
: ScriptClass(ScriptClass::ConstructFromCpp<NbtCompoundClass>{}) {
    nbt = std::move(p);
}

////////////////// Helper //////////////////
void NbtCompoundClassAddHelper(CompoundTag* tag, Local<Object>& obj) {
    auto keys = obj.getKeyNames();
    if (keys.size() > 0) {
        for (int i = 0; i < keys.size(); ++i) {
            Local<Value> t = obj.get(keys[i]);
            if (IsInstanceOf<NbtByteClass>(t)) tag->at(keys[i]) = *NbtByteClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtShortClass>(t)) tag->at(keys[i]) = *NbtShortClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtIntClass>(t)) tag->at(keys[i]) = *NbtIntClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtLongClass>(t)) tag->at(keys[i]) = *NbtLongClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtFloatClass>(t)) tag->at(keys[i]) = *NbtFloatClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtDoubleClass>(t)) tag->at(keys[i]) = *NbtDoubleClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtStringClass>(t)) tag->at(keys[i]) = *NbtStringClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtByteArrayClass>(t))
                tag->at(keys[i]) = *NbtByteArrayClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtListClass>(t)) tag->at(keys[i]) = *NbtListClass::extract(obj.get(keys[i]));
            else if (IsInstanceOf<NbtCompoundClass>(t)) tag->at(keys[i]) = *NbtCompoundClass::extract(obj.get(keys[i]));
            else if (t.isArray()) {
                auto arrTag = ListTag();
                auto data   = obj.get(keys[i]).asArray();
                NbtListClassAddHelper(&arrTag, data);
                tag->at(keys[i]) = std::move(arrTag);
            } else if (t.isObject()) {
                auto objTag = CompoundTag();
                auto data   = obj.get(keys[i]).asObject();
                NbtCompoundClassAddHelper(&objTag, data);
                tag->at(keys[i]) = std::move(objTag);
            } else {
                LOG_ERROR_WITH_SCRIPT_INFO("Wrong Type of data to set into NBT Compound!");
            }
        }
    }
}

NbtCompoundClass* NbtCompoundClass::constructor(const Arguments& args) {
    try {
        auto tag = CompoundTag();

        if (args.size() >= 1 && args[0].isObject()) {
            auto obj = args[0].asObject();
            NbtCompoundClassAddHelper(&tag, obj);
        }

        return new NbtCompoundClass(args.thiz(), std::make_unique<CompoundTag>(tag));
    }
    CATCH_C("Fail in Create ListTag!");
}

CompoundTag* NbtCompoundClass::extract(Local<Value> v) {
    if (EngineScope::currentEngine()->isInstanceOf<NbtCompoundClass>(v))
        return EngineScope::currentEngine()->getNativeInstance<NbtCompoundClass>(v)->nbt.get();
    else return nullptr;
}

Local<Value> NbtCompoundClass::pack(CompoundTag* tag, bool noDelete) {
    try {
        if (noDelete) // unique_ptr 共享指针 + noDelete
        {
            std::unique_ptr<CompoundTag> nbt(tag);
            auto*                        nbtObj = new NbtCompoundClass(std::move(nbt));
            nbtObj->canDelete                   = false;
            return nbtObj->getScriptObject();
        } else return (new NbtCompoundClass(std::unique_ptr<CompoundTag>(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtCompound!");
}

Local<Value> NbtCompoundClass::pack(std::unique_ptr<CompoundTag> tag) {
    try {
        return (new NbtCompoundClass(std::move(tag)))->getScriptObject();
    }
    CATCH("Fail in construct NbtCompound!");
}

Local<Value> NbtCompoundClass::getType(const Arguments& args) { return Number::newNumber((int)Tag::Type::Compound); }

Local<Value> NbtCompoundClass::getKeys(const Arguments& args) {
    try {
        Local<Array> arr  = Array::newArray();
        auto&        list = nbt->mTags;
        for (auto& [k, v] : list) {
            arr.add(String::newString(k));
        }

        return arr;
    }
    CATCH("Fail in NBT GetKeys!");
}

Local<Value> NbtCompoundClass::getTypeOf(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto& list = nbt->mTags;
        auto  key  = args[0].toStr();

        return Number::newNumber(int(list.at(key).getId()));
    } catch (const std::out_of_range& e) {
        return Local<Value>();
    }
    CATCH("Fail in NBT GetTypeOf!");
}

Local<Value> NbtCompoundClass::setEnd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto key     = args[0].toStr();
        nbt->at(key) = ByteTag(0);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetEnd!");
}

Local<Value> NbtCompoundClass::setByte(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto key     = args[0].toStr();
        auto data    = char(args[1].toInt());
        nbt->at(key) = ByteTag(data);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetByte!");
}

Local<Value> NbtCompoundClass::setInt(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto key     = args[0].toStr();
        auto data    = int(args[1].toInt());
        nbt->at(key) = IntTag(data);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetInt!");
}

Local<Value> NbtCompoundClass::setShort(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto key     = args[0].toStr();
        auto data    = short(args[1].toInt());
        nbt->at(key) = ShortTag(data);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetShort!");
}

Local<Value> NbtCompoundClass::setLong(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto key     = args[0].toStr();
        auto data    = args[1].asNumber().toInt64();
        nbt->at(key) = Int64Tag(data);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetLong!");
}

Local<Value> NbtCompoundClass::setFloat(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto key     = args[0].toStr();
        auto data    = args[1].asNumber().toFloat();
        nbt->at(key) = FloatTag(data);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetFloat!");
}

Local<Value> NbtCompoundClass::setDouble(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kNumber);

    try {
        auto& list = nbt;
        auto  key  = args[0].toStr();
        auto  data = args[1].asNumber().toDouble();

        list->at(key) = data;
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetDouble!");
}

Local<Value> NbtCompoundClass::setString(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kString);

    try {
        auto key     = args[0].toStr();
        auto data    = args[1].toStr();
        nbt->at(key) = StringTag(data);

        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetString!");
}

Local<Value> NbtCompoundClass::setByteArray(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kByteBuffer);

    try {
        auto         key  = args[0].toStr();
        auto         data = args[1].asByteBuffer();
        ByteArrayTag baTag;
        for (char c : data.describeUtf8()) {
            baTag.data.push_back(c);
        }
        nbt->at(key) = baTag;
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetString!");
}

Local<Value> NbtCompoundClass::setTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto key = args[0].toStr();

        if (IsInstanceOf<NbtByteClass>(args[1])) {
            nbt->at(key) = *NbtByteClass::extract(args[1]);
        } else if (IsInstanceOf<NbtShortClass>(args[1])) {
            nbt->at(key) = *NbtShortClass::extract(args[1]);
        } else if (IsInstanceOf<NbtIntClass>(args[1])) {
            nbt->at(key) = *NbtIntClass::extract(args[1]);
        } else if (IsInstanceOf<NbtLongClass>(args[1])) {
            nbt->at(key) = *NbtLongClass::extract(args[1]);
        } else if (IsInstanceOf<NbtFloatClass>(args[1])) {
            nbt->at(key) = *NbtFloatClass::extract(args[1]);
        } else if (IsInstanceOf<NbtDoubleClass>(args[1])) {
            nbt->at(key) = *NbtDoubleClass::extract(args[1]);
        } else if (IsInstanceOf<NbtStringClass>(args[1])) {
            nbt->at(key) = *NbtStringClass::extract(args[1]);
        } else if (IsInstanceOf<NbtByteArrayClass>(args[1])) {
            nbt->at(key) = *NbtByteArrayClass::extract(args[1]);
        } else if (IsInstanceOf<NbtListClass>(args[1])) {
            nbt->at(key) = *NbtListClass::extract(args[1]);
        } else if (IsInstanceOf<NbtCompoundClass>(args[1])) {
            nbt->at(key) = *NbtCompoundClass::extract(args[1]);
        } else {
            LOG_ERROR_WITH_SCRIPT_INFO("Unknown type! Cannot set Tag into Compound");
            return Local<Value>();
        }
        return this->getScriptObject();
    }
    CATCH("Fail in NBT SetTag!");
}

Local<Value> NbtCompoundClass::removeTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto& list = nbt->mTags;
        auto  key  = args[0].toStr();

        list.erase(key);
        return this->getScriptObject();
    } catch (const std::out_of_range& e) {
        LOG_ERROR_WITH_SCRIPT_INFO("Key no found in NBT Compound!");
        return Local<Value>();
    }
    CATCH("Fail in NBT RemoveTag!");
}

Local<Value> NbtCompoundClass::getData(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto key = args[0].toStr();

        return Tag2Value(nbt->at(key).get().as_ptr<Tag>());
    } catch (const std::out_of_range& e) {
        return Local<Value>();
    }
    CATCH("Fail in NBT GetData!")
}

Local<Value> NbtCompoundClass::getTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto key = args[0].toStr();

        Local<Value> res;
        // lse::getSelfPluginInstance().getLogger().info(
        //     "getCompoundTag Type {}",
        //     magic_enum::enum_name(nbt->at(key).getId())
        // );
        switch (nbt->at(key).getId()) {
        case Tag::Type::Byte:
            res = NbtByteClass::pack(nbt->at(key).get<ByteTag>().as_ptr<ByteTag>(),
                                     true); // share ptr
            break;
        case Tag::Type::Short:
            res = NbtShortClass::pack(nbt->at(key).get<ShortTag>().as_ptr<ShortTag>(),
                                      true); // share ptr
            break;
        case Tag::Type::Int:
            res = NbtIntClass::pack(nbt->at(key).get<IntTag>().as_ptr<IntTag>(),
                                    true); // share ptr
            break;
        case Tag::Type::Int64:
            res = NbtLongClass::pack(nbt->at(key).get<Int64Tag>().as_ptr<Int64Tag>(),
                                     true); // share ptr
            break;
        case Tag::Type::Float:
            res = NbtFloatClass::pack(nbt->at(key).get<FloatTag>().as_ptr<FloatTag>(),
                                      true); // share ptr
            break;
        case Tag::Type::Double:
            res = NbtDoubleClass::pack(nbt->at(key).get<DoubleTag>().as_ptr<DoubleTag>(),
                                       true); // share ptr
            break;
        case Tag::Type::String:
            res = NbtStringClass::pack(nbt->at(key).get<StringTag>().as_ptr<StringTag>(),
                                       true); // share ptr
            break;
        case Tag::Type::ByteArray:
            res = NbtByteArrayClass::pack(nbt->at(key).get<ByteArrayTag>().as_ptr<ByteArrayTag>(),
                                          true); // share ptr
            break;

        case Tag::Type::List:
            res = NbtListClass::pack(nbt->at(key).get<ListTag>().as_ptr<ListTag>(),
                                     true); // share ptr
            break;
        case Tag::Type::Compound:
            res = NbtCompoundClass::pack(nbt->at(key).get<CompoundTag>().as_ptr<CompoundTag>(),
                                         true); // share ptr
            break;
        case Tag::Type::End:
        default:
            res = Local<Value>();
            break;
        }
        return res;
    } catch (const std::out_of_range& e) {
        return Local<Value>();
    }
    CATCH("Fail in NBT GetTag!");
}

Local<Value> NbtCompoundClass::toObject(const Arguments& args) {
    try {
        auto&         comp = nbt->mTags;
        Local<Object> obj  = Object::newObject();

        for (auto& [k, v] : comp) {
            obj.set(k, Tag2Value(v.get().as_ptr<CompoundTag>(), true));
        }
        return obj;
    }
    CATCH("Fail in NBT ToObject!");
}

Local<Value> NbtCompoundClass::toSNBT(const Arguments& args) {
    try {
        int indent = args.size() >= 1 ? args[0].toInt() : -1;
        if (indent == -1) return String::newString(nbt->toSnbt(SnbtFormat::ForceQuote, 0));
        else return String::newString(nbt->toSnbt(SnbtFormat::PartialLineFeed, indent));
    }
    CATCH("Fail in toSNBT!");
}

Local<Value> NbtCompoundClass::toBinaryNBT(const Arguments& args) {
    try {
        auto res = nbt->toBinaryNbt();
        return ByteBuffer::newByteBuffer(res.data(), res.size());
    }
    CATCH("Fail in toBinaryNBT!");
}

Local<Value> NbtCompoundClass::toString(const Arguments& args) {
    if (args.size() >= 1) CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        return String::newString(TagToJson(nbt.get(), args.size() >= 1 ? args[0].toInt() : -1));
    }
    CATCH("Fail in NBTtoJson!");
}

Local<Value> NbtCompoundClass::destroy(const Arguments& args) { return Boolean::newBoolean(true); }

//////////////////// APIs ////////////////////

Local<Value> NbtStatic::newTag(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kNumber);

    try {
        auto type = enum_cast<Tag::Type>(args[0].toInt()).value();

        Local<Value> res;
        switch (type) {
        case Tag::Type::Byte: {
            ByteTag tag(0);
            if (args.size() >= 2 && args[1].isNumber()) {
                tag = ByteTag(args[1].toInt());
            }
            res = NbtByteClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::Short: {
            ShortTag tag(0);
            if (args.size() >= 2 && args[1].isNumber()) {
                tag.data = args[1].toInt();
            }
            res = NbtShortClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::Int: {
            IntTag tag(0);
            if (args.size() >= 2 && args[1].isNumber()) {
                tag.data = args[1].toInt();
            }
            res = NbtIntClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::Int64: {
            Int64Tag tag(0);
            if (args.size() >= 2 && args[1].isNumber()) {
                tag.data = args[1].asNumber().toInt64();
            }
            res = NbtLongClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::Float: {
            FloatTag tag;
            if (args.size() >= 2 && args[1].isNumber()) {
                tag.data = args[1].asNumber().toFloat();
            }
            res = NbtFloatClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::Double: {
            DoubleTag tag;
            if (args.size() >= 2 && args[1].isNumber()) {
                tag.data = args[1].asNumber().toDouble();
            }
            res = NbtDoubleClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::String: {
            StringTag tag;
            if (args.size() >= 2 && args[1].isString()) {
                tag.data = args[1].toStr();
            }
            res = NbtStringClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::ByteArray: {
            ByteArrayTag tag;
            if (args.size() >= 2 && args[1].isByteBuffer()) {
                Local<ByteBuffer> buf = args[1].asByteBuffer();
                for (char data : buf.describeUtf8()) {
                    tag.data.push_back(data);
                }
            }
            res = NbtByteArrayClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::List: {
            ListTag tag;
            if (args.size() >= 2 && args[1].isArray()) {
                Local<Array> arr = args[1].asArray();
                NbtListClassAddHelper(&tag, arr);
            }
            res = NbtListClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::Compound: {
            CompoundTag tag;
            if (args.size() >= 2 && args[1].isObject()) {
                Local<Object> obj = args[1].asObject();
                NbtCompoundClassAddHelper(&tag, obj);
            }
            res = NbtCompoundClass::pack(std::move(&tag));
            break;
        }
        case Tag::Type::End:
        default:
            res = Local<Value>();
            break;
        }
        return res;
    }
    CATCH("Fail in NBT CreateTag!");
}

Local<Value> NbtStatic::parseSNBT(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto tag = CompoundTag::fromSnbt(args[0].toStr());
        if (tag.has_value()) return NbtCompoundClass::pack(std::move(tag->clone()));
        else return Local<Value>();
    }
    CATCH("Fail in parseSNBT!");
}

Local<Value> NbtStatic::parseBinaryNBT(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kByteBuffer);

    try {
        auto data = args[0].asByteBuffer();
        auto tag  = CompoundTag::fromBinaryNbt(std::string_view((char*)data.getRawBytes(), data.byteLength()));
        if (tag.has_value()) return NbtCompoundClass::pack(std::move(tag->clone()));
        else return Local<Value>();
    }
    CATCH("Fail in parseBinaryNBT!");
}

//////////////////// Helper ////////////////////

bool IsNbtClass(Local<Value> value) {
    return IsInstanceOf<NbtByteClass>(value) || IsInstanceOf<NbtShortClass>(value) || IsInstanceOf<NbtIntClass>(value)
        || IsInstanceOf<NbtLongClass>(value) || IsInstanceOf<NbtFloatClass>(value)
        || IsInstanceOf<NbtDoubleClass>(value) || IsInstanceOf<NbtStringClass>(value)
        || IsInstanceOf<NbtByteArrayClass>(value) || IsInstanceOf<NbtListClass>(value)
        || IsInstanceOf<NbtCompoundClass>(value);
}

//////////////////// Tag To Value ////////////////////

Local<Value> Tag2Value_CompoundHelper(CompoundTag* nbt, bool autoExpansion = false);

Local<Value> Tag2Value_ListHelper(ListTag* nbt, bool autoExpansion = false) {
    Local<Array> res = Array::newArray();

    auto& list = nbt->as_ptr<ListTag>()->mList;
    for (auto& tag : list) {
        switch (tag->getId()) {
        case Tag::Type::Byte:
            res.add(Number::newNumber(tag->as_ptr<ByteTag>()->data));
            break;
        case Tag::Type::Short:
            res.add(Number::newNumber(tag->as_ptr<ShortTag>()->data));
            break;
        case Tag::Type::Int:
            res.add(Number::newNumber(tag->as_ptr<IntTag>()->data));
            break;
        case Tag::Type::Int64:
            res.add(Number::newNumber(tag->as_ptr<Int64Tag>()->data));
            break;
        case Tag::Type::Float:
            res.add(Number::newNumber(tag->as_ptr<FloatTag>()->data));
            break;
        case Tag::Type::Double:
            res.add(Number::newNumber(tag->as_ptr<DoubleTag>()->data));
            break;
        case Tag::Type::String:
            res.add(String::newString(tag->as_ptr<StringTag>()->data));
            break;
        case Tag::Type::ByteArray: {
            auto& data = tag->as_ptr<ByteArrayTag>()->data;
            char  buf[1024];
            for (unsigned int i = 0; i < data.size(); ++i) {
                buf[i] = data[i];
            }
            res.add(ByteBuffer::newByteBuffer(buf, data.size()));
            break;
        }
        case Tag::Type::List:
            if (!autoExpansion) res.add(NbtListClass::pack(tag->as_ptr<ListTag>()));
            else res.add(Tag2Value_ListHelper(tag->as_ptr<ListTag>(), autoExpansion));
            break;
        case Tag::Type::Compound:
            if (!autoExpansion) res.add(NbtCompoundClass::pack(tag->as_ptr<CompoundTag>()));
            else res.add(Tag2Value_CompoundHelper(tag->as_ptr<CompoundTag>(), autoExpansion));
        case Tag::Type::End:
        default:
            res.add(Local<Value>());
            break;
        }
    }
    return res;
}

Local<Value> Tag2Value_CompoundHelper(CompoundTag* nbt, bool autoExpansion) {
    Local<Object> res = Object::newObject();

    auto& list = nbt->as_ptr<CompoundTag>()->mTags;
    for (auto& [key, tag] : list) {
        switch (tag.getId()) {
        case Tag::Type::Byte:
            res.set(key, Number::newNumber(tag.get().as_ptr<ByteTag>()->data));
            break;
        case Tag::Type::Short:
            res.set(key, Number::newNumber(tag.get().as_ptr<ShortTag>()->data));
            break;
        case Tag::Type::Int:
            res.set(key, Number::newNumber(tag.get().as_ptr<IntTag>()->data));
            break;
        case Tag::Type::Int64:
            res.set(key, Number::newNumber(tag.get().as_ptr<Int64Tag>()->data));
            break;
        case Tag::Type::Float:
            res.set(key, Number::newNumber(tag.get().as_ptr<FloatTag>()->data));
            break;
        case Tag::Type::Double:
            res.set(key, Number::newNumber(tag.get().as_ptr<DoubleTag>()->data));
            break;
        case Tag::Type::String:
            res.set(key, String::newString(tag.get().as_ptr<StringTag>()->data));
            break;
        case Tag::Type::ByteArray: {
            auto& data = tag.get().as_ptr<ByteArrayTag>()->data;
            char  buf[1024];
            for (unsigned int i = 0; i < data.size(); ++i) {
                buf[i] = data[i];
            }
            res.set(key, ByteBuffer::newByteBuffer(buf, data.size()));
            break;
        }
        case Tag::Type::List:
            if (!autoExpansion) res.set(key, NbtListClass::pack(tag.get().as_ptr<ListTag>()));
            else res.set(key, Tag2Value_ListHelper(tag.get().as_ptr<ListTag>(), autoExpansion));
            break;
        case Tag::Type::Compound:
            if (!autoExpansion) res.set(key, NbtCompoundClass::pack(tag.get().as_ptr<CompoundTag>()));
            else res.set(key, Tag2Value_CompoundHelper(tag.get().as_ptr<CompoundTag>(), autoExpansion));
        case Tag::Type::End:
        default:
            res.set(key, Local<Value>());
            break;
        }
    }
    return res;
}

Local<Value> Tag2Value(Tag* nbt, bool autoExpansion) {
    Local<Value> value;

    switch (nbt->getId()) {
    case Tag::Type::Byte:
        value = Number::newNumber(nbt->as_ptr<ByteTag>()->data);
        break;
    case Tag::Type::Short:
        value = Number::newNumber(nbt->as_ptr<ShortTag>()->data);
        break;
    case Tag::Type::Int:
        value = Number::newNumber(nbt->as_ptr<IntTag>()->data);
        break;
    case Tag::Type::Int64:
        value = Number::newNumber(nbt->as_ptr<Int64Tag>()->data);
        break;
    case Tag::Type::Float:
        value = Number::newNumber(nbt->as_ptr<FloatTag>()->data);
        break;
    case Tag::Type::Double:
        value = Number::newNumber(nbt->as_ptr<DoubleTag>()->data);
        break;
    case Tag::Type::String:
        value = String::newString(nbt->as_ptr<StringTag>()->data);
        break;
    case Tag::Type::ByteArray: {
        auto& data = nbt->as_ptr<ByteArrayTag>()->data;
        char  buf[1024];
        for (unsigned int i = 0; i < data.size(); ++i) {
            buf[i] = data[i];
        }
        value = ByteBuffer::newByteBuffer(buf, data.size());
        break;
    }
    case Tag::Type::List:
        if (!autoExpansion) value = NbtListClass::pack(nbt->as_ptr<ListTag>(), true);
        else value = Tag2Value_ListHelper(nbt->as_ptr<ListTag>(), autoExpansion);
        break;
    case Tag::Type::Compound:
        if (!autoExpansion) value = NbtCompoundClass::pack(nbt->as_ptr<CompoundTag>(), true);
        else value = Tag2Value_CompoundHelper(nbt->as_ptr<CompoundTag>(), autoExpansion);
        break;
    case Tag::Type::End:
    default:
        value = Local<Value>();
        break;
    }
    return value;
}
