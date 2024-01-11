#pragma once
#include "legacyapi/utils/FileHelper.h"
#include "ll/api/Logger.h"
#include "ll/api/i18n/I18nAPI.h"
#include "ll/api/utils/StringUtils.h"
#include <Nlohmann/json.hpp>
#include <filesystem>
#include <string>

using namespace nlohmann;

extern ll::Logger logger;

inline ordered_json CreateJson(const std::string &path,
                               const std::string &defContent,
                               bool allowComment = true) {
  ordered_json jsonConf;
  if (!std::filesystem::exists(ll::string_utils::str2wstr(path))) {
    if (path.find('/') !=
        std::string::npos) { // e.g. plugins/LiteLoader/LiteLoader.json
      std::size_t pos = path.find_last_of('/');
      if (pos != std::string::npos) {
        std::string dirPath = path.substr(0, pos);
        CreateDirs(dirPath);
      }
    } else if (path.find('\\') !=
               std::string::npos) { // e.g. plugins\\LiteLoader\\LiteLoader.json
      std::size_t pos = path.find_last_of('\\');
      if (pos != std::string::npos) {
        std::string dirPath = path.substr(0, pos);
        CreateDirs(dirPath);
      }
    } else {
      logger.error("Fail in create json file!");
      logger.error("invalid path");
      jsonConf = ordered_json::object();
    }

    if (!defContent.empty()) {
      try {
        jsonConf = ordered_json::parse(defContent, nullptr, true, allowComment);
      } catch (std::exception &e) {
        logger.error("Fail to parse default json content!");
        logger.error(ll::string_utils::tou8str(e.what()));
        jsonConf = ordered_json::object();
      }
    } else {
      jsonConf = ordered_json::object();
    }

    std::ofstream jsonFile(path);
    if (jsonFile.is_open() && !defContent.empty())
      jsonFile << jsonConf.dump(4);
    jsonFile.close();
  } else {
    // 已存在
    auto jsonTexts = ReadAllFile(path);
    if (!jsonTexts) {
      jsonConf = ordered_json::object();
    } else {
      try {
        jsonConf = ordered_json::parse(*jsonTexts, nullptr, true, allowComment);
      } catch (std::exception &e) {
        logger.error("Fail to parse json content in file!");
        logger.error(ll::string_utils::tou8str(e.what()));
        jsonConf = ordered_json::object();
      }
    }
  }
  return jsonConf;
}