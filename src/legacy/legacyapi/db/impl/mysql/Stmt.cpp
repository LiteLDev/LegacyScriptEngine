#include "legacyapi/db/impl/mysql/Stmt.h"

#include "legacyapi/db/impl/mysql/Session.h"
#include "legacyapi/utils/StringReader.h"
#include "ll/api/io/Logger.h"
#include "lse/Entry.h"

#include <memory>

#define Wptr2MySQLSession(x) ((MySQLSession*)x.lock().get())

template <>
MYSQL_TIME any_to(DB::Any const& v) {
    switch (v.type) {
    case DB::Any::Type::Date: {
        MYSQL_TIME t;
        t.year        = std::get<DB::Date>(v.value).year;
        t.month       = std::get<DB::Date>(v.value).month;
        t.day         = std::get<DB::Date>(v.value).day;
        t.hour        = 0;
        t.minute      = 0;
        t.second      = 0;
        t.second_part = 0;
        t.time_type   = MYSQL_TIMESTAMP_DATE;
        t.neg         = 0;
        return t;
    }
    case DB::Any::Type::Time: {
        MYSQL_TIME t;
        t.year        = 0;
        t.month       = 0;
        t.day         = 0;
        t.hour        = std::get<DB::Time>(v.value).hour;
        t.minute      = std::get<DB::Time>(v.value).minute;
        t.second      = std::get<DB::Time>(v.value).second;
        t.second_part = 0;
        t.time_type   = MYSQL_TIMESTAMP_TIME;
        t.neg         = 0;
        return t;
    }
    case DB::Any::Type::DateTime: {
        MYSQL_TIME t;
        t.year        = std::get<DB::DateTime>(v.value).date.year;
        t.month       = std::get<DB::DateTime>(v.value).date.month;
        t.day         = std::get<DB::DateTime>(v.value).date.day;
        t.hour        = std::get<DB::DateTime>(v.value).time.hour;
        t.minute      = std::get<DB::DateTime>(v.value).time.minute;
        t.second      = std::get<DB::DateTime>(v.value).time.second;
        t.second_part = 0;
        t.time_type   = MYSQL_TIMESTAMP_DATETIME;
        t.neg         = 0;
        return t;
    }
    default:
        throw std::bad_cast();
    }
}

