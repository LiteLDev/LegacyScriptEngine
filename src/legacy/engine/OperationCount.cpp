#include "legacy/engine/OperationCount.h"

#include "legacy/engine/GlobalShareData.h"
#include "ll/api/mod/ModManager.h"

OperationCount::OperationCount(string const& name) : name(name) {}

OperationCount OperationCount::create(string const& name) {
    if (exists(name)) return OperationCount("");
    globalShareData->operationCountData[name] = 0;
    return OperationCount(name);
}

bool OperationCount::exists(string const& name) { return globalShareData->operationCountData.contains(name); }

bool OperationCount::remove() const {
    auto p = globalShareData->operationCountData.find(name);
    if (p != globalShareData->operationCountData.end()) {
        globalShareData->operationCountData.erase(p);
        return true;
    }
    return false;
}

bool OperationCount::done() const {
    auto p = globalShareData->operationCountData.find(name);
    if (p != globalShareData->operationCountData.end()) {
        InterlockedIncrement(reinterpret_cast<LONG*>(&(p->second)));
        return true;
    }
    return false;
}

int OperationCount::get() const {
    if (exists(name)) return globalShareData->operationCountData[name];
    return -1;
}

bool OperationCount::hasReachCount(int count) const { return get() >= count; }

bool OperationCount::hasReachMaxEngineCount() const {
    return hasReachCount(lse::LegacyScriptEngine::getInstance().getManager().getModCount());
}

bool OperationCount::hasReachMaxBackendCount() const { return hasReachCount(LLSE_VALID_BACKENDS_COUNT); }
