#include "legacyapi/db/Session.h"

#include "lse/Entry.h"
#ifndef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
#include "legacyapi/db/impl/mysql/Session.h"
#endif
#include "legacyapi/db/impl/sqlite/Session.h"
#include "ll/api/io/LoggerRegistry.h"
#include "ll/api/utils/StringUtils.h"

namespace DB {

void Session::setDebugOutput(bool enable) { debugOutput = enable; }

bool Session::relogin(const std::string&, const std::string&, const std::string&) {
    throw std::runtime_error("Session::relogin: Not implemented");
}

ResultSet Session::query(const std::string& query) {
    bool      headerSet = false;
    ResultSet result;
    this->query(query, [&](const Row& row) {
        if (!headerSet) {
            result.header = row.header;
            headerSet     = true;
        }
        result.push_back(row);
        return true;
    });
    IF_ENDBG {
        if (result.valid()) {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug("Session::query: Results >");
            for (auto& str : ll::string_utils::splitByPattern(result.toTableString(), "\n")) {
                lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug(str);
            }
        } else {
            lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug(
                "Session::query: Query returned no result"
            );
        }
    }
    return result;
}

std::string Session::getLastError() const { throw std::runtime_error("Session::getLastError: Not implemented"); }

std::weak_ptr<Session> Session::getOrSetSelf() {
    if (self.expired()) {
        IF_ENDBG lse::LegacyScriptEngine::getInstance().getSelf().getLogger().debug(
            "Session::getOrSetSelf: `self` expired, trying fetching"
        );
        return self = getSession(this);
    }
    return self;
}

SharedPointer<Stmt> Session::operator<<(const std::string& query) { return prepare(query); }

SharedPointer<Session> Session::create(DBType type) { return _Create(type); }
SharedPointer<Session> Session::create(const ConnParams& params) {
    static std::unordered_map<std::string, DBType> names{
        {"sqlite",  DBType::SQLite},
        {"sqlite3", DBType::SQLite},
        {"file",    DBType::SQLite},
        {"mysql",   DBType::MySQL },
        {"mysqlc",  DBType::MySQL }
    };
    ConnParams copy   = params;
    auto       scheme = copy.getScheme();
    std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);
    if (names.count(scheme)) {
        return _Create(names[scheme], params);
    } else {
        throw std::runtime_error("Session::create: Unknown/Unsupported database type");
    }
}
SharedPointer<Session> Session::create(DBType type, const ConnParams& params) { return _Create(type, params); }
SharedPointer<Session> Session::create(
    DBType             type,
    const std::string& host,
    uint16_t           port,
    const std::string& user,
    const std::string& password,
    const std::string& database
) {
    return _Create(
        type,
        {
            {"host",     host    },
            {"port",     port    },
            {"user",     user    },
            {"password", password},
            {"database", database}
    }
    );
}
SharedPointer<Session> Session::create(DBType type, const std::string& path) {
    return _Create(
        type,
        {
            {"path", path}
    }
    );
}

SharedPointer<Session> Session::_Create(DBType type, const ConnParams& params) {
    Session* session = nullptr;
    switch (type) {
    case DBType::SQLite:
        session = params.empty() ? new SQLiteSession() : new SQLiteSession(params);
        break;
    case DBType::MySQL:
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Session::_Create: MySQL is disabled in NodeJS backend because its OpenSSL has conflicts with libnode"
        );
#else
        session = params.empty() ? new MySQLSession() : new MySQLSession(params);
#endif
        break;
    default:
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error(
            "Session::_Create: Unknown/Unsupported database type"
        );
    }
    if (session) {
        auto result  = SharedPointer<Session>(session);
        result->self = result;
        sessionPool.push_back(result);
        return result;
    }
    return SharedPointer<Session>();
}

std::vector<std::weak_ptr<Session>> Session::sessionPool = std::vector<std::weak_ptr<Session>>();

} // namespace DB
