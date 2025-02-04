#include "engine/GlobalShareData.h"

#include "api/APIHelp.h"
#include "engine/LocalShareData.h"

#include <Windows.h>
#include <cstdlib>
#include <ctime>
#include <map>
#include <string>
#include <vector>

// 全局共享数据
GlobalDataType* globalShareData;

void InitGlobalShareData() {
    HANDLE hGlobalData = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        sizeof(GlobalDataType),
        (LLSE_GLOBAL_DATA_NAME + std::to_wstring(GetCurrentProcessId())).c_str()
    );
    if (hGlobalData == NULL) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Failed to initialize file mapping"_tr());
        localShareData->isFirstInstance = true;
        return;
    }

    LPVOID address = MapViewOfFile(hGlobalData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (address == NULL) {
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Failed to initialize map file"_tr());
        localShareData->isFirstInstance = true;
        return;
    }

    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        // First Time
        localShareData->isFirstInstance = true;
        globalShareData                 = new (address) GlobalDataType;
    } else {
        // Existing
        localShareData->isFirstInstance = false;
        globalShareData                 = (GlobalDataType*)address;
    }
}
