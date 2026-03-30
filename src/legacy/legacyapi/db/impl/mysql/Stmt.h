#pragma once
#include "legacyapi/db/Stmt.h"

#include <mariadb/mysql.h>

namespace DB {

class MySQLSession;

struct Receiver {
    MYSQL_FIELD             field;
    std::shared_ptr<char[]> buffer;
    unsigned long           length     = 0;
    my_bool                 isNull     = false;
    bool                    isUnsigned = false;
    my_bool                 error      = false;
};

class MySQLStmt : public Stmt {

    MYSQL_STMT*                          stmt         = nullptr;
    MYSQL_RES*                           metadata     = nullptr;
    std::shared_ptr<MYSQL_BIND[]>        params       = nullptr; ///< Parameters to bind
    std::shared_ptr<MYSQL_BIND[]>        result       = nullptr; ///< Result of query
    std::shared_ptr<RowHeader>           resultHeader = nullptr;
    std::vector<int>                     boundIndexes;
    std::vector<Receiver>                paramValues;
    std::vector<Receiver>                resultValues;
    std::unordered_map<std::string, int> paramIndexes;
    std::string                          query;
    unsigned long                        boundParamsCount = 0;
    unsigned long                        totalParamsCount = 0;
    int                                  steps            = 0;
    bool                                 fetched          = false;

    MySQLStmt(MYSQL_STMT* stmt, std::weak_ptr<Session> const& parent, bool autoExecute = false);
    [[nodiscard]] int getNextParamIndex() const;
    void              bindResult();

public:
    ~MySQLStmt() override;
    Stmt&                       bind(Any const& value, int index) override;
    Stmt&                       bind(Any const& value, std::string const& name) override;
    Stmt&                       bind(Any const& value) override;
    Stmt&                       execute() override;
    bool                        step() override;
    bool                        next() override;
    bool                        done() override;
    Row                         _Fetch() override;
    Stmt&                       reset() override;
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
