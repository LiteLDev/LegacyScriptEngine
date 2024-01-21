class DBStorage;
class MoreGlobal {
private:
    static DBStorage* db;

public:
    static void       Init();
    static DBStorage* getDBStorage() { return db; };
    static void       setDBStorage(DBStorage* dbs) { db = dbs; };
};