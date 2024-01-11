#include "FileHelper.h"

#include <io.h>

#include <filesystem>

#include "ll/api/utils/StringUtils.h"
#include "ll/api/utils/WinUtils.h"

ll::Logger logger("FileHelper");

std::optional<std::string> ReadAllFile(const std::string &filePath,
                                       bool isBinary) {
  std::ifstream fRead;

  std::ios_base::openmode mode = std::ios_base::in;
  if (isBinary)
    mode |= std::ios_base::binary;

  fRead.open(ll::string_utils::str2wstr(filePath), mode);
  if (!fRead.is_open()) {
    return std::nullopt;
  }
  std::string data((std::istreambuf_iterator<char>(fRead)),
                   std::istreambuf_iterator<char>());
  fRead.close();
  return data;
}

bool WriteAllFile(const std::string &filePath, const std::string &content,
                  bool isBinary) {
  std::ofstream fWrite;

  std::ios_base::openmode mode = std::ios_base::out;
  if (isBinary)
    mode |= std::ios_base::binary;

  fWrite.open(ll::string_utils::str2wstr(filePath), mode);
  if (!fWrite.is_open()) {
    return false;
  }
  fWrite << content;
  fWrite.close();
  return true;
}

std::vector<std::string> GetFileNameList(const std::string &dir) {
  std::filesystem::directory_entry d(dir);
  if (!d.is_directory())
    return {};

  std::vector<std::string> list;
  std::filesystem::directory_iterator deps(d);
  for (auto &i : deps) {
    list.push_back(ll::string_utils::u8str2str(i.path().filename().u8string()));
  }
  return list;
}

bool CreateDirs(const std::string path) {
  std::error_code ec;
  auto ret = std::filesystem::create_directories(
      std::filesystem::path(ll::string_utils::str2wstr(path)), ec);
  if (ec.value() != 0) {
    logger.error("Fail to create dir, err code: {}", ec.value());
    logger.error(ec.message());
  }
  return ret;
}

std::pair<int, std::string> UncompressFile(const std::string &filePath,
                                           const std::string &toDir,
                                           int processTimeout) {
  std::error_code ec;
  std::filesystem::create_directories(toDir, ec);
  std::string realToDir = EtoDir.ends_with('/') ? toDir : toDir + "/";
  auto &&[exitCode, output] =
      NewProcessSync(fmt::format(R"({} x "{}" -o"{}" -aoa)", ZIP_PROGRAM_PATH,
                                 filePath, realToDir),
                     processTimeout);
  return {exitCode, std::move(output)};
}