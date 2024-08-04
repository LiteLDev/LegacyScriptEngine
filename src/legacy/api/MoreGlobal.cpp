#include "MoreGlobal.h"

#include "ll/api/memory/Hook.h"
#include "mc/world/level/storage/DBStorage.h"
#include "mc/world/level/storage/DBStorageConfig.h"

DBStorage*             MoreGlobal::dbStorage;
DefaultDataLoadHelper* MoreGlobal::defaultDataLoadHelper;

LL_TYPE_INSTANCE_HOOK(
    DBStorageHook,
    HookPriority::Normal,
    DBStorage,
    "??0DBStorage@@QEAA@UDBStorageConfig@@V?$not_null@V?$NonOwnerPointer@VLevelDbEnv@@@Bedrock@@@gsl@@@Z",
    DBStorage*,
    struct DBStorageConfig&                        cfg,
    Bedrock::NotNullNonOwnerPtr<class LevelDbEnv>& dbEnv
) {
    DBStorage* ori        = origin(cfg, dbEnv);
    MoreGlobal::dbStorage = ori;
    return ori;
};

void MoreGlobal::onLoad() { DBStorageHook::hook(); }

bool MoreGlobal::onEnable() {
    defaultDataLoadHelper =
        static_cast<DefaultDataLoadHelper*>(ll::memory::resolveSymbol("??_7DefaultDataLoadHelper@@6B@"));
    if (defaultDataLoadHelper && DBStorageHook::unhook()) {
        return true;
    }
    return false;
}
