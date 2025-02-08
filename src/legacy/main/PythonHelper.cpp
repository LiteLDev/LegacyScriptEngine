#pragma warning(disable : 4251)
#include "PythonHelper.h"

#include "Global.h"
#include "engine/EngineManager.h"
#include "engine/RemoteCall.h"
#include "engine/TimeTaskSystem.h"
#include "legacy/api/CommandAPI.h"
#include "ll/api/utils/StringUtils.h"
#include "lse/Entry.h"
#include "utils/Utils.h"

#include <Python.h>
#include <filesystem>
#include <toml++/toml.h>

const unsigned long PIP_EXECUTE_TIMEOUT = 1800 * 1000;

// pre-declare
extern void                            BindAPIs(ScriptEngine* engine);
extern bool                            InConsoleDebugMode;
extern ScriptEngine*                   DebugEngine;
extern std::shared_ptr<ll::io::Logger> DebugCmdLogger;

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
        script::py_interop::setPythonHomePath(lse::LegacyScriptEngine::getInstance().getSelf().getModDir());
        const char*               pathEnv = std::getenv("PATH");
        auto                      paths   = ll::string_utils::splitByPattern(pathEnv, ";");
        std::vector<std::wstring> modulePaths;
        modulePaths.push_back(lse::LegacyScriptEngine::getInstance().getSelf().getModDir() / "lib");
        modulePaths.push_back(lse::LegacyScriptEngine::getInstance().getSelf().getModDir() / "DLLs");
        modulePaths.push_back(lse::LegacyScriptEngine::getInstance().getSelf().getModDir() / "site-packages");
        for (const auto& p : paths) {
            if (p.find("Python") != std::string::npos) {
                std::wstring wstr = ll::string_utils::str2wstr(p);
                modulePaths.push_back(wstr + L"\\DLLs");
                modulePaths.push_back(wstr + L"\\Lib");
                modulePaths.push_back(wstr + L"\\site-packages");
            }
        }
        script::py_interop::setModuleSearchPaths(modulePaths);
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
        lse::LegacyScriptEngine::getInstance().getSelf().getLogger().error("Fail in Loading Script Plugin!\n");
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

inline void PrintPyDebugSign() { DebugCmdLogger->info(">>> "); }
inline void OutputDebugNeedMoreCodeSign() { DebugCmdLogger->info("... "); }
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
    auto& logger = lse::LegacyScriptEngine::getInstance().getSelf().getLogger();
    if (cmd == LLSE_DEBUG_CMD) {
        if (InConsoleDebugMode) {
            // EndDebug
            logger.info("Debug mode ended");
            InConsoleDebugMode = false;
        } else {
            // StartDebug
            logger.info("Debug mode begins");
            codeBuffer.clear();
            isInsideCodeBlock  = false;
            InConsoleDebugMode = true;
            PrintPyDebugSign();
        }
        return false;
    }
    if (InConsoleDebugMode) {
        EngineScope enter(DebugEngine);
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
                        OutputDebugNeedMoreCodeSign();
                        return false;
                    }
                } else {
                    // not in code block mode
                    if (cmd.ends_with(':')) {
                        // begin code block mode
                        isInsideCodeBlock = true;
                        codeBuffer        = cmd + "\n";
                        OutputDebugNeedMoreCodeSign();
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
            } catch (...) {
                isInsideCodeBlock = false;
                codeBuffer.clear();
                ll::error_utils::printCurrentException(logger);
            }
        }
        PrintPyDebugSign();
        return false;
    }
    return true;
}

bool processConsolePipCmd(const std::string& cmd) {
#ifdef LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON
    if (cmd == "pip" || cmd.starts_with("pip ")) {
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
    cmd = ll::string_utils::u8str2str(
              (lse::LegacyScriptEngine::getInstance().getSelf().getModDir() / "python.exe").u8string()
          )
        + " -m" + cmd;

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

} // namespace PythonHelper
