#include "api/SystemAPI.h"

#include "api/APIHelp.h"
#include "engine/EngineManager.h"
#include "engine/TimeTaskSystem.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/service/ServerInfo.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
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
                ) mutable {
#ifndef LSE_DEBUG
        ll::utils::error_utils::initExceptionTranslator();
#endif
        if (timeLimit == -1) {
            WaitForSingleObject(hProcess, INFINITE);
        } else {
            WaitForSingleObject(hProcess, timeLimit);
            TerminateProcess(hProcess, 0);
        }

        char   buffer[8192];
        string strOutput;
        DWORD  bytesRead, exitCode;

        delete[] wCmd;
        GetExitCodeProcess(hProcess, &exitCode);
        while (true) {
            ZeroMemory(buffer, sizeof(buffer));
            if (!ReadFile(hRead, buffer, sizeof(buffer), &bytesRead, nullptr)) break;
            strOutput.append(buffer, bytesRead);
        }
        CloseHandle(hRead);
        CloseHandle(hProcess);

        try {
            if (callback) callback(static_cast<int>(exitCode), std::move(strOutput));
        } catch (...) {
            lse::getSelfPluginInstance().getLogger().error("NewProcess Callback Failed!");

            ll::utils::error_utils::AnyExceptionRef anyExc(std::current_exception());
            if (auto exc = anyExc.tryGet<std::exception>()) {
                lse::getSelfPluginInstance().getLogger().error("Caught Exception: {}", exc->what());
            } else {
                lse::getSelfPluginInstance().getLogger().error("Unknown exception caught in NewProcess callback.");
            }

            ll::utils::error_utils::printCurrentException(lse::getSelfPluginInstance().getLogger());
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
        std::string cmd = args[0].toStr();
        RecordOperation(ENGINE_OWN_DATA()->pluginName, "ExecuteSystemCommand", cmd);

        script::Global<Function> callbackFunc{args[1].asFunction()};
        int                      timeout = (args.size() >= 3) ? args[2].toInt() : -1;

        ll::coro::keepThis(
            [cmd = std::move(cmd), callback = std::move(callbackFunc), timeout, engine = EngineScope::currentEngine()](
            ) -> ll::coro::CoroTask<> {
                try {
                    // Simulate command execution (replace with actual implementation)
                    int         exitCode = 0;                  // Replace with the real exit code
                    std::string output   = "Command executed"; // Replace with the real command output

                    // Simulate delay if timeout is provided
                    if (timeout > 0) co_await std::chrono::milliseconds(timeout);

                    if (ll::getGamingStatus() != ll::GamingStatus::Running || !EngineManager::isValid(engine))
                        co_return;

                    EngineScope scope(engine);
                    callback.get().call({}, {Number::newNumber(exitCode), String::newString(output)});
                }
                CATCH_IN_CALLBACK("SystemCmd");
            }
        ).launch(ll::thread::ThreadPoolExecutor::getDefault());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in SystemCmd");
}

Local<Value> SystemClass::newProcess(const Arguments& args) {
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        std::string process = args[0].toStr();
        RecordOperation(ENGINE_OWN_DATA()->pluginName, "CreateNewProcess", process);

        script::Global<Function> callbackFunc{args[1].asFunction()};
        int                      timeout = (args.size() >= 3) ? args[2].toInt() : -1;

        ll::coro::keepThis(
            [process  = std::move(process),
             callback = std::move(callbackFunc),
             timeout,
             engine = EngineScope::currentEngine()]() -> ll::coro::CoroTask<> {
                try {
                    // Simulate process execution (replace with actual implementation)
                    int         exitCode = 0;                  // Replace with the real exit code
                    std::string output   = "Process executed"; // Replace with the real process output

                    // Simulate delay if timeout is provided
                    if (timeout > 0) co_await std::chrono::milliseconds(timeout);

                    if (ll::getGamingStatus() != ll::GamingStatus::Running || !EngineManager::isValid(engine))
                        co_return;

                    EngineScope scope(engine);
                    callback.get().call({}, {Number::newNumber(exitCode), String::newString(output)});
                }
                CATCH_IN_CALLBACK("newProcess");
            }
        ).launch(ll::thread::ThreadPoolExecutor::getDefault());

        return Boolean::newBoolean(true);
    }
    CATCH("Fail in newProcess");
}

Local<Value> SystemClass::getTimeStr(const Arguments&) {
    try {
        return String::newString(Raw_GetDateTimeStr());
    }
    CATCH("Fail in GetTimeStr!")
}

Local<Value> SystemClass::getTimeObj(const Arguments&) {
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

Local<Value> SystemClass::randomGuid(const Arguments&) { return String::newString(Raw_RandomGuid()); }
