#include "api/APIHelp.h"

#include "api/BaseAPI.h"
#include "api/BlockAPI.h"
#include "api/ContainerAPI.h"
#include "api/DataAPI.h"
#include "api/DatabaseAPI.h"
#include "api/DeviceAPI.h"
#include "api/EntityAPI.h"
#include "api/GuiAPI.h"
#include "api/ItemAPI.h"
#include "api/NbtAPI.h"
#include "api/NetworkAPI.h"
#include "api/PlayerAPI.h"
#include "ll/api/utils/StringUtils.h"
#include "main/Global.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

//////////////////// APIs ////////////////////
template <typename T>
void PrintValue(T& out, Local<Value> v) {
    switch (v.getKind()) {
    case ValueKind::kString:
        out << v.asString().toString();
        break;
    case ValueKind::kNumber:
        if (CheckIsFloat(v)) out << v.asNumber().toDouble();
        else out << v.asNumber().toInt64();
        break;
    case ValueKind::kBoolean:
        out << v.asBoolean().value();
        break;
    case ValueKind::kNull:
        out << "<Null>";
        break;
    case ValueKind::kArray: {
        Local<Array> arr = v.asArray();
        if (arr.size() == 0) out << "[]";
        else {
            static std::vector<Local<Array>> arrStack = {};
            if (std::find(arrStack.begin(), arrStack.end(), arr) == arrStack.end()) {
                arrStack.push_back(arr);
                out << '[';
                PrintValue(out, arr.get(0));
                for (int i = 1; i < arr.size(); ++i) {
                    out << ',';
                    PrintValue(out, arr.get(i));
                }
                out << ']';
                arrStack.pop_back();
            } else {
                out << "<Recursive Array>";
            }
        }
        break;
    }
    case ValueKind::kObject: {
        // 自定义类型也会被识别为Object，优先处理
        // IntPos
        IntPos* intpos = IntPos::extractPos(v);
        if (intpos != nullptr) {
            out << DimId2Name(intpos->dim) << "(" << intpos->x << "," << intpos->y << "," << intpos->z << ")";
            break;
        }

        // FloatPos
        FloatPos* floatpos = FloatPos::extractPos(v);
        if (floatpos != nullptr) {
            out << std::fixed << std::setprecision(2) << DimId2Name(floatpos->dim) << "(" << floatpos->x << ","
                << floatpos->y << "," << floatpos->z << ")";
            break;
        }

        if (IsInstanceOf<BlockClass>(v)) {
            out << "<Block>";
            break;
        }
        if (IsInstanceOf<KVDBClass>(v)) {
            out << "<Database>";
            break;
        }
        if (IsInstanceOf<ConfJsonClass>(v)) {
            out << "<ConfJson>";
            break;
        }
        if (IsInstanceOf<ConfIniClass>(v)) {
            out << "<ConfIni>";
            break;
        }
        if (IsInstanceOf<DeviceClass>(v)) {
            out << "<DeviceInfo>";
            break;
        }
        if (IsInstanceOf<ContainerClass>(v)) {
            out << "<Container>";
            break;
        }
        if (IsInstanceOf<EntityClass>(v)) {
            out << "<Entity>";
            break;
        }
        if (IsInstanceOf<SimpleFormClass>(v)) {
            out << "<SimpleForm>";
            break;
        }
        if (IsInstanceOf<CustomFormClass>(v)) {
            out << "<CustomForm>";
            break;
        }
        if (IsInstanceOf<ItemClass>(v)) {
            out << "<Item>";
            break;
        }
        if (IsInstanceOf<PlayerClass>(v)) {
            out << "<Player>";
            break;
        }
        if (IsNbtClass(v)) {
            out << "<NbtClass>";
            break;
        }
        if (IsInstanceOf<DBSessionClass>(v)) {
            out << "<DBSession>";
        }
        if (IsInstanceOf<DBStmtClass>(v)) {
            out << "<DBStmt>";
        }
        if (IsInstanceOf<HttpServerClass>(v)) {
            out << "<HttpServer>";
        }
        if (IsInstanceOf<HttpRequestClass>(v)) {
            out << "<HttpRequest>";
        }
        if (IsInstanceOf<HttpResponseClass>(v)) {
            out << "<HttpResponse>";
        }

        Local<Object>            obj  = v.asObject();
        std::vector<std::string> keys = obj.getKeyNames();
        if (keys.empty()) out << "{}";
        else {
            static std::vector<Local<Object>> objStack = {};
            if (std::find(objStack.begin(), objStack.end(), obj) == objStack.end()) {
                objStack.push_back(obj);
                out << '{';
                out << keys[0] + ":";
                PrintValue(out, obj.get(keys[0]));
                for (int i = 1; i < keys.size(); ++i) {
                    out << "," + keys[i] + ":";
                    PrintValue(out, obj.get(keys[i]));
                }
                out << '}';
                objStack.pop_back();
            } else {
                out << "<Recursive Object>";
            }
        }
        break;
    }
    case ValueKind::kByteBuffer: {
        Local<ByteBuffer> bytes = v.asByteBuffer();
        out << ((const char*)bytes.getRawBytes(), bytes.byteLength());
        break;
    }
    case ValueKind::kFunction: {
        out << "<Function>";
        break;
    }
    default: {
        out << "<Unknown>";
        break;
    }
    }
}

