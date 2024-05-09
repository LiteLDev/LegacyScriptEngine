#pragma warning(disable : 4251)
#include "Configs.h"
#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
#include "Global.h"
#include "PythonHelper.h"
#include "engine/EngineManager.h"
#include "engine/EngineOwnData.h"
#include "engine/RemoteCall.h"
#include "legacy/api/CommandAPI.h"
#include "legacy/api/CommandCompatibleAPI.h"
#include "legacy/api/EventAPI.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/Entry.h"
#include "utils/Utils.h"

#include <Python.h>
#include <detours.h>
#include <engine/TimeTaskSystem.h>
#include <filesystem>
#include <toml.h>

#define PIP_EXECUTE_TIMEOUT 1800 * 1000

// pre-declare
extern void          BindAPIs(ScriptEngine* engine);
extern bool          isInConsoleDebugMode;
extern ScriptEngine* debugEngine;

struct PyConfig;
typedef PyObject* (*create_stdio_func_type)(
    const PyConfig* config,
    PyObject*       io,
    int             fd,
    int             write_mode,
    const char*     name,
    const wchar_t*  encoding,
    const wchar_t*  errors
);

namespace PythonHelper {

bool pythonInited = false;

bool initPythonRuntime() {
    if (!pythonInited) {
        script::py_interop::setPythonHomePath(lse::getSelfPluginInstance().getPluginDir());
        script::py_interop::setModuleSearchPaths({
            lse::getSelfPluginInstance().getPluginDir() / "python310.zip",
            lse::getSelfPluginInstance().getPluginDir() / "DLLs",
            lse::getSelfPluginInstance().getPluginDir() / "Lib",
            lse::getSelfPluginInstance().getPluginDir() / "site-packages",
        });
        pythonInited = true;
    }
    return true;
}

bool loadPluginCode(script::ScriptEngine* engine, std::string entryScriptPath, std::string pluginDirPath) {
    // TODO: add import path to sys.path
    try {
        engine->loadFile(String::newString(entryScriptPath));
    } catch (const Exception& e1) {
        // Fail
        lse::getSelfPluginInstance().getLogger().error("Fail in Loading Script Plugin!\n");
        throw e1;
    }
    return true;
}

std::string findEntryScript(const std::string& dirPath) {
    auto dirPath_obj = std::filesystem::path(dirPath);

    std::filesystem::path entryFilePath = dirPath_obj / "__init__.py";
    if (!std::filesystem::exists(entryFilePath)) return "";
    else return ll::string_utils::u8str2str(entryFilePath.u8string());
}

std::string getPluginPackageName(const std::string& dirPath) {
    auto        dirPath_obj       = std::filesystem::path(dirPath);
    std::string defaultReturnName = ll::string_utils::u8str2str(std::filesystem::path(dirPath).filename().u8string());

    std::filesystem::path packageFilePath = dirPath_obj / std::filesystem::path("pyproject.toml");
    if (!std::filesystem::exists(packageFilePath)) return defaultReturnName;

    try {
        string      packageFilePathStr = ll::string_utils::u8str2str(packageFilePath.make_preferred().u8string());
        toml::table configData         = toml::parse_file(packageFilePathStr);
        auto        projectNode        = configData["project"];
        if (!projectNode["name"]) return defaultReturnName;
        std::optional<std::string> packageName = projectNode["name"].value<std::string>();
        if (packageName && !packageName->empty()) return *packageName;
        else return defaultReturnName;
    } catch (...) {
        return defaultReturnName;
    }
}

std::string getPluginPackDependencyFilePath(const std::string& dirPath) {
    auto                  dirPath_obj             = std::filesystem::path(dirPath);
    std::filesystem::path packageFilePath         = dirPath_obj / std::filesystem::path("pyproject.toml");
    std::filesystem::path requirementsFilePath    = dirPath_obj / std::filesystem::path("requirements.txt");
    std::filesystem::path requirementsTmpFilePath = dirPath_obj / std::filesystem::path("_requirements_llse_temp.txt");

    // if requirements.txt exists, copy a temp version
    if (std::filesystem::exists(requirementsFilePath)) {
        std::error_code ec;
        std::filesystem::copy_file(requirementsFilePath, requirementsTmpFilePath, ec);
    }

    if (std::filesystem::exists(packageFilePath)) {
        // copy dependencies from pyproject.toml to _requirements_llse_temp.txt
        std::string dependsAdded = "";
        try {
            string      packageFilePathStr = ll::string_utils::u8str2str(packageFilePath.make_preferred().u8string());
            toml::table configData         = toml::parse_file(packageFilePathStr);
            auto        projectNode        = configData["project"];
            if (projectNode["dependencies"]) {
                toml::array* arr = projectNode["dependencies"].as_array();
                arr->for_each([&dependsAdded](toml::value<std::string>& elem) {
                    std::optional<std::string> depend  = *elem;
                    dependsAdded                      += "\n" + *depend;
                });
            }
        } catch (...) {}

        if (!dependsAdded.empty()) {
            std::ofstream fout(
                ll::string_utils::u8str2str(requirementsTmpFilePath.make_preferred().u8string()),
                std::ios::app
            );
            fout << dependsAdded;
            fout.close();
        }
    }

    if (std::filesystem::exists(requirementsTmpFilePath))
        return ll::string_utils::u8str2str(requirementsTmpFilePath.make_preferred().u8string());
    else return "";
}

#define OUTPUT_DEBUG_SIGN()                std::cout << ">>> " << std::flush
#define OUTPUT_DEBUG_NEED_MORE_CODE_SIGN() std::cout << "... " << std::flush
std::string codeBuffer        = "";
bool        isInsideCodeBlock = false;

static PyObject* getPyGlobalDict() {
    PyObject* m = PyImport_AddModule("__main__");
    if (m == nullptr) {
        throw Exception("can't find __main__ module");
    }
    return PyModule_GetDict(m);
}

bool processPythonDebugEngine(const std::string& cmd) {
    if (cmd == LLSE_DEBUG_CMD) {
        if (isInConsoleDebugMode) {
            // EndDebug
            lse::getSelfPluginInstance().getLogger().info("Debug mode ended");
            isInConsoleDebugMode = false;
        } else {
            // StartDebug
            lse::getSelfPluginInstance().getLogger().info("Debug mode begins");
            codeBuffer.clear();
            isInsideCodeBlock    = false;
            isInConsoleDebugMode = true;
            OUTPUT_DEBUG_SIGN();
        }
        return false;
    }
    if (isInConsoleDebugMode) {
        EngineScope enter(debugEngine);
        if (cmd == "stop") {
            return true;
        } else {
            try {
                if (isInsideCodeBlock) {
                    // is in code block mode
                    if (cmd.empty()) {
                        // exit code block
                        isInsideCodeBlock = false;
                    } else {
                        // add a new line to buffer
                        codeBuffer += cmd + "\n";
                        OUTPUT_DEBUG_NEED_MORE_CODE_SIGN();
                        return false;
                    }
                } else {
                    // not in code block mode
                    if (cmd.ends_with(':')) {
                        // begin code block mode
                        isInsideCodeBlock = true;
                        codeBuffer        = cmd + "\n";
                        OUTPUT_DEBUG_NEED_MORE_CODE_SIGN();
                        return false;
                    } else {
                        codeBuffer = cmd;
                    }
                }

                PyRun_StringFlags(codeBuffer.c_str(), Py_single_input, getPyGlobalDict(), nullptr, nullptr);
                codeBuffer.clear();
                if (script::py_interop::hasException()) {
                    auto exp = script::py_interop::getAndClearLastException();
                    throw exp;
                }
            } catch (const Exception& e) {
                isInsideCodeBlock = false;
                codeBuffer.clear();
                lse::getSelfPluginInstance().getLogger().error("Exception:\n" + e.stacktrace() + "\n" + e.message());
            }
        }
        OUTPUT_DEBUG_SIGN();
        return false;
    }
    return true;
}

bool processConsolePipCmd(const std::string& cmd) {
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    if (ll::string_utils::splitByPattern(cmd, " ")[0] == "pip") {
        PythonHelper::executePipCommand(cmd);
        return false;
    } else return true;
#else
    return true;
#endif
}

// if no -t in cmd, packages will install to default global embedding
// site-package dir
// (./plugins/legacy-script-engine/lib/python-env/Lib/site-packages)
int executePipCommand(std::string cmd) {
    if (cmd.find("--disable-pip-version-check") == std::string::npos) cmd += " --disable-pip-version-check";
    cmd = ll::string_utils::u8str2str((lse::getSelfPluginInstance().getPluginDir() / "python.exe").u8string()) + " -m"
        + cmd;

    SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = TRUE;

    STARTUPINFOW        si = {0};
    PROCESS_INFORMATION pi;
    si.cb = sizeof(STARTUPINFO);
    GetStartupInfoW(&si);

    auto wCmd = str2cwstr(cmd);
    if (!CreateProcessW(nullptr, wCmd, nullptr, nullptr, TRUE, 0, nullptr, nullptr, &si, &pi)) {
        delete[] wCmd;
        return -1;
    }
    CloseHandle(pi.hThread);

    if (WaitForSingleObject(pi.hProcess, PIP_EXECUTE_TIMEOUT) == WAIT_TIMEOUT) TerminateProcess(pi.hProcess, -1);

    DWORD exitCode = 0;
    GetExitCodeProcess(pi.hProcess, &exitCode);
    CloseHandle(pi.hProcess);
    delete[] wCmd;
    return exitCode;
}

// This fix is used for Python3.10's bug:
// The thread will freeze when creating a new engine while another thread is
// blocking to read stdin Side effects: sys.stdin cannot be used after this
// patch. More info to see: https://github.com/python/cpython/issues/83526
//
// Attention! When CPython is upgraded, this fix must be re-adapted or removed!!
//
namespace FixPython310Stdin {
// Hard coded function address
const uintptr_t create_stdio_func_base_offset = 0xCE0F4;

create_stdio_func_type create_stdio_original = nullptr;

PyObject* create_stdio_hooked(
    const PyConfig* config,
    PyObject*       io,
    int             fd,
    int             write_mode,
    const char*     name,
    const wchar_t*  encoding,
    const wchar_t*  errors
) {
    if (fd == 0) {
        Py_RETURN_NONE;
    }
    return create_stdio_original(config, io, fd, write_mode, name, encoding, errors);
}

bool patchPython310CreateStdio() {
    if (create_stdio_original == nullptr) {
        HMODULE hModule = GetModuleHandleW(L"python310.dll");
        if (hModule == NULL) return false;
        create_stdio_original = (create_stdio_func_type)(void*)(((uintptr_t)hModule) + create_stdio_func_base_offset);
    }

    DetourRestoreAfterWith();
    if (DetourTransactionBegin() != NO_ERROR) return false;
    else if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) return false;
    else if (DetourAttach((PVOID*)&create_stdio_original, create_stdio_hooked) != NO_ERROR) return false;
    else if (DetourTransactionCommit() != NO_ERROR) return false;
    return true;
}

bool unpatchPython310CreateStdio() {
    if (DetourTransactionBegin() != NO_ERROR) return false;
    else if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) return false;
    else if (DetourDetach((PVOID*)&create_stdio_original, create_stdio_hooked) != NO_ERROR) return false;
    else if (DetourTransactionCommit() != NO_ERROR) return false;
    return true;
}

} // namespace FixPython310Stdin

} // namespace PythonHelper

#endif
