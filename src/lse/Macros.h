#pragma once

#ifndef LEGACY_SCRIPT_ENGINE_BACKEND

#error "LEGACY_SCRIPT_ENGINE_BACKEND is not defined!"

#endif

#if LEGACY_SCRIPT_ENGINE_BACKEND == lua

#define LLSE_BACKEND_LUA

#elif LEGACY_SCRIPT_ENGINE_BACKEND == quickjs

#define LLSE_BACKEND_QUICKJS

#endif
