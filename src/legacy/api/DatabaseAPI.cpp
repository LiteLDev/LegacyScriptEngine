#include "legacy/api/DatabaseAPI.h"

using namespace DB;

//////////////////// Class Definition ////////////////////

ClassDefine<KVDBClass> KVDBClassBuilder = defineClass<KVDBClass>("KVDatabase")
                                              .constructor(&KVDBClass::constructor)
                                              .instanceFunction("get", &KVDBClass::get)
                                              .instanceFunction("set", &KVDBClass::set)
                                              .instanceFunction("delete", &KVDBClass::del)
                                              .instanceFunction("close", &KVDBClass::close)
                                              .instanceFunction("listKey", &KVDBClass::listKey)
                                              .build();

ClassDefine<DBSessionClass> DBSessionClassBuilder = defineClass<DBSessionClass>("DBSession")
                                                        .constructor(&DBSessionClass::constructor)
                                                        .instanceFunction("query", &DBSessionClass::query)
                                                        .instanceFunction("exec", &DBSessionClass::exec)
                                                        .instanceFunction("execute", &DBSessionClass::exec)
                                                        .instanceFunction("prepare", &DBSessionClass::prepare)
                                                        .instanceFunction("close", &DBSessionClass::close)
                                                        .instanceFunction("isOpen", &DBSessionClass::isOpen)
                                                        .build();

ClassDefine<DBStmtClass> DBStmtClassBuilder = defineClass<DBStmtClass>("DBStmt")
                                                  .constructor(nullptr)
                                                  .instanceProperty("affectedRows", &DBStmtClass::getAffectedRows)
                                                  .instanceProperty("insertId", &DBStmtClass::getInsertId)

                                                  .instanceFunction("bind", &DBStmtClass::bind)
                                                  .instanceFunction("execute", &DBStmtClass::execute)
                                                  .instanceFunction("step", &DBStmtClass::step)
                                                  .instanceFunction("fetch", &DBStmtClass::fetch)
                                                  .instanceFunction("fetchAll", &DBStmtClass::fetchAll)
                                                  .instanceFunction("reset", &DBStmtClass::reset)
                                                  .instanceFunction("reexec", &DBStmtClass::reexec)
                                                  .instanceFunction("clear", &DBStmtClass::clear)
                                                  .build();

//////////////////// Functions ////////////////////

Any LocalValueToAny(Local<Value> const& val) {
    switch (val.getKind()) {
    case ValueKind::kObject:
    case ValueKind::kArray:
        throw std::exception("Cannot convert script object(array) to Any");
    case ValueKind::kNull:
    case ValueKind::kUnsupported:
        return Any();
    case ValueKind::kBoolean:
        return Any(val.asBoolean().value());
    case ValueKind::kNumber: {
        if (CheckIsFloat(val.asNumber())) return Any(val.asNumber().toDouble());
        return Any(val.asNumber().toInt64());
    }
    case ValueKind::kString:
        return Any(val.asString().toString());
    case ValueKind::kByteBuffer:
        switch (val.asByteBuffer().getType()) {
        case ByteBuffer::Type::kInt8:
        case ByteBuffer::Type::kUint8: {
            auto buf = static_cast<uint8_t*>(val.asByteBuffer().getRawBytes());
            return Any(ByteArray(buf, buf + val.asByteBuffer().elementCount()));
        }
        default:
            break;
        }
        break;
    default:
        break;
    }
    return Any();
}

template <>
Local<Value> any_to(Any const& val) {
    switch (val.type) {
    case Any::Type::Null:
        return {};
    case Any::Type::Boolean:
        return Boolean::newBoolean(std::get<bool>(val.value));
    case Any::Type::Integer:
        return Number::newNumber(std::get<int64_t>(val.value));
    case Any::Type::UInteger:
        if (std::get<uint64_t>(val.value) > LLONG_MAX)
            return Number::newNumber(static_cast<double>(std::get<uint64_t>(val.value)));
        return Number::newNumber(static_cast<int64_t>(std::get<uint64_t>(val.value)));
    case Any::Type::Floating:
        return Number::newNumber(std::get<double>(val.value));
    case Any::Type::String:
        return String::newString(std::get<std::string>(val.value));
    case Any::Type::Date: {
        auto obj = Object::newObject();
        obj.set("Y", std::get<Date>(val.value).year);
        obj.set("M", std::get<Date>(val.value).month);
        obj.set("D", std::get<Date>(val.value).day);
        return obj;
    }
    case Any::Type::Time: {
        auto obj = Object::newObject();
        obj.set("h", std::get<Time>(val.value).hour);
        obj.set("m", std::get<Time>(val.value).minute);
        obj.set("s", std::get<Time>(val.value).second);
        return obj;
    }
    case Any::Type::DateTime: {
        auto obj = Object::newObject();
        obj.set("Y", std::get<DateTime>(val.value).date.year);
        obj.set("M", std::get<DateTime>(val.value).date.month);
        obj.set("D", std::get<DateTime>(val.value).date.day);
        obj.set("h", std::get<DateTime>(val.value).time.hour);
        obj.set("m", std::get<DateTime>(val.value).time.minute);
        obj.set("s", std::get<DateTime>(val.value).time.second);
        return obj;
    }
    case Any::Type::Blob:
        return String::newString(val.get<std::string>());
    default:
        break;
    }
    return {};
}

