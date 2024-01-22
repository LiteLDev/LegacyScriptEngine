#include "FileHelper.h"

#include "ll/api/utils/StringUtils.h"
#include "ll/api/utils/WinUtils.h"

#include <filesystem>
#include <io.h>

extern ll::Logger logger;

std::optional<std::string> ReadAllFile(const std::string& filePath, bool isBinary) {
    std::ifstream fRead;

    std::ios_base::openmode mode = std::ios_base::in;
    if (isBinary) mode |= std::ios_base::binary;

    fRead.open(ll::string_utils::str2wstr(filePath), mode);
    if (!fRead.is_open()) {
        return std::nullopt;
    }
    std::string data((std::istreambuf_iterator<char>(fRead)), std::istreambuf_iterator<char>());
    fRead.close();
    return data;
}

bool WriteAllFile(const std::string& filePath, const std::string& content, bool isBinary) {
    std::ofstream fWrite;

    std::ios_base::openmode mode = std::ios_base::out;
    if (isBinary) mode |= std::ios_base::binary;

    fWrite.open(ll::string_utils::str2wstr(filePath), mode);
    if (!fWrite.is_open()) {
        return false;
    }
    fWrite << content;
    fWrite.close();
    return true;
}

std::vector<std::string> GetFileNameList(const std::string& dir) {
    std::filesystem::directory_entry d(dir);
    if (!d.is_directory()) return {};

    std::vector<std::string>            list;
    std::filesystem::directory_iterator deps(d);
    for (auto& i : deps) {
        list.push_back(ll::string_utils::u8str2str(i.path().filename().u8string()));
    }
    return list;
}

bool CreateDirs(const std::string path) {
    std::error_code ec;
    auto ret = std::filesystem::create_directories(std::filesystem::path(ll::string_utils::str2wstr(path)), ec);
    if (ec.value() != 0) {
        logger.error("Fail to create dir, err code: {}", ec.value());
        logger.error(ec.message());
    }
    return ret;
}

// From LiteLoaderBDSv2 llapi/utils/WinHelper.cpp
wchar_t* str2cwstr(const std::string& str) {
    auto  len    = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
    auto* buffer = new wchar_t[len + 1];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, len + 1);
    buffer[len] = L'\0';
    return buffer;
}

std::pair<int, std::string> NewProcessSync(const std::string& process, int timeLimit = -1, bool noReadOutput = true) {
    SECURITY_ATTRIBUTES sa;
    HANDLE              hRead, hWrite;
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = TRUE;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return {-1, ""};
    STARTUPINFOW        si = {0};
    PROCESS_INFORMATION pi;

    si.cb = sizeof(STARTUPINFO);
    GetStartupInfoW(&si);
    si.hStdOutput = si.hStdError = hWrite;
    si.dwFlags                   = STARTF_USESTDHANDLES;

    auto wCmd = str2cwstr(process);
    if (!CreateProcessW(nullptr, wCmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        delete[] wCmd;
        return {-1, ""};
    }
    CloseHandle(hWrite);
    CloseHandle(pi.hThread);

    if (timeLimit == -1) WaitForSingleObject(pi.hProcess, INFINITE);
    else {
        WaitForSingleObject(pi.hProcess, timeLimit);
        TerminateProcess(pi.hProcess, -1);
    }
    char        buffer[4096];
    std::string strOutput;
    DWORD       bytesRead, exitCode;

    delete[] wCmd;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (!noReadOutput) {
        while (true) {
            ZeroMemory(buffer, 4096);
            if (!ReadFile(hRead, buffer, 4096, &bytesRead, nullptr)) break;
            strOutput.append(buffer, bytesRead);
        }
    }
    CloseHandle(hRead);
    CloseHandle(pi.hProcess);
    return {exitCode, strOutput};
}

std::pair<int, std::string> UncompressFile(const std::string& filePath, const std::string& toDir, int processTimeout) {
    std::error_code ec;
    std::filesystem::create_directories(toDir, ec);
    std::string realToDir     = toDir.ends_with('/') ? toDir : toDir + "/";
    auto&& [exitCode, output] = NewProcessSync(
        fmt::format(R"({} x "{}" -o"{}" -aoa)", "./plugins/LegacyScriptEngine/7z/7za.exe", filePath, realToDir),
        processTimeout
    );
    return {exitCode, std::move(output)};
}
