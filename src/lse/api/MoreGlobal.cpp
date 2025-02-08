#include "MoreGlobal.h"

#include "ll/api/memory/Hook.h"
#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
#include "mc/world/level/storage/DBStorage.h"
#include "mc/world/level/storage/DBStorageConfig.h"

namespace lse::api::MoreGlobal {
DBStorage*             dbStorage;
DefaultDataLoadHelper* helper;
DefaultDataLoadHelper& defaultDataLoadHelper() { return (DefaultDataLoadHelper&)helper; }

LL_TYPE_INSTANCE_HOOK(
    DBStorageHook,
    HookPriority::Normal,
    DBStorage,
    &DBStorage::$ctor,
    void*,
    ::DBStorageConfig                           config,
    ::Bedrock::NotNullNonOwnerPtr<::LevelDbEnv> levelDbEnv
) {
    void* ori             = origin(std::move(config), levelDbEnv);
    MoreGlobal::dbStorage = (DBStorage*)ori;
    return ori;
};

void onLoad() { DBStorageHook::hook(); }

bool onEnable() {
    helper = (DefaultDataLoadHelper*)DefaultDataLoadHelper::$vftable();
    if (helper && dbStorage && DBStorageHook::unhook()) {
        return true;
    }
    return false;
}
} // namespace lse::api::MoreGlobal
