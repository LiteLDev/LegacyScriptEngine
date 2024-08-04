#include "mc/dataloadhelper/DefaultDataLoadHelper.h"
class DBStorage;
namespace MoreGlobal {
extern DBStorage*             dbStorage;
extern DefaultDataLoadHelper* defaultDataLoadHelper;
extern void                   onLoad();
extern bool                   onEnable();
}; // namespace MoreGlobal
