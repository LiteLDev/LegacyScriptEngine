#include "legacy/engine/GlobalShareData.h"

#include "legacy/api/APIHelp.h"
#include "legacy/engine/LocalShareData.h"

#include <Windows.h>
#include <atomic>
#include <string>
#include <vector>

// 全局共享数据
GlobalDataType* globalShareData = nullptr;

void InitGlobalShareData() {
    HANDLE hGlobalData = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        nullptr,
        PAGE_READWRITE,
        0,
        sizeof(GlobalDataType),
        (LLSE_GLOBAL_DATA_NAME + std::to_wstring(GetCurrentProcessId())).c_str()
    );
    if (hGlobalData == nullptr) {
        lse::LegacyScriptEngine::getLogger().error("Failed to initialize file mapping"_tr());
        localShareData->isFirstInstance = true;
        return;
    }

    LPVOID address = MapViewOfFile(hGlobalData, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    if (address == nullptr) {
        lse::LegacyScriptEngine::getLogger().error("Failed to initialize map file"_tr());
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
        globalShareData                 = static_cast<GlobalDataType*>(address);
    }

    globalShareData->globalEngineSnapshot.store(
        std::make_shared<std::vector<std::shared_ptr<ScriptEngine>>>(
            globalShareData->globalEngineList.begin(),
            globalShareData->globalEngineList.end()
        ),
        std::memory_order_release
    );
}
