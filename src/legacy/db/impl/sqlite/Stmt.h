#pragma once
#include "legacy//db/Stmt.h"

struct sqlite3_stmt;

namespace DB {

class SQLiteSession;

class SQLiteStmt : public Stmt {

    std::shared_ptr<RowHeader> resultHeader;
    sqlite3_stmt*              stmt             = nullptr;
    int                        boundParamsCount = 0;
    int                        totalParamsCount = 0;
    int                        steps            = 0;
    uint64_t                   affectedRowCount = -1;
    uint64_t                   insertRowId      = -1;
    bool                       stepped          = false;
    bool                       executed         = false;
    std::vector<int>           boundIndexes;

    int  getNextParamIndex() const;
    void fetchResultHeader();

public:
    SQLiteStmt(sqlite3_stmt* stmt, std::weak_ptr<Session> const& parent, bool autoExecute);
    ~SQLiteStmt() override;
    Stmt& bind(Any const& value, int index) override;
    Stmt& bind(Any const& value, std::string const& name) override;
    Stmt& bind(Any const& value) override;
    Stmt& execute() override;
    bool  step() override;
    bool  next() override;
    bool  done() override;
    Row   _Fetch() override;
    Stmt& reset() override;
    /**
     * @see Stmt::reexec for details
     * @see https://www.sqlite.org/c3ref/reexec.html
     */
    Stmt&                       reexec() override;
    Stmt&                       clear() override;
    void                        close() override;
    [[nodiscard]] uint64_t      getAffectedRows() const override;
    [[nodiscard]] uint64_t      getInsertId() const override;
    [[nodiscard]] unsigned long getUnboundParams() const override;
    [[nodiscard]] unsigned long getBoundParams() const override;
    [[nodiscard]] unsigned long getParamsCount() const override;
    [[nodiscard]] DBType        getType() const override;

    static SharedPointer<Stmt>
    create(std::weak_ptr<Session> const& sess, std::string const& sql, bool autoExecute = false);
};

} // namespace DB
