#pragma once

///////////////////////// Configs /////////////////////////

#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS)
// QuickJs
#define LLSE_BACKEND_TYPE "Js"

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_LUA)
// Lua
#define LLSE_BACKEND_TYPE "Lua"

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
// NodeJs
#define LLSE_BACKEND_TYPE "NodeJs"

#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
// Python
#define LLSE_BACKEND_TYPE "Python"
#endif

// All backends information
#define LLSE_MODULE_TYPE          LLSE_BACKEND_TYPE
#define LLSE_VALID_BACKENDS_COUNT 4

// Debug engine information
#if defined(LEGACY_SCRIPT_ENGINE_BACKEND_NODEJS)
#define LLSE_DEBUG_CMD "nodejsdebug"
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_QUICKJS)
#define LLSE_DEBUG_CMD "jsdebug"
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_LUA)
#define LLSE_DEBUG_CMD "luadebug"
#elif defined(LEGACY_SCRIPT_ENGINE_BACKEND_PYTHON)
#define LLSE_DEBUG_CMD "pydebug"
#endif
// Global communication
#define LLSE_GLOBAL_DATA_NAME                   L"LLSE_GLOBAL_DATA_SECTION"
#define LLSE_MESSAGE_SYSTEM_WAIT_CHECK_INTERVAL 5

// Thread pool parameter
#define LLSE_POOL_THREAD_COUNT 4