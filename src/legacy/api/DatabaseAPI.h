#pragma once
#include "legacy/api/APIHelp.h"
#include "legacy//db/Session.h"
#include "ll/api/data/KeyValueDB.h"

//////////////////// Classes ////////////////////

//// KVDB
class KVDBClass : public ScriptClass {
private:
    std::unique_ptr<ll::data::KeyValueDB> kvdb                = nullptr;
    int                                   unloadCallbackIndex = -1;

public:
    explicit KVDBClass(Local<Object> const& scriptObj, std::string const& dir);
    explicit KVDBClass(std::string const& dir);
    ~KVDBClass() override;
    static KVDBClass* constructor(Arguments const& args);

    bool isValid() const { return kvdb.get(); }

    Local<Value> get(Arguments const& args);
    Local<Value> set(Arguments const& args);
    Local<Value> del(Arguments const& args);
    Local<Value> close(Arguments const& args);
    Local<Value> listKey(Arguments const& args);

    // For Compatibility
    static Local<Value> newDb(std::string const& dir);
};
extern ClassDefine<KVDBClass> KVDBClassBuilder;

//// SQLDB
class DBSessionClass : public ScriptClass {
private:
    DB::SharedPointer<DB::Session> session;

public:
    explicit DBSessionClass(Local<Object> const& scriptObj, DB::ConnParams const& params);
    explicit DBSessionClass(DB::ConnParams const& params);
    ~DBSessionClass() override;
    static DBSessionClass* constructor(Arguments const& args);

    Local<Value> query(Arguments const& args) const;
    Local<Value> exec(Arguments const& args) const;
    Local<Value> prepare(Arguments const& args) const;
    Local<Value> close(Arguments const& args) const;
    Local<Value> isOpen(Arguments const& args) const;
};
extern ClassDefine<DBSessionClass> DBSessionClassBuilder;

class DBStmtClass : public ScriptClass {
private:
    DB::SharedPointer<DB::Stmt> stmt;

public:
    explicit DBStmtClass(Local<Object> const& scriptObj, DB::SharedPointer<DB::Stmt> const& stmt);
    explicit DBStmtClass(DB::SharedPointer<DB::Stmt> const& stmt);
    ~DBStmtClass() override;

    Local<Value> getAffectedRows() const;
    Local<Value> getInsertId() const;

    Local<Value> bind(Arguments const& args) const;
    Local<Value> execute(Arguments const& args) const;
    Local<Value> step(Arguments const& args) const;
    Local<Value> fetch(Arguments const& args) const;
    Local<Value> fetchAll(Arguments const& args) const;
    Local<Value> reset(Arguments const& args) const;
    Local<Value> reexec(Arguments const& args) const;
    Local<Value> clear(Arguments const& args) const;
};
extern ClassDefine<DBStmtClass> DBStmtClassBuilder;
