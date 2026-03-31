#include "legacy//db/Session.h"

#include "legacy//db/impl/mysql/Session.h"
#include "legacy//db/impl/sqlite/Session.h"
#include "ll/api/io/LoggerRegistry.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/Entry.h"

namespace DB {

void Session::setDebugOutput(bool enable) { debugOutput = enable; }

bool Session::relogin(std::string const&, std::string const&, std::string const&) {
    throw std::runtime_error("Session::relogin: Not implemented");
}

ResultSet Session::query(std::string const& query) {
    bool      headerSet = false;
    ResultSet result;
    this->query(query, [&](Row const& row) {
        if (!headerSet) {
            result.header = row.header;
            headerSet     = true;
        }
        result.push_back(row);
        return true;
    });
    IF_ENDBG {
        if (result.valid()) {
            lse::LegacyScriptEngine::getLogger().debug("Session::query: Results >");
            for (auto& str : ll::string_utils::splitByPattern(result.toTableString(), "\n")) {
                lse::LegacyScriptEngine::getLogger().debug(str);
            }
        } else {
            lse::LegacyScriptEngine::getLogger().debug("Session::query: Query returned no result");
        }
    }
    return result;
}

std::string Session::getLastError() const { throw std::runtime_error("Session::getLastError: Not implemented"); }

std::weak_ptr<Session> Session::getOrSetSelf() {
    if (self.expired()) {
        IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("Session::getOrSetSelf: `self` expired, trying fetching");
        return self = getSession(this);
    }
    return self;
}

SharedPointer<Stmt> Session::operator<<(std::string const& query) { return prepare(query, false); }

SharedPointer<Session> Session::create(DBType type) { return _Create(type); }
SharedPointer<Session> Session::create(ConnParams const& params) {
    static std::unordered_map<std::string, DBType> names{
        {"sqlite",  DBType::SQLite},
        {"sqlite3", DBType::SQLite},
        {"file",    DBType::SQLite},
        {"mysql",   DBType::MySQL },
        {"mysqlc",  DBType::MySQL }
    };
    ConnParams copy   = params;
    auto       scheme = copy.getScheme();
    std::ranges::transform(scheme, scheme.begin(), ::tolower);
    if (names.contains(scheme)) {
        return _Create(names[scheme], params);
    }
    throw std::runtime_error("Session::create: Unknown/Unsupported database type");
}
SharedPointer<Session> Session::create(DBType type, ConnParams const& params) { return _Create(type, params); }
SharedPointer<Session> Session::create(
    DBType             type,
    std::string const& host,
    uint16_t           port,
    std::string const& user,
    std::string const& password,
    std::string const& database
) {
    return _Create(
        type,
        {
            {"host",     Any(host)    },
            {"port",     Any(port)    },
            {"user",     Any(user)    },
            {"password", Any(password)},
            {"database", Any(database)}
    }
    );
}
SharedPointer<Session> Session::create(DBType type, std::string const& path) {
    return _Create(
        type,
        {
            {"path", Any(path)}
    }
    );
}

SharedPointer<Session> Session::_Create(DBType type, ConnParams const& params) {
    std::shared_ptr<Session> session = nullptr;
    switch (type) {
    case DBType::SQLite:
        session = params.empty() ? std::make_shared<SQLiteSession>() : std::make_shared<SQLiteSession>(params);
        break;
    case DBType::MySQL:
        session = params.empty() ? std::make_shared<MySQLSession>() : std::make_shared<MySQLSession>(params);
        break;
    default:
        lse::LegacyScriptEngine::getLogger().error("Session::_Create: Unknown/Unsupported database type");
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
