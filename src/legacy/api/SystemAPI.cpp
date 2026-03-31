#include "legacy/api/SystemAPI.h"

#include "legacy/api/APIHelp.h"
#include "legacy/engine/EngineManager.h"
#include "legacy/engine/TimeTaskSystem.h"
#include "legacy/main/SafeGuardRecord.h"
#include "legacy/utils/Utils.h"
#include "ll/api/chrono/GameChrono.h"
#include "ll/api/coro/CoroTask.h"
#include "ll/api/service/GamingStatus.h"
#include "ll/api/thread/ThreadPoolExecutor.h"
#include "ll/api/utils/ErrorUtils.h"

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
    std::string const&                    process,
    std::function<void(int, std::string)> callback  = nullptr,
    int                                   timeLimit = -1
) {
    SECURITY_ATTRIBUTES sa;
    HANDLE              hRead = nullptr, hWrite = nullptr;
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = TRUE;

    if (!CreatePipe(&hRead, &hWrite, &sa, 0)) {
        if (hRead) CloseHandle(hRead);
        if (hWrite) CloseHandle(hWrite);
        return false;
    }

    STARTUPINFOW        si = {0};
    PROCESS_INFORMATION pi = {nullptr};
    si.cb                  = sizeof(STARTUPINFO);
    GetStartupInfoW(&si);
    si.hStdOutput = si.hStdError = hWrite;
    si.dwFlags                   = STARTF_USESTDHANDLES;

    auto wCmd = str2cwstr(process);
    if (!CreateProcessW(nullptr, wCmd.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(hRead);
        CloseHandle(hWrite);
        return false;
    }

    CloseHandle(hWrite);
    CloseHandle(pi.hThread);

    std::jthread([hRead{hRead}, hProcess{pi.hProcess}, callback{std::move(callback)}, timeLimit{timeLimit}]() mutable {
        if (timeLimit == -1) {
            WaitForSingleObject(hProcess, INFINITE);
        } else {
            WaitForSingleObject(hProcess, timeLimit);
            TerminateProcess(hProcess, 0);
        }

        char        buffer[8192];
        std::string strOutput;
        DWORD       bytesRead, exitCode;

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
            lse::LegacyScriptEngine::getLogger().error("NewProcess Callback Failed!");
            ll::utils::error_utils::printCurrentException(lse::LegacyScriptEngine::getLogger());
        }
    }).detach();

    return true;
}

Local<Value> SystemClass::cmd(Arguments const& args) {
    using namespace ll::chrono_literals;
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        std::string cmd = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "ExecuteSystemCommand", cmd);

        script::Global<Function> callbackFunc{args[1].asFunction()};

        return Boolean::newBoolean(NewProcess(
            "cmd /c" + cmd,
            [callback{std::move(callbackFunc)},
             engine{EngineScope::currentEngine()}](int exitCode, std::string output) mutable {
                ll::coro::keepThis(
                    [engine, callback = std::move(callback), exitCode, output = std::move(output)]()
                        -> ll::coro::CoroTask<> {
                        co_await 1_tick;
                        if ((ll::getGamingStatus() != ll::GamingStatus::Running)) co_return;
                        if (!EngineManager::isValid(engine)) co_return;

                        EngineScope scope(engine);
                        try {
                            NewTimeout(callback.get(), {Number::newNumber(exitCode), String::newString(output)}, 1);
                        }
                        CATCH_IN_CALLBACK("SystemCmd")
                    }
                ).launch(ll::thread::ThreadPoolExecutor::getDefault());
            },
            args.size() >= 3 ? args[2].asNumber().toInt32() : -1
        ));
    }
    CATCH_AND_THROW
}

Local<Value> SystemClass::newProcess(Arguments const& args) {
    using namespace ll::chrono_literals;
    CHECK_ARGS_COUNT(args, 2);
    CHECK_ARG_TYPE(args[0], ValueKind::kString);
    CHECK_ARG_TYPE(args[1], ValueKind::kFunction);
    if (args.size() >= 3) CHECK_ARG_TYPE(args[2], ValueKind::kNumber);

    try {
        std::string process = args[0].asString().toString();
        RecordOperation(getEngineOwnData()->pluginName, "CreateNewProcess", process);

        script::Global<Function> callbackFunc{args[1].asFunction()};

        return Boolean::newBoolean(NewProcess(
            process,
            [callback{std::move(callbackFunc)},
             engine{EngineScope::currentEngine()}](int exitCode, std::string output) mutable {
                ll::coro::keepThis(
                    [engine, callback = std::move(callback), exitCode, output = std::move(output)]()
                        -> ll::coro::CoroTask<> {
                        co_await 1_tick;
                        if ((ll::getGamingStatus() != ll::GamingStatus::Running)) co_return;
                        if (!EngineManager::isValid(engine)) co_return;

                        EngineScope scope(engine);
                        try {
                            NewTimeout(callback.get(), {Number::newNumber(exitCode), String::newString(output)}, 1);
                        }
                        CATCH_IN_CALLBACK("newProcess")
                    }
                ).launch(ll::thread::ThreadPoolExecutor::getDefault());
            },
            args.size() >= 3 ? args[2].asNumber().toInt32() : -1
        ));
    }
    CATCH_AND_THROW
}

Local<Value> SystemClass::getTimeStr(Arguments const&) {
    try {
        return String::newString(Raw_GetDateTimeStr());
    }
    CATCH_AND_THROW
}

Local<Value> SystemClass::getTimeObj(Arguments const&) {
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
    CATCH_AND_THROW
}

Local<Value> SystemClass::randomGuid(Arguments const&) { return String::newString(Raw_RandomGuid()); }