Local<Value> RowSetToLocalValue(RowSet const& rows) {
    if (rows.empty() || !rows.header) {
        return {};
    }
    Local<Array> arr    = Array::newArray();
    Local<Array> header = Array::newArray();
    for (auto& col : *rows.header) header.add(String::newString(col));
    arr.add(header);
    for (auto& row : rows) {
        Local<Array> rowValues = Array::newArray();
        for (auto& col : row) rowValues.add(col.get<Local<Value>>());
        arr.add(rowValues);
    }
    return arr;
}

Local<Value> RowToLocalValue(Row const& row) {
    auto result = Object::newObject();
    row.forEach([&](std::string const& key, Any const& value) {
        result.set(key, value.get<Local<Value>>());
        return true;
    });
    return result;
}

//////////////////// Classes KVDB ////////////////////

// 生成函数
KVDBClass::KVDBClass(Local<Object> const& scriptObj, std::string const& dir) : ScriptClass(scriptObj) {
    try {
        kvdb = std::make_unique<ll::data::KeyValueDB>(dir);
    } catch (...) {
        kvdb.reset();
    }

    unloadCallbackIndex = getEngineOwnData()->addUnloadCallback([&](std::shared_ptr<ScriptEngine>) { kvdb.reset(); });
}

KVDBClass::KVDBClass(std::string const& dir) : ScriptClass(script::ScriptClass::ConstructFromCpp<KVDBClass>{}) {
    try {
        kvdb = std::make_unique<ll::data::KeyValueDB>(dir);
    } catch (...) {
        kvdb.reset();
    }
    unloadCallbackIndex = getEngineOwnData()->addUnloadCallback([&](std::shared_ptr<ScriptEngine>) { kvdb.reset(); });
}

KVDBClass::~KVDBClass() {}

KVDBClass* KVDBClass::constructor(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto res = new KVDBClass(args.thiz(), args[0].asString().toString());
        if (res->isValid()) return res;
        return nullptr;
    }
    CATCH_AND_THROW
}

Local<Value> KVDBClass::get(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (!isValid()) return {};

        auto res = kvdb->get(args[0].asString().toString());
        if (!res) return {};

        return JsonToValue(*res);
    }
    CATCH_AND_THROW
}

Local<Value> KVDBClass::set(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (!isValid()) return {};

        kvdb->set(args[0].asString().toString(), ValueToJson(args[1]));
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> KVDBClass::del(Arguments const& args) {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        if (!isValid()) return {};

        return Boolean::newBoolean(kvdb->del(args[0].asString().toString()));
    }
    CATCH_AND_THROW
}

Local<Value> KVDBClass::close(Arguments const&) {
    getEngineOwnData()->removeUnloadCallback(unloadCallbackIndex);
    unloadCallbackIndex = -1;
    try {
        kvdb.reset();
        return Boolean::newBoolean(true);
    }
    CATCH_AND_THROW
}

Local<Value> KVDBClass::listKey(Arguments const&) {
    try {
        if (!isValid()) return {};

        Local<Array> array = Array::newArray();
        for (auto const& [key, _] : kvdb->iter()) {
            array.add(String::newString(key));
        }
        return array;
    }
    CATCH_AND_THROW
}

//////////////////// Classes DBSession ////////////////////

// 生成函数
DBSessionClass::DBSessionClass(Local<Object> const& scriptObj, ConnParams const& params)
: ScriptClass(scriptObj),
  session(Session::create(params)) {
    session->setDebugOutput(true);
}
DBSessionClass::DBSessionClass(ConnParams const& params)
: ScriptClass(script::ScriptClass::ConstructFromCpp<DBSessionClass>{}),
  session(Session::create(params)) {
    session->setDebugOutput(true);
}

DBSessionClass::~DBSessionClass() {}

