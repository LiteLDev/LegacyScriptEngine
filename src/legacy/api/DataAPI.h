#pragma once
#include "legacy/api/APIHelp.h"
#include "legacy/utils/IniHelper.h"
#include "legacy/utils/JsonHelper.h"

#include <memory>
#include <string>

//////////////////// Data Static ////////////////////

class DataClass {
public:
    static Local<Value> xuid2name(Arguments const& args);
    static Local<Value> name2xuid(Arguments const& args);
    static Local<Value> xuid2uuid(Arguments const& args);
    static Local<Value> name2uuid(Arguments const& args);
    static Local<Value> getAllPlayerInfo(Arguments const& args);
    // New API for LSE
    static Local<Value> fromUuid(Arguments const& args);
    static Local<Value> fromXuid(Arguments const& args);
    static Local<Value> fromName(Arguments const& args);

    static Local<Value> parseJson(Arguments const& args);
    static Local<Value> toJson(Arguments const& args);
    static Local<Value> toMD5(Arguments const& args);
    static Local<Value> toSHA1(Arguments const& args);
    static Local<Value> toBase64(Arguments const& args);
    static Local<Value> fromBase64(Arguments const& args);

    // For Compatibility
    static Local<Value> openConfig(Arguments const& args);
    static Local<Value> openDB(Arguments const& args);
};
extern ClassDefine<void> DataClassBuilder;

Local<Array> objectificationMoneyHistory(std::string const& res);

//////////////////// Money Static ////////////////////

class MoneyClass {
public:
    static Local<Value> set(Arguments const& args);
    static Local<Value> get(Arguments const& args);
    static Local<Value> add(Arguments const& args);
    static Local<Value> reduce(Arguments const& args);
    static Local<Value> trans(Arguments const& args);
    static Local<Value> getHistory(Arguments const& args);
    static Local<Value> clearHistory(Arguments const& args);
};
extern ClassDefine<void> MoneyClassBuilder;

//////////////////// Classes ////////////////////

class ConfBaseClass {
protected:
    std::string  confPath;
    virtual bool flush()  = 0;
    virtual bool close()  = 0;
    virtual bool reload() = 0;

public:
    explicit ConfBaseClass(std::string const& dir);

    virtual Local<Value> reload(Arguments const& args) = 0;
    virtual Local<Value> close(Arguments const& args)  = 0;
    Local<Value>         getPath(Arguments const& args) const;
    Local<Value>         read(Arguments const& args) const;
    virtual Local<Value> write(Arguments const& args) = 0;
    virtual ~ConfBaseClass()                          = default;
};

class ConfJsonClass : public ScriptClass, public ConfBaseClass {
private:
    ordered_json jsonConf;
    bool         flush() override;
    bool         close() override;
    bool         reload() override;

public:
    explicit ConfJsonClass(Local<Object> const& scriptObj, std::string const& path, std::string const& defContent);
    explicit ConfJsonClass(std::string const& path, std::string const& defContent);
    ~ConfJsonClass() override;
    static ConfJsonClass* constructor(Arguments const& args);

    Local<Value>         init(Arguments const& args);
    Local<Value>         get(Arguments const& args);
    Local<Value>         set(Arguments const& args);
    Local<Value>         del(Arguments const& args);
    virtual Local<Value> reload(Arguments const& args) override;
    virtual Local<Value> close(Arguments const& args) override;
    virtual Local<Value> write(Arguments const& args) override;

    // For Compatibility
    static Local<Value> newConf(std::string const& path, std::string const& defContent = "");
};
extern ClassDefine<ConfJsonClass> ConfJsonClassBuilder;

class ConfIniClass : public ScriptClass, public ConfBaseClass {
private:
    std::unique_ptr<SimpleIni> iniConf = nullptr;
    bool                       flush() override;
    bool                       close() override;
    bool                       reload() override;

public:
    explicit ConfIniClass(Local<Object> const& scriptObj, std::string const& path, std::string const& defContent);
    explicit ConfIniClass(std::string const& path, std::string const& defContent);
    ~ConfIniClass() override;
    static ConfIniClass* constructor(Arguments const& args);

    bool isValid() const { return iniConf != nullptr; }

    Local<Value>         init(Arguments const& args);
    Local<Value>         set(Arguments const& args);
    Local<Value>         getStr(Arguments const& args) const;
    Local<Value>         getInt(Arguments const& args) const;
    Local<Value>         getFloat(Arguments const& args) const;
    Local<Value>         getBool(Arguments const& args) const;
    Local<Value>         del(Arguments const& args);
    virtual Local<Value> reload(Arguments const& args) override;
    virtual Local<Value> close(Arguments const& args) override;
    virtual Local<Value> write(Arguments const& args) override;

    // For Compatibility
    static Local<Value> newConf(std::string const& path, std::string const& defContent = "");
};
extern ClassDefine<ConfIniClass> ConfIniClassBuilder;
