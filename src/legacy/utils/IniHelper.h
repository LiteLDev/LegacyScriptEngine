#pragma once
#include "SimpleIni.h"

#include <memory>
#include <string>

class SimpleIni : public CSimpleIniA {
public:
    std::string filePath;

    static inline std::unique_ptr<SimpleIni> create(const std::string& path) { return create(path, ""); }
    static std::unique_ptr<SimpleIni>        create(const std::string& path, const std::string& defContent);

    bool        setInt(const std::string& sec, const std::string& key, int value);
    bool        setFloat(const std::string& sec, const std::string& key, float value);
    bool        setString(const std::string& sec, const std::string& key, const std::string& value);
    bool        setBool(const std::string& sec, const std::string& key, bool value);
    int         getInt(const std::string& sec, const std::string& key, int def);
    float       getFloat(const std::string& sec, const std::string& key, float def);
    std::string getString(const std::string& sec, const std::string& key, const std::string& def);
    bool        getBool(const std::string& sec, const std::string& key, bool def);
    bool        deleteKey(const std::string& sec, const std::string& key);
};