DBSessionClass* DBSessionClass::constructor(Arguments const& args) {
    try {
        DBSessionClass* result = nullptr;
        switch (args.size()) {
        case 1: {
            // When the first argument is a string, it is a url or file path.
            if (args[0].isString()) {
                result = new DBSessionClass(args.thiz(), ConnParams(args[0].asString().toString()));
            } else if (args[0].isObject()) {
                auto       obj = args[0].asObject();
                ConnParams params;
                for (auto& key : obj.getKeys()) params[key.toString()] = LocalValueToAny(obj.get(key));
                result = new DBSessionClass(args.thiz(), params);
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
            break;
        }
        case 2: {
            CHECK_ARG_TYPE(args[0], ValueKind::kString);
            CHECK_ARG_TYPE(args[1], ValueKind::kObject);
            auto       obj = args[1].asObject();
            ConnParams params;
            params["type"] = Any(args[0].asString().toString());
            for (auto& key : obj.getKeys()) params[key.toString()] = LocalValueToAny(obj.get(key));
            result = new DBSessionClass(args.thiz(), ConnParams(params));
            break;
        }
        default:
            throw WrongArgTypeException(__FUNCTION__);
        }
        return result;
    }
    CATCH_AND_THROW
}

Local<Value> DBSessionClass::query(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto res = session->query(args[0].asString().toString());
        return RowSetToLocalValue(res);
    }
    CATCH_AND_THROW
}

Local<Value> DBSessionClass::exec(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        session->execute(args[0].asString().toString());
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBSessionClass::prepare(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);

    try {
        auto stmt = new DBStmtClass(session->prepare(args[0].asString().toString(), false));
        return stmt->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBSessionClass::close(Arguments const& args) const {

    try {
        session->close();
        return Boolean::newBoolean(true);
    }
    CATCH
    return Boolean::newBoolean(false);
}
Local<Value> DBSessionClass::isOpen(Arguments const& args) const {

    try {
        return Boolean::newBoolean(session->isOpen());
    }
    CATCH_AND_THROW
}

//////////////////// Classes DBStmt ////////////////////

// 生成函数
DBStmtClass::DBStmtClass(Local<Object> const& scriptObj, DB::SharedPointer<DB::Stmt> const& stmt)
: ScriptClass(scriptObj),
  stmt(stmt) {}

DBStmtClass::DBStmtClass(DB::SharedPointer<DB::Stmt> const& stmt)
: ScriptClass(script::ScriptClass::ConstructFromCpp<DBStmtClass>{}),
  stmt(stmt) {}

DBStmtClass::~DBStmtClass() {}

Local<Value> DBStmtClass::getAffectedRows() const {
    try {
        auto res = stmt->getAffectedRows();
        if (res == static_cast<uint64_t>(-1)) return Number::newNumber(-1);
        if (res > LLONG_MAX) return Number::newNumber(static_cast<double>(res));
        return Number::newNumber(static_cast<int64_t>(res));
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::getInsertId() const {
    try {
        auto res = stmt->getInsertId();
        if (res == static_cast<uint64_t>(-1)) return Number::newNumber(-1);
        if (res > LLONG_MAX) return Number::newNumber(static_cast<double>(res));
        return Number::newNumber(static_cast<int64_t>(res));
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::bind(Arguments const& args) const {
    CHECK_ARGS_COUNT(args, 1);
    try {
        if (args.size() == 1) {
            switch (args[0].getKind()) {
            case ValueKind::kArray: {
                auto arr = args[0].asArray();
                for (size_t i = 0; i < arr.size(); ++i) stmt->bind(LocalValueToAny(arr.get(i)));
                break;
            }
            case ValueKind::kObject: {
                auto obj = args[0].asObject();
                for (auto& key : obj.getKeys()) stmt->bind(LocalValueToAny(obj.get(key)), key.toString());
                break;
            }
            default:
                stmt->bind(LocalValueToAny(args[0]));
            }
        } else if (args.size() >= 2) {
            if (args[1].isNumber()) {
                stmt->bind(LocalValueToAny(args[0]), static_cast<int>(args[1].asNumber().toInt64()));
            } else if (args[1].isString()) {
                stmt->bind(LocalValueToAny(args[0]), args[1].asString().toString());
            } else {
                throw WrongArgTypeException(__FUNCTION__);
            }
        }
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::execute(Arguments const& args) const {
    try {
        stmt->execute();
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::step(Arguments const& args) const {
    try {
        return Boolean::newBoolean(stmt->step());
    }
    CATCH
    return Boolean::newBoolean(false);
}

Local<Value> DBStmtClass::fetch(Arguments const& args) const {
    try {
        return RowToLocalValue(stmt->fetch());
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::fetchAll(Arguments const& args) const {
    try {
        if (args.size() == 0) {
            return RowSetToLocalValue(stmt->fetchAll());
        }
        CHECK_ARG_TYPE(args[0], ValueKind::kFunction);
        auto func = args[0].asFunction();
        stmt->fetchAll([&](Row const& row) {
            auto res = func.call({}, RowToLocalValue(row));
            if (res.isBoolean()) {
                return res.asBoolean().value();
            }
            return true;
        });
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::reset(Arguments const& args) const {
    try {
        stmt->reset();
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::reexec(Arguments const& args) const {
    try {
        stmt->reexec();
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}

Local<Value> DBStmtClass::clear(Arguments const& args) const {
    try {
        stmt->clear();
        return this->getScriptObject();
    }
    CATCH_AND_THROW
}
