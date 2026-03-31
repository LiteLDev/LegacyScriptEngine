#pragma once
#include "legacy//db/ConnParams.h"
#include "legacy//db/Pointer.h"
#include "legacy//db/RowSet.h"
#include "legacy//db/Stmt.h"

#include <stdexcept>

namespace DB {

class Session {

protected:
#if defined(LLDB_DEBUG_MODE)
    bool debugOutput = true;
#else
    bool debugOutput = false;
#endif
    std::weak_ptr<Session>           self;
    std::vector<std::weak_ptr<Stmt>> stmtPool; ///< List of statements opened by prepare method.

public:
    /// Destructor
    virtual ~Session() = default;
    /**
     * @brief Open the database connection.
     *
     * @par Implementation
     * @see SQLiteSession::open
     */
    virtual void open(ConnParams const& params) = 0;
    /**
     * @brief Turn on/off debug output.
     *
     * @param enable  Enable or not
     */
    void setDebugOutput(bool enable);
    /**
     * @brief Change current user and database.
     *
     * @param  user  Username
     * @param  password  Password
     * @param  db    Database name
     * @return bool  Success or not
     * @throws std::runtime_error  If not implemented
     * @par Implementation
     *  None
     */
    virtual bool relogin(std::string const& user, std::string const& password, std::string const& db);
    /**
     * @brief Execute a query.
     *
     * @param  query     Query to execute
     * @param  callback  Callback to process results
     * @return *this
     *
     * @par Implementation
     * @see SQLiteSession::query
     */
    virtual Session& query(std::string const& query, std::function<bool(Row const&)> callback) = 0;
    /**
     * @brief Execute a query.
     *
     * @param  query     The query to execute
     * @return ResultSet Result set
     */
    virtual ResultSet query(std::string const& query);
    /**
     * @brief Execute a query without results.
     *
     * @param  query  The query to execute
     * @return bool   Success or not
     */
    virtual bool execute(std::string const& query) = 0;
    /**
     * @brief Prepare a query.
     *
     * @param  query                The query to execute
     * @param  autoExecute          Whether to execute the statement automatically
     * after binding all parameters
     * @return SharedPointer<Stmt>  The statement
     * @par Example
     * @code
     * auto& stmt = session.prepare("SELECT * FROM table WHERE id = ?");
     * stmt.bind(1);
     * auto res = stmt.getResults();
     * stmt.close();
     * @endcode
     */
    virtual SharedPointer<Stmt> prepare(std::string const& query, bool autoExecute) = 0;
    /**
     * @brief Get the last error message
     *
     * @return std::string  Error message
     */
    [[nodiscard]] virtual std::string getLastError() const;
    /**
     * @brief Get the number of affected rows by the last query.
     *
     * @return uint64_t  The number of affected rows
     */
    [[nodiscard]] virtual uint64_t getAffectedRows() const = 0;
    /**
     * @brief Get the last insert id
     *
     * @return uint64_t  The row id of the last inserted row
     */
    [[nodiscard]] virtual uint64_t getLastInsertId() const = 0;
    /**
     * @brief Close the session.
     *
     */
    virtual void close() = 0;
    /**
     * @brief Get whether the session is open.
     *
     */
    virtual bool isOpen() = 0;
    /**
     * @brief Get the type of session
     *
     * @return DBType  The database type
     */
    virtual DBType getType() = 0;
    /**
     * @brief Get or set the self pointer
     *
     * @return std::weak_ptr<Session>  self
     */
    virtual std::weak_ptr<Session> getOrSetSelf();

    /**
     * @brief Operator<< to execute a query.
     *
     * @param  query  The query to execute
     * @return SharedPointer<Stmt>  The prepared statement
     * @par Example
     * @code
     * ResultSet res;
     * session << "SELECT * FROM table WHERE id = ?", bind(114514), into(res);
     * @endcode
     * @note It is not recommended to store the DB::Stmt reference returned by
     * this method, it will be closed on the next execution.
     */
    virtual SharedPointer<Stmt> operator<<(std::string const& query);

    /**
     * @brief Create a new session.
     *
     * @param  type  Database type
     * @return SharedPointer<Session>  The session
     */
    static SharedPointer<Session> create(DBType type);
    /**
     * @brief Create and open a new session.
     *
     * @param  params  Connection parameters
     * @return SharedPointer<Session>  The session
     */
    static SharedPointer<Session> create(ConnParams const& params);
    /**
     * @brief Create and open a new session.
     *
     * @param  type    Database type
     * @param  params  Connection parameters
     * @return SharedPointer<Session>  The session
     */
    static SharedPointer<Session> create(DBType type, ConnParams const& params);
    /**
     * @brief Create and open a new session.
     *
     * @param  type      Database type
     * @param  host      Hostname
     * @param  port      Port
     * @param  user      Username
     * @param  password  Password
     * @param  database  Database name
     * @return SharedPointer<Session>  The session
     */
    static SharedPointer<Session> create(
        DBType             type,
        std::string const& host,
        uint16_t           port,
        std::string const& user,
        std::string const& password,
        std::string const& database
    );
    /**
     * @brief Create and open a new session.
     *
     * @param  type  Database type
     * @param  path  Path to the database file
     * @return SharedPointer<Session>  The session
     */
    static SharedPointer<Session> create(DBType type, std::string const& path);

private:
    /**
     * @brief Create a new session(internal).
     *
     * @param  type    Database type
     * @param  params  Connection parameters
     * @return SharedPointer<Session>  The session
     */
    static SharedPointer<Session> _Create(DBType type, ConnParams const& params = {});

    static std::vector<std::weak_ptr<Session>> sessionPool; ///< List of sessions(weak pointers)

public:
    /**
     * @brief Get the Session ptr by the (this) pointer.
     *
     * @param  session  The (this) pointer
     * @return std::shared_ptr<Session>  The Session ptr
     */
    static std::shared_ptr<Session> getSession(Session const* session) {
        for (auto& s : sessionPool) {
            if (s.expired()) continue;
            auto ptr = s.lock();
            if (ptr.get() == session) return ptr;
        }
        throw std::runtime_error("Session::getSession: Session is not found or expired");
    }
};

} // namespace DB
