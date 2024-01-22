#include "MoreGlobal.h"

#include "ll/api/memory/Hook.h"
#include "mc/world/level/storage/DBStorage.h"
#include "mc/world/level/storage/DBStorageConfig.h"

LL_TYPE_INSTANCE_HOOK(
    DBStorageHook,
    HookPriority::Normal,
    DBStorage,
    "??0DBStorage@@QEAA@UDBStorageConfig@@V?$not_null@V?$"
    "NonOwnerPointer@VLevelDbEnv@@@Bedrock@@@gsl@@@Z",
    DBStorage,
    struct DBStorageConfig                        cfg,
    Bedrock::NotNullNonOwnerPtr<class LevelDbEnv> dbEnv
) {

    DBStorage ori = origin(cfg, dbEnv);
    MoreGlobal::setDBStorage(&ori);
    return ori;
};

void MoreGlobal::Init() { DBStorageHook::hook(); }