#include "lse/api/MoreGlobal.h"

#include "mc/dataloadhelper/DefaultDataLoadHelper.h"

namespace lse::api::MoreGlobal {

DefaultDataLoadHelper  helper;
DefaultDataLoadHelper& defaultDataLoadHelper() { return helper; }

} // namespace lse::api::MoreGlobal
