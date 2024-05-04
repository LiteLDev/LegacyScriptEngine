#pragma once
#include "legacyapi/utils/FileHelper.h"
#include "ll/api/Logger.h"
#include "ll/api/i18n/I18n.h"
#include "ll/api/io/FileUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/Entry.h"

#include <Nlohmann/json.hpp>
#include <filesystem>
#include <string>

using namespace nlohmann;

inline ordered_json CreateJson(const std::string& path, const std::string& defContent, bool allowComment = true) {
    ordered_json jsonConf;
    if (!std::filesystem::exists(ll::string_utils::str2wstr(path))) {
        if (path.find('/') != std::string::npos) { // e.g. plugins/LeviLamina/LeviLamina.json
            std::size_t pos = path.find_last_of('/');
            if (pos != std::string::npos) {
                std::string dirPath = path.substr(0, pos);
                std::filesystem::create_directories(dirPath);
            }
        } else if (path.find('\\') != std::string::npos) { // e.g. plugins\\LeviLamina\\LeviLamina.json
            std::size_t pos = path.find_last_of('\\');
            if (pos != std::string::npos) {
                std::string dirPath = path.substr(0, pos);
                std::filesystem::create_directories(dirPath);
            }
        } else {
            lse::getSelfPluginInstance().getLogger().error("Fail in create json file!");
            lse::getSelfPluginInstance().getLogger().error("invalid path");
            jsonConf = ordered_json::object();
        }

        if (!defContent.empty()) {
            try {
                jsonConf = ordered_json::parse(defContent, nullptr, true, allowComment);
            } catch (std::exception& e) {
                lse::getSelfPluginInstance().getLogger().error("Fail to parse default json content!");
                lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));
                jsonConf = ordered_json::object();
            }
        } else {
            jsonConf = ordered_json::object();
        }

        std::ofstream jsonFile(path);
        if (jsonFile.is_open() && !defContent.empty()) jsonFile << jsonConf.dump(4);
        jsonFile.close();
    } else {
        // 已存在
        auto jsonTexts = ll::file_utils::readFile(ll::string_utils::str2u8str(path));
        if (!jsonTexts) {
            jsonConf = ordered_json::object();
        } else {
            try {
                jsonConf = ordered_json::parse(*jsonTexts, nullptr, true, allowComment);
            } catch (std::exception& e) {
                lse::getSelfPluginInstance().getLogger().error("Fail to parse json content in file!");
                lse::getSelfPluginInstance().getLogger().error(ll::string_utils::tou8str(e.what()));
                jsonConf = ordered_json::object();
            }
        }
    }
    return jsonConf;
}
