#include "main/Global.h"

// 全局变量
bool isCmdRegisterEnabled = false;
#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS)
// QuickJs
const std::string LLSE_BACKEND_TYPE = "Js";

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_LUA)
// Lua
const std::string LLSE_BACKEND_TYPE = "Lua";

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
// NodeJs
const std::string LLSE_BACKEND_TYPE = "NodeJs";

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
// Python
const std::string LLSE_BACKEND_TYPE = "Python";
#endif

const int LLSE_VALID_BACKENDS_COUNT = 4;

// Debug engine information
#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
const std::string LLSE_DEBUG_CMD = "nodejsdebug";
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS)
const std::string LLSE_DEBUG_CMD = "jsdebug";
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_LUA)
const std::string LLSE_DEBUG_CMD = "luadebug";
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
const std::string LLSE_DEBUG_CMD = "pydebug";
#endif

const wchar_t* LLSE_GLOBAL_DATA_NAME = L"LLSE_GLOBAL_DATA_SECTION";