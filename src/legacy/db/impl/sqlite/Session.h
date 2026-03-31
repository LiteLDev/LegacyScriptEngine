#pragma once
#include "legacy/legacyapi/db/Session.h"

struct sqlite3;
namespace DB {

class SQLiteSession : public Session {

    sqlite3* conn = nullptr;

public:
    SQLiteSession();
    explicit SQLiteSession(ConnParams const& params);
    ~SQLiteSession() override;
    void                      open(ConnParams const& params) override;
    bool                      execute(std::string const& query) override;
    Session&                  query(std::string const& query, std::function<bool(Row const&)> callback) override;
    SharedPointer<Stmt>       prepare(std::string const& query, bool autoExecute) override;
    [[nodiscard]] std::string getLastError() const override;
    [[nodiscard]] uint64_t    getAffectedRows() const override;
    [[nodiscard]] uint64_t    getLastInsertId() const override;
    void                      close() override;
    bool                      isOpen() override;
    DBType                    getType() override;

    SharedPointer<Stmt> operator<<(std::string const& query) override;

    friend class SQLiteStmt;
};

} // namespace DB