namespace DB {

/**
 * @brief Parse the query with named parameters
 *
 * @param query  Query with named parameters
 * @return std::unordered_map<std::string, int>  Indexes of parameters
 */
std::unordered_map<std::string, int> ParseStmtParams(std::string& query) {
    lse::legacy::StringReader            reader(query);
    std::unordered_map<std::string, int> result;
    bool                                 inSingleQuote = false;
    bool                                 inDoubleQuote = false;
    bool                                 inString      = false;
    size_t                               delta         = 0;
    int                                  cur           = 0;
    while (reader.isValid()) {
        char c = reader.read();
        if (c == '\'') {
            if (!inDoubleQuote) {
                inSingleQuote = true;
                inString      = true;
            } else if (inSingleQuote) {
                inSingleQuote = false;
                inString      = false;
            }
        } else if (c == '"') {
            if (!inSingleQuote) {
                inDoubleQuote = true;
                inString      = true;
            } else if (inDoubleQuote) {
                inDoubleQuote = false;
                inString      = false;
            }
        } else if (!inString && (c == '?' || c == '$' || c == ':')) {
            query[reader.getPos() - 1 - delta] = '?';                       // Replace $ or : with ?
            auto name                          = reader.readVariableName(); // Read parameter name
            if (name.empty())                                               // No parameter name
            {
                cur++;
                continue;
            }
            if (result.contains(name)) {
                throw std::runtime_error("ParseStmtParams: Duplicate parameter name: " + name);
            }
            result[name] = cur++;
            // Remove the name so that mysql can parse it
            query  = query.substr(0, reader.getPos() - name.length() - delta) + query.substr(reader.getPos() - delta);
            delta += name.size(); // Update delta
        }
    }
    return result;
}

/**
 * @brief Allocate the memory for the buffer in field receiver
 *
 * @param  field  Field to allocate memory for
 * @return std::pair<std::shared_ptr<char[]>, std::size_t>  Allocated memory and
 * its size
 */
std::pair<std::shared_ptr<char[]>, std::size_t> AllocateBuffer(const MYSQL_FIELD& field) {
    std::size_t len = 0;
    switch (field.type) {
    case MYSQL_TYPE_NULL:
        return {nullptr, 0};
    case MYSQL_TYPE_TINY:
        len = sizeof(char);
        break;
    case MYSQL_TYPE_SHORT:
        len = sizeof(short);
        break;
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
        len = sizeof(int);
        break;
    case MYSQL_TYPE_BIT:
    case MYSQL_TYPE_LONGLONG:
        len = sizeof(long long);
        break;
    case MYSQL_TYPE_FLOAT:
        len = sizeof(float);
        break;
    case MYSQL_TYPE_DOUBLE:
        len = sizeof(double);
        break;
    case MYSQL_TYPE_YEAR:
        len = sizeof(short);
        break;
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
        len = sizeof(MYSQL_TIME);
        break;
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_BLOB:
    case MYSQL_TYPE_TINY_BLOB:
    case MYSQL_TYPE_MEDIUM_BLOB:
    case MYSQL_TYPE_LONG_BLOB:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
        len = field.max_length + 1;
        break;
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_NEWDECIMAL:
        len = 64;
        break;
    default:
        throw std::runtime_error("AllocateBuffer: Unsupported MySQL data type: " + std::to_string(field.type));
    }
    if (len) {
        auto buffer = std::shared_ptr<char[]>(new char[len]);
#if defined(LLDB_DEBUG_MODE)
         lse::LegacyScriptEngine::getLogger().debug("AllocateBuffer: Allocated! Buffer size:
         {}", len);
#endif
         return std::make_pair(buffer, len);
    }
    return std::make_pair(std::shared_ptr<char[]>(), len);
}

/**
 * @brief Judge if the field is unsigned type
 *
 * @param  field  Field to judge
 * @return bool   True if the field is unsigned
 */
bool IsUnsigned(const MYSQL_FIELD& field) { return field.flags & UNSIGNED_FLAG; }

/**
 * @brief Convert Receiver to Any
 *
 * @param  rec  Receiver to convert
 * @return Any       Converted value
 */
Any ReceiverToAny(Receiver const& rec) {
    if (rec.isNull || rec.error) {
        return {};
    }
    switch (rec.field.type) {
    case MYSQL_TYPE_NULL:
        return {};
    case MYSQL_TYPE_TINY:
        if (IsUnsigned(rec.field)) return Any(*reinterpret_cast<unsigned char*>(rec.buffer.get()));
        return Any(*reinterpret_cast<char*>(rec.buffer.get()));
    case MYSQL_TYPE_SHORT:
        if (IsUnsigned(rec.field)) return Any(*reinterpret_cast<unsigned short*>(rec.buffer.get()));
        return Any(*reinterpret_cast<short*>(rec.buffer.get()));
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
        if (IsUnsigned(rec.field)) return Any(*reinterpret_cast<unsigned int*>(rec.buffer.get()));
        return Any(*reinterpret_cast<int*>(rec.buffer.get()));
    case MYSQL_TYPE_BIT:
        return Any(*reinterpret_cast<unsigned long long*>(rec.buffer.get()));
    case MYSQL_TYPE_LONGLONG:
        if (IsUnsigned(rec.field)) return Any(*reinterpret_cast<unsigned long long*>(rec.buffer.get()));
        return Any(*reinterpret_cast<long long*>(rec.buffer.get()));
    case MYSQL_TYPE_FLOAT:
        return Any(*reinterpret_cast<float*>(rec.buffer.get()));
    case MYSQL_TYPE_DOUBLE:
        return Any(*reinterpret_cast<double*>(rec.buffer.get()));
    case MYSQL_TYPE_YEAR:
        return Any(*reinterpret_cast<unsigned short*>(rec.buffer.get()));
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_TIMESTAMP:
        return {};
        // TODO: return Any(rec.buffer.get());
    case MYSQL_TYPE_STRING:
    case MYSQL_TYPE_VAR_STRING:
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_SET:
    case MYSQL_TYPE_ENUM:
    // case MYSQL_TYPE_GEOMETRY:
    case MYSQL_TYPE_JSON:
#if defined(LLDB_DEBUG_MODE)
         lse::LegacyScriptEngine::getLogger().debug("ReceiverToAny: string: length: {}, buffer
         {}", rec.length, rec.buffer.get());
#endif
         return Any(std::string(rec.buffer.get()));
     case MYSQL_TYPE_BLOB:
     case MYSQL_TYPE_TINY_BLOB:
     case MYSQL_TYPE_MEDIUM_BLOB:
     case MYSQL_TYPE_LONG_BLOB:
#if defined(LLDB_DEBUG_MODE)
         lse::LegacyScriptEngine::getLogger().debug("ReceiverToAny: blob: length: {}, buffer
         {}", rec.length, rec.buffer.get());
#endif
         // Unknown bug: rec.length is 0
         return Any(ByteArray(rec.buffer.get(), rec.buffer.get() + rec.length));
     case MYSQL_TYPE_DECIMAL:
     case MYSQL_TYPE_NEWDECIMAL:
         // TODO: Decimal
         lse::LegacyScriptEngine::getLogger().debug("MySQL_Decimal: {}",
         std::string(rec.buffer.get(), rec.length)); return {};
     default:
         throw std::runtime_error("ReceiverToAny: Unsupported MySQL data type: " + std::to_string(rec.field.type));
    }
}

MySQLStmt::MySQLStmt(MYSQL_STMT* stmt, std::weak_ptr<Session> const& parent, bool autoExecute)
: Stmt(parent, autoExecute),
  stmt(stmt) {
    this->parent     = parent;
    totalParamsCount = mysql_stmt_param_count(stmt);
    if (totalParamsCount) params = std::make_shared<MYSQL_BIND[]>(totalParamsCount);
    else {
        if (autoExecute) MySQLStmt::execute(); // Execute without params
    }
}

int MySQLStmt::getNextParamIndex() const {
    int res = -1;
    // Find the first unbound parameter
    for (int i = 0; i < boundIndexes.size() && i < totalParamsCount; i++) {
        if (boundIndexes[i] == res + 1) {
            res++;
        }
    }
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug(
        "MySQLStmt::getNextParamIndex: The next param index is {}",
        res + 1
    );
    return res + 1;
}

void MySQLStmt::bindResult() {
    metadata = mysql_stmt_result_metadata(stmt); // Get the result metadata
    if (mysql_stmt_errno(stmt) && !metadata) {
        throw std::runtime_error(
            "MySQLStmt::bindResult: Failed to get result metadata" + std::string(mysql_stmt_error(stmt))
        );
    }
    if (!metadata) {
        IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::bindResult: No result metadata");
        return;
    }
    // Set the attribute UPDATE_MAX_LENGTH to true
    //  so that we can get the max length of the field to allocate the buffer
    bool attr_max_length = true;
    mysql_stmt_attr_set(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, (void const*)&attr_max_length);
    // Store all the results to client or we can't get the max length of the field
    // later
    mysql_stmt_store_result(stmt);
    auto     cnt = mysql_num_fields(metadata);
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::bindResult: mysql_num_fields : {} ", cnt);
    auto     fields = mysql_fetch_fields(metadata);
    result          = std::make_shared<MYSQL_BIND[]>(cnt);
    // Allocate result bindings
    resultHeader = std::make_shared<RowHeader>();
    // Allocate result header
    for (auto i = 0U; i < cnt; i++) {
        resultValues.push_back({});        // Allocate result values
        auto& rec   = resultValues.back(); // Get the binder
        auto& field = fields[i];
        auto& data  = result[i];

        data.buffer_type = field.type;  // Set the type
        data.is_null     = &rec.isNull; // Set the isNull receiver
        data.is_unsigned = static_cast<char>(field.flags & UNSIGNED_FLAG);
        data.error       = &rec.error;  // Set the error receiver
        data.length      = &rec.length; // Set the length receiver

        auto&& [buffer, len] = AllocateBuffer(field);           // Allocate buffer
        rec.buffer           = buffer;                          // Extend lifetime
        data.buffer_length   = static_cast<unsigned long>(len); // Set the buffer length
        data.buffer          = rec.buffer.get();                // Set the buffer address

        rec.field = field;             // Save the field
        resultHeader->add(field.name); // Add the field name to the header
        IF_ENDBG lse::LegacyScriptEngine::getLogger().debug(
            "MySQLStmt::bindResult: Bind result {} (type {}) at index {}",
            field.name,
            static_cast<int>(field.type),
            i
        );
    }
    if (mysql_stmt_bind_result(stmt, result.get())) {
        throw std::runtime_error(
            "MySQLStmt::bindResult: Failed to bind result: " + std::string(mysql_stmt_error(stmt))
        );
    }
}

MySQLStmt::~MySQLStmt() { MySQLStmt::close(); }

Stmt& MySQLStmt::bind(Any const& value, int index) {
    if (index < 0 || index > totalParamsCount) {
        throw std::invalid_argument("MySQLStmt::bind: Invalid argument `index`");
    }
    if (getUnboundParams() == 0) {
        throw std::runtime_error("MySQLStmt::bind: All the parameters are already bound");
    }
    paramValues.push_back({}); // Save the value or it will be lost
    auto& param = paramValues.back();
    switch (value.type) {
    case Any::Type::Null:
        params[index].buffer_type = MYSQL_TYPE_NULL;
        break;
    case Any::Type::Boolean:
        params[index].buffer_type = MYSQL_TYPE_TINY;
        param.buffer.reset(new char[1]);
        param.buffer[0]             = static_cast<char>(std::get<bool>(value.value));
        params[index].buffer        = param.buffer.get();
        params[index].buffer_length = 1;
        params[index].is_unsigned   = true;
        break;
    case Any::Type::Integer:
        params[index].buffer_type = MYSQL_TYPE_LONGLONG;
        param.buffer.reset(new char[sizeof(int64_t)]);
        *reinterpret_cast<int64_t*>(param.buffer.get()) = std::get<int64_t>(value.value);
        params[index].buffer                            = param.buffer.get();
        params[index].buffer_length                     = sizeof(int64_t);
        params[index].is_unsigned                       = false;
        params[index].is_null                           = nullptr;
        params[index].length                            = nullptr;
        break;
    case Any::Type::UInteger:
        params[index].buffer_type = MYSQL_TYPE_LONGLONG;
        param.buffer.reset(new char[sizeof(uint64_t)]);
        *reinterpret_cast<uint64_t*>(param.buffer.get()) = std::get<uint64_t>(value.value);
        params[index].buffer                             = param.buffer.get();
        params[index].buffer_length                      = sizeof(uint64_t);
        params[index].is_unsigned                        = true;
        params[index].is_null                            = nullptr;
        params[index].length                             = nullptr;
        break;
    case Any::Type::Floating:
        params[index].buffer_type = MYSQL_TYPE_DOUBLE;
        param.buffer.reset(new char[sizeof(double)]);
        *reinterpret_cast<double*>(param.buffer.get()) = std::get<double>(value.value);
        params[index].buffer                           = param.buffer.get();
        params[index].buffer_length                    = sizeof(double);
        params[index].is_unsigned                      = false;
        params[index].is_null                          = nullptr;
        params[index].length                           = nullptr;
        break;
    case Any::Type::String: {
        params[index].buffer_type = MYSQL_TYPE_STRING;
        auto sz                   = std::get<std::string>(value.value).length(); // Don't +1 here!!!
        param.buffer.reset(new char[sz]);
        memcpy(param.buffer.get(), std::get<std::string>(value.value).data(), sz); // No '\0'
        param.length = sz; // Must set the length to the buffer size, otherwise it
                           // will be truncated
        params[index].buffer        = param.buffer.get();
        params[index].buffer_length = sz;
        params[index].is_null       = nullptr;
        params[index].length        = (unsigned long*)&param.length;
        IF_ENDBG lse::LegacyScriptEngine::getLogger()
            .debug("MySQLStmt::bind: Bound string param at {}: {}", index, param.buffer.get());
        break;
    }
    case Any::Type::Date:
    case Any::Type::Time:
    case Any::Type::DateTime: {
        params[index].buffer_type = MYSQL_TYPE_TIME;
        auto tm                   = value.get<MYSQL_TIME>();
        param.buffer.reset(new char[sizeof(MYSQL_TIME)]);
        *reinterpret_cast<MYSQL_TIME*>(param.buffer.get()) = tm;
        params[index].buffer                               = param.buffer.get();
        params[index].buffer_length                        = sizeof(MYSQL_TIME);
        params[index].is_null                              = nullptr;
        params[index].length                               = nullptr;
        break;
    }
    case Any::Type::Blob: {
        params[index].buffer_type = MYSQL_TYPE_BLOB;
        auto blob                 = value.get<ByteArray>();
        auto sz                   = blob.size();
        param.buffer.reset(new char[sz]);
        memcpy(param.buffer.get(), blob.data(), sz);
        param.length = sz; // Must set the length to the buffer size, otherwise it
                           // will be truncated
        params[index].buffer        = param.buffer.get();
        params[index].buffer_length = sz;
        params[index].is_null       = nullptr;
        params[index].length        = (unsigned long*)&param.length;
        break;
    }
    default:
        throw std::runtime_error("MySQLStmt::bind: Unsupported type");
    }
    boundIndexes.push_back(index);
    boundParamsCount++;
    if (getUnboundParams() == 0) {
        if (mysql_stmt_bind_param(stmt, params.get())) {
            throw std::runtime_error("MySQLStmt::bind: " + std::string(mysql_stmt_error(stmt)));
        }
        if (autoExecute) execute();
    }
    return *this;
}

Stmt& MySQLStmt::bind(Any const& value, std::string const& name) {
    if (!paramIndexes.contains(name)) {
        throw std::invalid_argument("MySQLStmt::bind: Parameter name `" + name + "` not found");
    }
    return bind(value, paramIndexes[name]);
}

Stmt& MySQLStmt::bind(Any const& value) { return bind(value, getNextParamIndex()); }

Stmt& MySQLStmt::execute() {
    if (mysql_stmt_execute(stmt)) {
        if (!parent.expired()) mysql_rollback(Wptr2MySQLSession(parent)->conn); // Rollback
        throw std::runtime_error("MySQLStmt::execute: Failed to execute stmt: " + std::string(mysql_stmt_error(stmt)));
    }
    mysql_commit(Wptr2MySQLSession(parent)->conn); // Commit
    bindResult();
    if (metadata) // If the statement has result
        step();   // Step to the first result
    paramValues = {};
    return *this;
}

bool MySQLStmt::step() {
    auto res = mysql_stmt_fetch(stmt);
    if (res == MYSQL_NO_DATA) {
        fetched = true;
        return false;
    } else if (res == MYSQL_DATA_TRUNCATED) {
        throw std::runtime_error("MySQLStmt::step: Data truncated!");
    } else if (res) {
        throw std::runtime_error(fmt::format("MySQLStmt::step: {}", mysql_stmt_error(stmt)));
    }
    ++steps;
    return true;
}

bool MySQLStmt::next() { return step(); }

bool MySQLStmt::done() { return fetched; }

Row MySQLStmt::_Fetch() {
    // TODO: execute check
    if (fetched) {
        throw std::runtime_error("MySQLStmt::_Fetch: No more rows");
    }
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::_Fetch: Fetching row...");
    Row      row(resultHeader);
    IF_ENDBG
    lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::_Fetch: RowHeader size {}", row.header->size());
    int i = 0;
    for (auto& col : resultValues) {
        // Because of the inexplicable problems of MySQL C API,
        //  we must set the length of the field manually.
        auto length = *(stmt->bind + i)->length;
        col.length  = length;
        auto v      = ReceiverToAny(col);
        row.push_back(v);
        IF_ENDBG lse::LegacyScriptEngine::getLogger().debug(
            "MySQLStmt::_Fetch: Fetched column `{}`, type {}, value {}",
            col.field.name,
            Any::type2str(v.type),
            v.get<std::string>()
        );
        i++;
    }
    return row;
}

Stmt& MySQLStmt::reset() {
    if (mysql_stmt_reset(stmt)) {
        throw std::runtime_error("MySQLStmt::reset: " + std::string(mysql_stmt_error(stmt)));
    }
    result.reset();
    resultHeader.reset();
    resultValues = {};
    steps        = 0;
    fetched      = false;
    return *this;
}

Stmt& MySQLStmt::reexec() {
    reset();
    execute();
    return *this;
}

Stmt& MySQLStmt::clear() {
    if (query.empty()) {
        throw std::runtime_error("MySQLStmt::clear: No query");
    }
    close();
    *this = *static_cast<MySQLStmt*>(create(parent, query).get());
    return *this;
}

void MySQLStmt::close() {
    if (metadata) {
        mysql_free_result(metadata);
        metadata = nullptr;
    }
    if (stmt) {
        // CRASH!?
        // I(Jasonzyt) had no idea how to fix it, so I just disabled it
        //  idk if it will cause any problem like memory leak but it's better than
        //  crash lol
        // mysql_stmt_close(stmt);
        // stmt = nullptr;
    }
    params.reset();
    result.reset();
    resultHeader.reset();
    totalParamsCount = 0;
    boundParamsCount = 0;
    paramValues      = {};
    resultValues     = {};
    paramIndexes     = {};
    boundIndexes     = {};
    steps            = 0;
    fetched          = false;
}

uint64_t MySQLStmt::getAffectedRows() const { return mysql_stmt_affected_rows(stmt); }

uint64_t MySQLStmt::getInsertId() const { return mysql_stmt_insert_id(stmt); }

unsigned long MySQLStmt::getUnboundParams() const { return totalParamsCount - boundParamsCount; }

unsigned long MySQLStmt::getBoundParams() const { return boundParamsCount; }

unsigned long MySQLStmt::getParamsCount() const { return totalParamsCount; }

DBType MySQLStmt::getType() const { return DBType::MySQL; }

SharedPointer<Stmt> MySQLStmt::create(std::weak_ptr<Session> const& session, std::string const& sql, bool autoExecute) {
    auto s = session.lock();
    if (!s || s->getType() != DBType::MySQL) {
        throw std::invalid_argument("MySQLStmt::create: Session is invalid");
    }
    auto raw  = static_cast<MySQLSession*>(s.get());
    auto stmt = mysql_stmt_init(raw->conn);
    if (!stmt) {
        throw std::runtime_error("MySQLStmt::create: " + s->getLastError());
    }
    auto query  = sql;
    auto params = ParseStmtParams(query);
    if (raw->debugOutput && !params.empty()) {
        lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::create: Parsed named parameters in query: ");
        lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::create: - SQL without named parameters: {}", query);
        for (auto& [k, v] : params) {
            lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::create: - {}: {}", k, v);
        }
    }
    if (mysql_stmt_prepare(stmt, query.c_str(), query.size())) {
        throw std::runtime_error("MySQLStmt::create: " + std::string(mysql_stmt_error(stmt)));
    }
    auto result          = new MySQLStmt(stmt, session, autoExecute);
    result->query        = sql;
    result->paramIndexes = params;
    result->setDebugOutput(raw->debugOutput);
    if (raw->debugOutput) lse::LegacyScriptEngine::getLogger().debug("MySQLStmt::create: Prepared > " + query);
    auto shared  = SharedPointer<Stmt>(result);
    result->self = shared;
    return shared;
}

} // namespace DB
