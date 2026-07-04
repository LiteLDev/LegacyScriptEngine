#include "legacy//db/impl/sqlite/Session.h"

#include "legacy//db/impl/sqlite/Stmt.h"
#include "ll/api/io/LoggerRegistry.h"
#include "lse/Entry.h"
#include "sqlite3.h"

namespace DB {

SQLiteSession::SQLiteSession() {
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug(
        "SQLiteSession: Constructed! this: {}",
        static_cast<void*>(this)
    );
}
SQLiteSession::SQLiteSession(ConnParams const& params) {
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug(
        "SQLiteSession: Constructed! this: {}",
        static_cast<void*>(this)
    );
    SQLiteSession::open(params);
}

SQLiteSession::~SQLiteSession() {
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug(
        "SQLiteSession::~SQLiteSession: Destructor: this: {}",
        static_cast<void*>(this)
    );
    SQLiteSession::close();
}

void SQLiteSession::open(ConnParams const& params) {
    // see https://www.sqlite.org/c3ref/open.html
    auto p    = params; // Copy to avoid modifying the origin.
    auto path = p.getPath();
    if (path.empty()) {
        path = ":memory:";
        // throw std::invalid_argument("SQLiteSession::SQLiteSession: No path
        // specified!");
    }
    auto flags = 0;
    if (p.get<bool>({"create", "create_if_not_exist", "createifnotexist"}, true, false)) {
        flags |= SQLITE_OPEN_CREATE;
    }
    if (p.get<bool>({"readonly", "readonly_mode", "readonlymode", "r"}, false, false)) {
        flags |= SQLITE_OPEN_READONLY;
    }
    if (p.get<bool>({"readwrite", "readwrite_mode", "readwritemode", "read_write", "rw"}, false, false)) {
        flags |= SQLITE_OPEN_READWRITE;
    }
    if (p.get<bool>({"privatecache", "private_cache"}, false, false)) {
        flags |= SQLITE_OPEN_PRIVATECACHE;
    }
    if (p.get<bool>({"sharedcache", "shared_cache"}, false, false)) {
        flags |= SQLITE_OPEN_SHAREDCACHE;
    }
    if (p.get<bool>({"nomutex", "no_mutex"}, false, false)) {
        flags |= SQLITE_OPEN_NOMUTEX;
    }
    if (p.get<bool>({"fullmutex", "full_mutex"}, false, false)) {
        flags |= SQLITE_OPEN_FULLMUTEX;
    }
    if (p.get<bool>({"nofollow", "no_follow"}, false, false)) {
        flags |= SQLITE_OPEN_NOFOLLOW;
    }
    if (!params.contains("readonly") && !params.contains("readwrite")) {
        flags |= SQLITE_OPEN_READWRITE;
    }
    if (!params.contains("create")) {
        flags |= SQLITE_OPEN_CREATE;
    }
    auto res = sqlite3_open_v2(path.c_str(), &conn, flags, nullptr);
    if (res != SQLITE_OK) {
        throw std::runtime_error("SQLiteSession::open: Failed to open database: " + std::string(sqlite3_errmsg(conn)));
    }
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("SQLiteSession::open: Opened database: {}", std::string(path));
}

bool SQLiteSession::execute(std::string const& query) {
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("SQLiteSession::execute: Executing > {}", query);
    auto     res = sqlite3_exec(conn, query.c_str(), nullptr, nullptr, nullptr);
    return res == SQLITE_OK;
}

Session& SQLiteSession::query(std::string const& query, std::function<bool(Row const&)> callback) {
    IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("SQLiteSession::query: Querying > {}", query);
    auto     stmt = prepare(query, false);
    stmt->fetchAll(callback);
    return *this;
}

SharedPointer<Stmt> SQLiteSession::prepare(std::string const& query, bool autoExecute) {
    auto stmt = SQLiteStmt::create(getOrSetSelf(), query, autoExecute);
    stmtPool.push_back(stmt);
    return stmt;
}

std::string SQLiteSession::getLastError() const { return sqlite3_errmsg(conn); }

uint64_t SQLiteSession::getAffectedRows() const { return sqlite3_changes(conn); }

uint64_t SQLiteSession::getLastInsertId() const { return sqlite3_last_insert_rowid(conn); }

void SQLiteSession::close() {
    while (!stmtPool.empty()) {
        // Close all the active statements or it will error when closing
        auto& wptr = stmtPool.back();
        auto  ptr  = wptr.lock();
        if (!wptr.expired() && ptr) {
            ptr->close();
        }
        stmtPool.pop_back();
    }
    if (conn) {
        auto res = sqlite3_close(conn);
        if (res != SQLITE_OK) {
            throw std::runtime_error(
                "SQLiteSession::close: Failed to close database: " + std::string(sqlite3_errmsg(conn))
            );
        }
        conn = nullptr;
        IF_ENDBG lse::LegacyScriptEngine::getLogger().debug("SQLiteSession::close: Closed database");
    }
}

bool SQLiteSession::isOpen() { return conn != nullptr; }

DBType SQLiteSession::getType() { return DBType::SQLite; }

bool SQLiteSession::backup(std::filesystem::path const& backupPath) {
    if (!conn) {
        IF_ENDBG lse::LegacyScriptEngine::getLogger().error("SQLiteSession::backup: No open database connection.");
        return false;
    }

    struct DestDb {
        sqlite3* mDb{nullptr};
        explicit DestDb(std::filesystem::path const& path) {
            {
                std::error_code ec;
                std::filesystem::create_directories(path.parent_path(), ec);
            }
            if (sqlite3_open_v2(
                    ll::string_utils::u8str2str(path.u8string()).c_str(),
                    &mDb,
                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE,
                    nullptr
                )
                != SQLITE_OK) {
                if (mDb) sqlite3_close(mDb);
                mDb = nullptr;
                return;
            }
        }
        ~DestDb() {
            if (mDb) sqlite3_close(mDb);
        }
    } destDb{backupPath};
    if (!destDb.mDb) {
        IF_ENDBG lse::LegacyScriptEngine::getLogger().error(
            "SQLiteSession::backup: Failed to open destination database '{}'",
            backupPath.string()
        );
        return false;
    }

    sqlite3_exec(conn, "ROLLBACK", nullptr, nullptr, nullptr);
    struct Backup {
        sqlite3_backup* mBackup{nullptr};
        Backup(sqlite3* srcDb, sqlite3* destDb) : mBackup(sqlite3_backup_init(destDb, "main", srcDb, "main")) {}
        ~Backup() {
            if (mBackup) sqlite3_backup_finish(mBackup);
        }
    } backup{conn, destDb.mDb};
    if (!backup.mBackup) {
        IF_ENDBG lse::LegacyScriptEngine::getLogger().error("SQLiteSession::backup: Failed to initialize backup.");
        return false;
    }

    if (auto rc = sqlite3_backup_step(backup.mBackup, -1); rc != SQLITE_DONE) {
        IF_ENDBG lse::LegacyScriptEngine::getLogger()
            .error("SQLiteSession::backup: Backup step failed with code {} ({})", rc, sqlite3_errstr(rc));
        return false;
    }

    IF_ENDBG lse::LegacyScriptEngine::getLogger().info(
        "SQLiteSession::backup: Database successfully backed up to '{}'",
        backupPath.string()
    );
    return true;
}

SharedPointer<Stmt> SQLiteSession::operator<<(std::string const& query) { return prepare(query, true); }

} // namespace DB
