#include "legacy/utils/IniHelper.h"

#include "ll/api/io/Logger.h"
#include "lse/Entry.h"

#include <filesystem>

std::unique_ptr<SimpleIni> SimpleIni::create(std::string const& path, std::string const& defContent) {
    namespace fs = std::filesystem;
    auto fpath   = fs::path(ll::string_utils::str2wstr(path));
    if (!fpath.empty() && !fs::exists(fpath)) { // Create new file
        if (fpath.has_root_directory() && fpath.has_parent_path()) {
            fs::create_directories(fpath.parent_path());
        }
        std::ofstream iniFile(path);
        if (iniFile.is_open() && defContent != "") iniFile << defContent;
        iniFile.close();
    }

    // Exist
    auto root = std::make_unique<SimpleIni>();
    root->SetUnicode(true);
    auto res = root->LoadFile(path.c_str());
    if (res < 0) {
        lse::LegacyScriptEngine::getLogger().error("Failed in loading ini file");
        lse::LegacyScriptEngine::getLogger().error(std::string("Error Code:") + std::to_string((int)res));
        return nullptr;
    }
    root->filePath = path;
    return root;
}

bool SimpleIni::setInt(std::string const& sec, std::string const& key, int value) {
    bool isOk = SetLongValue(sec.c_str(), key.c_str(), value) >= 0;
    SaveFile(filePath.c_str());
    return isOk;
}

bool SimpleIni::setFloat(std::string const& sec, std::string const& key, float value) {
    bool isOk = SetDoubleValue(sec.c_str(), key.c_str(), value) >= 0;
    SaveFile(filePath.c_str());
    return isOk;
}

bool SimpleIni::setString(std::string const& sec, std::string const& key, std::string const& value) {
    bool isOk = SetValue(sec.c_str(), key.c_str(), value.c_str()) >= 0;
    SaveFile(filePath.c_str());
    return isOk;
}

bool SimpleIni::setBool(std::string const& sec, std::string const& key, bool value) {
    bool isOk = SetBoolValue(sec.c_str(), key.c_str(), value) >= 0;
    SaveFile(filePath.c_str());
    return isOk;
}

int SimpleIni::getInt(std::string const& sec, std::string const& key, int def) const {
    return GetLongValue(sec.c_str(), key.c_str(), def);
}

float SimpleIni::getFloat(std::string const& sec, std::string const& key, float def) const {
    return static_cast<float>(GetDoubleValue(sec.c_str(), key.c_str(), def));
}

std::string SimpleIni::getString(std::string const& sec, std::string const& key, std::string const& def) const {
    return GetValue(sec.c_str(), key.c_str(), def.c_str());
}

bool SimpleIni::getBool(std::string const& sec, std::string const& key, bool def) const {
    return GetBoolValue(sec.c_str(), key.c_str(), def);
}

bool SimpleIni::deleteKey(std::string const& sec, std::string const& key) {
    return Delete(sec.c_str(), key.c_str(), true);
}
