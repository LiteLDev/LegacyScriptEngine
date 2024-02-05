// Windows ComBase API
#include "utils/Utils.h"

#include "ll/api/utils/StringUtils.h"

#include <combaseapi.h>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>
#pragma comment(lib, "Ole32.lib")

std::vector<std::string> SplitCmdLine(const std::string& paras) {
    if (paras.empty()) return std::vector<std::string>();

    std::vector<std::string> res;
    std::string              now, strInQuote = "";
    std::istringstream       strIn(paras);
    while (strIn >> now) {
        if (!strInQuote.empty()) {
            strInQuote = strInQuote + " " + now;
            if (now.back() == '\"') {
                strInQuote.pop_back();
                res.push_back(strInQuote.erase(0, 1));
                strInQuote = "";
            }
        } else {
            if (now.front() == '\"') {
                if (now.back() == '\"') {
                    now = now.substr(1, now.size() - 2);
                    res.push_back(now);
                } else strInQuote = now;
            } else res.push_back(now);
        }
    }
    if (!strInQuote.empty()) {
        std::istringstream leftIn(strInQuote);
        while (leftIn >> now) res.push_back(now);
    }
    return res;
}

bool IsVersionLess(const std::string& v1, const std::string& v2) {
    auto vers1 = ll::string_utils::splitByPattern(v1, ".");
    auto vers2 = ll::string_utils::splitByPattern(v2, ".");
    return IsVersionLess(
        std::stoi(std::string(vers1[0])),
        std::stoi(std::string(vers1[1])),
        std::stoi(std::string(vers1[2])),
        std::stoi(std::string(vers2[0])),
        std::stoi(std::string(vers2[1])),
        std::stoi(std::string(vers2[2]))
    );
}

bool IsVersionLess(int v1a, int v1b, int v1c, int v2a, int v2b, int v2c) {
    return (v1a < v2a || (v1a == v2a && v1b < v2b) || (v1a == v2a && v1b == v2b && v1c < v2c));
}

wchar_t* str2cwstr(std::string str) {
    auto     len    = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    wchar_t* buffer = new wchar_t[len + 1];
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer, len + 1);
    buffer[len] = L'\0';
    return buffer;
}

unsigned long long GetCurrentTimeStampMS() {
    SYSTEMTIME currentTime = {0};
    GetLocalTime(&currentTime);

    struct tm currTm = {
        currentTime.wSecond,
        currentTime.wMinute,
        currentTime.wHour,
        currentTime.wDay,
        currentTime.wMonth - 1,
        currentTime.wYear - 1900
    };

    unsigned long long nNow = mktime(&currTm) * 1000 + currentTime.wMilliseconds;
    return nNow;
}

std::string Raw_GetDateTimeStr() {
    time_t t = time(NULL);
    tm     ts;
    localtime_s(&ts, &t);
    char buf[24] = {0};
    strftime(buf, 24, "%Y-%m-%d %H:%M:%S", &ts);
    return std::string(buf);
}

std::string Raw_RandomGuid() {
    GUID guid;
    CoCreateGuid(&guid);

    char dst[36] = {0};
    snprintf(
        dst,
        36,
        "%08lx%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
        guid.Data1,
        guid.Data2,
        guid.Data3,
        guid.Data4[0],
        guid.Data4[1],
        guid.Data4[2],
        guid.Data4[3],
        guid.Data4[4],
        guid.Data4[5],
        guid.Data4[6],
        guid.Data4[7]
    );
    return std::string(dst);
}

std::wstring Raw_RandomGuidW() {
    GUID guid;
    CoCreateGuid(&guid);

    wchar_t dst[36] = {0};
    _snwprintf_s(
        dst,
        36,
        L"%08x%04x%04x%02x%02x%02x%02x%02x%02x%02x%02x",
        guid.Data1,
        guid.Data2,
        guid.Data3,
        guid.Data4[0],
        guid.Data4[1],
        guid.Data4[2],
        guid.Data4[3],
        guid.Data4[4],
        guid.Data4[5],
        guid.Data4[6],
        guid.Data4[7]
    );
    return std::wstring(dst);
}
