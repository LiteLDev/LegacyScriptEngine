#include "LegacyScriptEngine.h"

#include "../api/EventAPI.h"
#include "../api/MoreGlobal.h"
#include "../engine/GlobalShareData.h"
#include "../engine/LocalShareData.h"
#include "../engine/MessageSystem.h"
#include "../main/BuiltinCommands.h"
#include "../main/SafeGuardRecord.h"

#include <ll/api/plugin/NativePlugin.h>

#ifdef LLSE_BACKEND_PYTHON
#include "../main/PythonHelper.h"
#endif

ll::Logger logger(LLSE_LOADER_NAME);

extern void LoadDepends();
extern void LoadMain();
extern void BindAPIs(ScriptEngine* engine);
extern void LoadDebugEngine();

namespace lse {

LegacyScriptEngine::LegacyScriptEngine(ll::plugin::NativePlugin& self) : mSelf(self) {
    mSelf.getLogger().info("loading...");

    // Code for loading the plugin goes here.
    // Load i18n files
    ll::i18n::load(u8"plugins/LeviLamina/lang");

    // Init global share data
    InitLocalShareData();
    InitGlobalShareData();
    InitSafeGuardRecord();

    EconomySystem::init();

#ifdef LLSE_BACKEND_PYTHON
    // This fix is used for Python3.10's bug:
    // The thread will freeze when creating a new engine while another thread is
    // blocking to read stdin Side effects: sys.stdin cannot be used after this
    // patch. More info to see: https://github.com/python/cpython/issues/83526
    //
    // Attention! When CPython is upgraded, this fix must be re-adapted or
    // removed!!
    //
    PythonHelper::FixPython310Stdin::patchPython310CreateStdio();

    PythonHelper::initPythonRuntime();
#endif

    // Pre-load depending libs
    LoadDepends();

    // Load plugins
    LoadMain();

    // Register real-time debug
    LoadDebugEngine();

    // Register basic event listeners
    InitBasicEventListeners();

    // Init message system
    InitMessageSystem();

    MoreGlobal::Init();
}

ll::plugin::NativePlugin& LegacyScriptEngine::getSelf() const { return mSelf; }

bool LegacyScriptEngine::enable() {
    mSelf.getLogger().info("enabling...");

    // Code for enabling the plugin goes here.
    RegisterDebugCommand();
    return true;
}

} // namespace lse
