#include "api/SystemAPI.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/TimeTaskSystem.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/schedule/Scheduler.h"
#include "ll/api/schedule/Task.h"
#include "ll/api/service/ServerInfo.h"
#include "ll/api/utils/ErrorUtils.h"
#include "ll/api/utils/StringUtils.h"
#include "main/SafeGuardRecord.h"
#include "utils/Utils.h"

#include <filesystem>
#include <fstream>

using namespace std::filesystem;

//////////////////// Classes ////////////////////

ClassDefine<void> SystemClassBuilder = defineClass("system")
                                           .function("getTimeStr", &SystemClass::getTimeStr)
                                           .function("getTimeObj", &SystemClass::getTimeObj)
                                           .function("randomGuid", &SystemClass::randomGuid)
                                           .function("cmd", &SystemClass::cmd)
                                           .function("newProcess", &SystemClass::newProcess)
                                           .build();

ll::schedule::ServerTimeScheduler systemScheduler;
// From LiteLoaderBDSv2 llapi/utils/WinHelper.cpp
bool NewProcess(
    const std::string&                    process,
    std::function<void(int, std::string)> callback  = nullptr,
    int                                   timeLimit = -1
) {
    SECURITY_ATTRIBUTES sa;
    HANDLE              hRead, hWrite;
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = TRUE;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) return false;
    STARTUPINFOW        si = {0};
    PROCESS_INFORMATION pi;

    si.cb = sizeof(STARTUPINFO);
    GetStartupInfoW(&si);
    si.hStdOutput = si.hStdError = hWrite;
    si.dwFlags                   = STARTF_USESTDHANDLES;

    auto wCmd = str2cwstr(process);
    if (!CreateProcessW(nullptr, wCmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        delete[] wCmd;
        return false;
    }
    CloseHandle(hWrite);
    CloseHandle(pi.hThread);

    std::thread([hRead{hRead}, hProcess{pi.hProcess}, callback{std::move(callback)}, timeLimit{timeLimit}, wCmd{wCmd}](
                ) {
#ifndef LSE_DEBUG
        _set_se_translator(ll::error_utils::translateSEHtoCE);
#endif
        if (timeLimit == -1) WaitForSingleObject(hProcess, INFINITE);
        else {
            WaitForSingleObject(hProcess, timeLimit);
            TerminateProcess(hProcess, -1);
        }
        char   buffer[8192];
        string strOutput;
        DWORD  bytesRead, exitCode;

        delete[] wCmd;
        GetExitCodeProcess(hProcess, &exitCode);
        while (true) {
            ZeroMemory(buffer, 8192);
            if (!ReadFile(hRead, buffer, 8192, &bytesRead, nullptr)) break;
            strOutput.append(buffer, bytesRead);
        }
        CloseHandle(hRead);
        CloseHandle(hProcess);

        try {
            if (callback) callback((int)exitCode, strOutput);
        } catch (const ll::error_utils::seh_exception& e) {
            lse::getSelfPluginInstance().getLogger().error(
                "SEH Uncaught Exception Detected!\n{}",
                ll::string_utils::tou8str(e.what())
            );
            lse::getSelfPluginInstance().getLogger().error("In NewProcess callback");
            //   PrintCurrentStackTraceback();
            ll::error_utils::printCurrentException(lse::getSelfPluginInstance().getLogger());
        } catch (...) {
            lse::getSelfPluginInstance().getLogger().error("NewProcess Callback Failed!");
            lse::getSelfPluginInstance().getLogger().error("Uncaught Exception Detected!");
            //   PrintCurrentStackTraceback();
            ll::error_utils::printCurrentException(lse::getSelfPluginInstance().getLogger());
        }
    }).detach();

    return true;
}

Local<Value> SystemClass::cmd(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        string cmd = args[0].toStr();
        RecordOperation(ENGINE_OWN_DATA()->pluginName, "ExecuteSystemCommand", cmd);

        script::Global<Function> callbackFunc{args[1].asFunction()};

        return Boolean::newBoolean(NewProcess(
            "cmd /c" + cmd,
            [callback{std::move(callbackFunc)}, engine{EngineScope::currentEngine()}](int exitCode, string output) {
                systemScheduler.add<ll::schedule::DelayTask>(
                    ll::chrono::ticks(1),
                    [engine, callback = std::move(callback), exitCode, output = std::move(output)]() {
                        if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                        if (!EngineManager::isValid(engine)) return;

                        EngineScope scope(engine);
                        try {
                            NewTimeout(callback.get(), {Number::newNumber(exitCode), String::newString(output)}, 1);
                        }
                        CATCH_IN_CALLBACK("SystemCmd")
                    }
                );
            },
            args.size() >= 3 ? args[2].toInt() : -1
        ));
    }
    CATCH("Fail in SystemCmd");
}

Local<Value> SystemClass::newProcess(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        string process = args[0].toStr();
        RecordOperation(ENGINE_OWN_DATA()->pluginName, "CreateNewProcess", process);

        script::Global<Function> callbackFunc{args[1].asFunction()};

        return Boolean::newBoolean(NewProcess(
            process,
            [callback{std::move(callbackFunc)}, engine{EngineScope::currentEngine()}](int exitCode, string output) {
                systemScheduler.add<ll::schedule::DelayTask>(
                    ll::chrono::ticks(1),
                    [engine, callback = std::move(callback), exitCode, output = std::move(output)]() {
                        if ((ll::getServerStatus() != ll::ServerStatus::Running)) return;
                        if (!EngineManager::isValid(engine)) return;

                        EngineScope scope(engine);
                        try {
                            NewTimeout(callback.get(), {Number::newNumber(exitCode), String::newString(output)}, 1);
                        }
                        CATCH_IN_CALLBACK("newProcess")
                    }
                );
            },
            args.size() >= 3 ? args[2].toInt() : -1
        ));
    }
    CATCH("Fail in newProcess");
}

Local<Value> SystemClass::getTimeStr(const Arguments& args) {
    try {
        return String::newString(Raw_GetDateTimeStr());
    }
    CATCH("Fail in GetTimeStr!")
}

Local<Value> SystemClass::getTimeObj(const Arguments& args) {
    try {
        SYSTEMTIME st;
        GetLocalTime(&st);
        Local<Object> res = Object::newObject();
        res.set("Y", Number::newNumber((int)st.wYear));
        res.set("M", Number::newNumber((int)st.wMonth));
        res.set("D", Number::newNumber((int)st.wDay));
        res.set("h", Number::newNumber((int)st.wHour));
        res.set("m", Number::newNumber((int)st.wMinute));
        res.set("s", Number::newNumber((int)st.wSecond));
        res.set("ms", Number::newNumber((int)st.wMilliseconds));
        return res;
    }
    CATCH("Fail in GetTimeNow!")
}

Local<Value> SystemClass::randomGuid(const Arguments& args) { return String::newString(Raw_RandomGuid()); }