template void PrintValue(std::ostream& out, Local<Value> v);

std::string ValueToString(Local<Value> v) {
    std::ostringstream sout;
    PrintValue(sout, v);
    return sout.str();
}

bool CheckIsFloat(const Local<Value>& num) {
    try {
        return fabs(num.asNumber().toDouble() - num.asNumber().toInt64()) >= 1e-15;
    } catch (...) {
        return false;
    }
}

///////////////////// Json To Value /////////////////////

Local<Value> BigInteger_Helper(ordered_json& i) {
    if (i.is_number_integer()) {
        if (i.is_number_unsigned()) {
            auto ui = i.get<uint64_t>();
            if (ui <= LLONG_MAX) return Number::newNumber((int64_t)ui);
            return Number::newNumber((double)ui);
        }
        return Number::newNumber(i.get<int64_t>());
    }
    return Local<Value>();
}

void JsonToValue_Helper(Local<Array>& res, ordered_json& j);

void JsonToValue_Helper(Local<Object>& res, const string& key, ordered_json& j) {
    switch (j.type()) {
    case ordered_json::value_t::string:
        res.set(key, String::newString(j.get<string>()));
        break;
    case ordered_json::value_t::number_integer:
    case ordered_json::value_t::number_unsigned:
        res.set(key, BigInteger_Helper(j));
        break;
    case ordered_json::value_t::number_float:
        res.set(key, Number::newNumber(j.get<double>()));
        break;
    case ordered_json::value_t::boolean:
        res.set(key, Boolean::newBoolean(j.get<bool>()));
        break;
    case ordered_json::value_t::null:
        res.set(key, Local<Value>());
        break;
    case ordered_json::value_t::array: {
        Local<Array> arrToAdd = Array::newArray();
        for (ordered_json::iterator it = j.begin(); it != j.end(); ++it) JsonToValue_Helper(arrToAdd, *it);
        res.set(key, arrToAdd);
        break;
    }
    case ordered_json::value_t::object: {
        Local<Object> objToAdd = Object::newObject();
        for (ordered_json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(objToAdd, it.key(), it.value());
        res.set(key, objToAdd);
        break;
    }
    default:
        res.set(key, Local<Value>());
        break;
    }
}

void JsonToValue_Helper(Local<Array>& res, ordered_json& j) {
    switch (j.type()) {
    case ordered_json::value_t::string:
        res.add(String::newString(j.get<string>()));
        break;
    case ordered_json::value_t::number_integer:
    case ordered_json::value_t::number_unsigned:
        res.add(BigInteger_Helper(j));
        break;
    case ordered_json::value_t::number_float:
        res.add(Number::newNumber(j.get<double>()));
        break;
    case ordered_json::value_t::boolean:
        res.add(Boolean::newBoolean(j.get<bool>()));
        break;
    case ordered_json::value_t::null:
        res.add(Local<Value>());
        break;
    case ordered_json::value_t::array: {
        Local<Array> arrToAdd = Array::newArray();
        for (ordered_json::iterator it = j.begin(); it != j.end(); ++it) JsonToValue_Helper(arrToAdd, *it);
        res.add(arrToAdd);
        break;
    }
    case ordered_json::value_t::object: {
        Local<Object> objToAdd = Object::newObject();
        for (ordered_json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(objToAdd, it.key(), it.value());
        res.add(objToAdd);
        break;
    }
    default:
        res.add(Local<Value>());
        break;
    }
}

Local<Value> JsonToValue(ordered_json j) {
    Local<Value> res;

    switch (j.type()) {
    case ordered_json::value_t::string:
        res = String::newString(j.get<string>());
        break;
    case ordered_json::value_t::number_integer:
    case ordered_json::value_t::number_unsigned:
        res = BigInteger_Helper(j);
        break;
    case ordered_json::value_t::number_float:
        res = Number::newNumber(j.get<double>());
        break;
    case ordered_json::value_t::boolean:
        res = Boolean::newBoolean(j.get<bool>());
        break;
    case ordered_json::value_t::null:
        res = Local<Value>();
        break;
    case ordered_json::value_t::array: {
        Local<Array> resArr = Array::newArray();
        for (ordered_json::iterator it = j.begin(); it != j.end(); ++it) JsonToValue_Helper(resArr, *it);
        res = resArr;
        break;
    }
    case ordered_json::value_t::object: {
        Local<Object> resObj = Object::newObject();
        for (ordered_json::iterator it = j.begin(); it != j.end(); ++it)
            JsonToValue_Helper(resObj, it.key(), it.value());
        res = resObj;
        break;
    }
    default:
        res = Local<Value>();
        break;
    }

    return res;
}

Local<Value> JsonToValue(std::string jsonStr) {
    try {
        if (jsonStr.empty()) return String::newString("");
        if (jsonStr.front() == '\"' && jsonStr.back() == '\"')
            return String::newString(jsonStr.substr(1, jsonStr.size() - 2));
        auto j = ordered_json::parse(jsonStr, nullptr, true, true);
        return JsonToValue(j);
    } catch (const ordered_json::exception& e) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().warn(
            "{}{}",
            "JSON parse error"_tr(),
            ll::string_utils::tou8str(e.what())
        );
        return String::newString(jsonStr);
    }
}

///////////////////// Value To Json /////////////////////

void ValueToJson_Obj_Helper(ordered_json& res, const Local<Object>& v);

void ValueToJson_Arr_Helper(ordered_json& res, const Local<Array>& v) {
    for (int i = 0; i < v.size(); ++i) {
        switch (v.get(i).getKind()) {
        case ValueKind::kString:
            res.push_back(v.get(i).asString().toString());
            break;
        case ValueKind::kNumber:
            if (CheckIsFloat(v.get(i))) res.push_back(v.get(i).asNumber().toDouble());
            else res.push_back(v.get(i).asNumber().toInt64());
            break;
        case ValueKind::kBoolean:
            res.push_back(v.get(i).asBoolean().value());
            break;
        case ValueKind::kNull:
            res.push_back(nullptr);
            break;
        case ValueKind::kArray: {
            Local<Array> arrToAdd = v.get(i).asArray();
            if (arrToAdd.size() == 0) res.push_back(ordered_json::array());
            else {
                ordered_json arrJson = ordered_json::array();
                ValueToJson_Arr_Helper(arrJson, arrToAdd);
                res.push_back(arrJson);
            }
            break;
        }
        case ValueKind::kObject: {
            Local<Object> objToAdd = v.get(i).asObject();
            if (objToAdd.getKeyNames().empty()) res.push_back(ordered_json::object());
            else {
                ordered_json objJson = ordered_json::object();
                ValueToJson_Obj_Helper(objJson, objToAdd);
                res.push_back(objJson);
            }
            break;
        }
        default:
            res.push_back(nullptr);
            break;
        }
    }
}

void ValueToJson_Obj_Helper(ordered_json& res, const Local<Object>& v) {
    auto keys = v.getKeyNames();
    for (auto& key : keys) {
        switch (v.get(key).getKind()) {
        case ValueKind::kString:
            res.push_back({key, v.get(key).asString().toString()});
            break;
        case ValueKind::kNumber:
            if (CheckIsFloat(v.get(key))) res.push_back({key, v.get(key).asNumber().toDouble()});
            else res.push_back({key, v.get(key).asNumber().toInt64()});
            break;
        case ValueKind::kBoolean:
            res.push_back({key, v.get(key).asBoolean().value()});
            break;
        case ValueKind::kNull:
            res.push_back({key, nullptr});
            break;
        case ValueKind::kArray: {
            Local<Array> arrToAdd = v.get(key).asArray();
            if (arrToAdd.size() == 0) res.push_back({key, ordered_json::array()});
            else {
                ordered_json arrJson = ordered_json::array();
                ValueToJson_Arr_Helper(arrJson, arrToAdd);
                res.push_back({key, arrJson});
            }
            break;
        }
        case ValueKind::kObject: {
            Local<Object> objToAdd = v.get(key).asObject();
            if (objToAdd.getKeyNames().empty()) res.push_back({key, ordered_json::object()});
            else {
                ordered_json objJson = ordered_json::object();
                ValueToJson_Obj_Helper(objJson, objToAdd);
                res.push_back({key, objJson});
            }
            break;
        }
        default:
            res.push_back({key, nullptr});
            break;
        }
    }
}

std::string ValueToJson(Local<Value> v, int formatIndent) {
    string result;
    switch (v.getKind()) {
    case ValueKind::kString:
        result = "\"" + v.asString().toString() + "\"";
        result = ll::string_utils::replaceAll(result, "\n", "\\n");
        break;
    case ValueKind::kNumber:
        if (CheckIsFloat(v)) {
            result = std::to_string(v.asNumber().toDouble());
        } else {
            result = std::to_string(v.asNumber().toInt64());
        }
        break;
    case ValueKind::kBoolean:
        result = std::to_string(v.asBoolean().value());
        break;
    case ValueKind::kNull:
        result = "";
        break;
    case ValueKind::kArray: {
        ordered_json jsonRes = ordered_json::array();
        ValueToJson_Arr_Helper(jsonRes, v.asArray());
        result = jsonRes.dump(formatIndent);
        break;
    }
    case ValueKind::kObject: {
        ordered_json jsonRes = ordered_json::object();
        ValueToJson_Obj_Helper(jsonRes, v.asObject());
        result = jsonRes.dump(formatIndent);
        break;
    }
    default:
        result = "";
        break;
    }
    return result;
}

std::string ValueKindToString(const ValueKind& kind) {
    switch (kind) {
    case ValueKind::kString:
        return "string";
    case ValueKind::kNumber:
        return "number";
    case ValueKind::kBoolean:
        return "boolean";
    case ValueKind::kNull:
        return "null";
    case ValueKind::kObject:
        return "object";
    case ValueKind::kArray:
        return "array";
    case ValueKind::kFunction:
        return "function";
    case ValueKind::kByteBuffer:
        return "bytebuffer";
    default:
        return "unknown";
    }
}
