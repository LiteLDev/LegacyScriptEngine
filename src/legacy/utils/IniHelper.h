#pragma once
#include "SimpleIni.h"

#include <memory>
#include <string>

class SimpleIni : public CSimpleIniA {
public:
    std::string filePath;

    static std::unique_ptr<SimpleIni> create(std::string const& path) { return create(path, ""); }
    static std::unique_ptr<SimpleIni> create(std::string const& path, std::string const& defContent);

    bool        setInt(std::string const& sec, std::string const& key, int value);
    bool        setFloat(std::string const& sec, std::string const& key, float value);
    bool        setString(std::string const& sec, std::string const& key, std::string const& value);
    bool        setBool(std::string const& sec, std::string const& key, bool value);
    int         getInt(std::string const& sec, std::string const& key, int def) const;
    float       getFloat(std::string const& sec, std::string const& key, float def) const;
    std::string getString(std::string const& sec, std::string const& key, std::string const& def) const;
    bool        getBool(std::string const& sec, std::string const& key, bool def) const;
    bool        deleteKey(std::string const& sec, std::string const& key);
};
