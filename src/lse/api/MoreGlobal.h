#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
class DBStorage;
namespace lse::api::MoreGlobal {
extern DBStorage*             dbStorage;
extern DefaultDataLoadHelper& defaultDataLoadHelper();
extern void                   onLoad();
extern bool                   onEnable();
}; // namespace lse::api::MoreGlobal
