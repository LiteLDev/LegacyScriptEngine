#include "FileHelper.h"

#include "ll/api/utils/StringUtils.h"
#include "ll/api/utils/WinUtils.h"
#include "lse/Entry.h"

#include <filesystem>
#include <io.h>

namespace lse::legacy {

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

    auto wCmd = ll::string_utils::str2wstr(process);
    if (!CreateProcessW(
            nullptr,
            const_cast<wchar_t*>(wCmd.c_str()),
            nullptr,
            nullptr,
            TRUE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi
        )) {
        return {-1, ""};
    }
    CloseHandle(hWrite);
    CloseHandle(pi.hThread);

    if (timeLimit == -1) WaitForSingleObject(pi.hProcess, INFINITE);
    else {
        WaitForSingleObject(pi.hProcess, timeLimit);
        TerminateProcess(pi.hProcess, -1);
    }
    char        buffer[8192];
    std::string strOutput;
    DWORD       bytesRead, exitCode;

    GetExitCodeProcess(pi.hProcess, &exitCode);
    if (!noReadOutput) {
        while (true) {
            ZeroMemory(buffer, 8192);
            if (!ReadFile(hRead, buffer, 8192, &bytesRead, nullptr)) break;
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
    std::string realToDir = toDir.ends_with('/') ? toDir : toDir + "/";
    auto&& [exitCode, output] =
        NewProcessSync(fmt::format(R"({} x "{}" -o"{}" -aoa)", "7za.exe", filePath, realToDir), processTimeout);
    return {exitCode, std::move(output)};
}
} // namespace lse::legacy
