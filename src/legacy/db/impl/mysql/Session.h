#pragma once
#include "legacy//db/Session.h"

#include <mariadb/mysql.h>

namespace DB {

class MySQLStmt;

class MySQLSession : public Session {

    MYSQL* conn = nullptr;

    void setSSL(ConnParams const& params) const;

public:
    MySQLSession();
    explicit MySQLSession(ConnParams const& params);
    ~MySQLSession() override;
    void                open(ConnParams const& params) override;
    bool                execute(std::string const& query) override;
    bool                relogin(std::string const& user, std::string const& password, std::string const& db) override;
    Session&            query(std::string const& query, std::function<bool(Row const&)> callback) override;
    SharedPointer<Stmt> prepare(std::string const& query, bool autoExecute) override;
    [[nodiscard]] std::string getLastError() const override;
    [[nodiscard]] uint64_t    getAffectedRows() const override;
    [[nodiscard]] uint64_t    getLastInsertId() const override;
    void                      close() override;
    bool                      isOpen() override;
    DBType                    getType() override;

    SharedPointer<Stmt> operator<<(std::string const& query) override;

    friend class MySQLStmt;
};

} // namespace DB
