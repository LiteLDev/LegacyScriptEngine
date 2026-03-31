#include "legacy/main/BindAPIs.h"

#include "legacy/api/APIHelp.h"
#include "legacy/api/BaseAPI.h"
#include "legacy/api/BlockAPI.h"
#include "legacy/api/BlockEntityAPI.h"
#include "legacy/api/CommandAPI.h"
#include "legacy/api/CommandOriginAPI.h"
#include "legacy/api/CommandOutputAPI.h"
#include "legacy/api/ContainerAPI.h"
#include "legacy/api/DataAPI.h"
#include "legacy/api/DatabaseAPI.h"
#include "legacy/api/DeviceAPI.h"
#include "legacy/api/EntityAPI.h"
#include "legacy/api/FileSystemAPI.h"
#include "legacy/api/GameUtilsAPI.h"
#include "legacy/api/GuiAPI.h"
#include "legacy/api/ItemAPI.h"
#include "legacy/api/LlAPI.h"
#include "legacy/api/LoggerAPI.h"
#include "legacy/api/McAPI.h"
#include "legacy/api/NbtAPI.h"
#include "legacy/api/NetworkAPI.h"
#include "legacy/api/PacketAPI.h"
#include "legacy/api/PlayerAPI.h"
#include "legacy/api/ScoreboardAPI.h"
#include "legacy/api/ScriptAPI.h"
#include "legacy/api/SystemAPI.h"

// #include "legacy/api/PermissionAPI.h"
#include "legacy/api/InternationalAPI.h"
#include "legacy/api/ParticleAPI.h"

void BindAPIs(std::shared_ptr<ScriptEngine> const& engine) {

    //////////////// 全局函数 ////////////////

    engine->set("log", Function::newFunction(Log));
    engine->set("colorLog", Function::newFunction(ColorLog));
    engine->set("fastLog", Function::newFunction(FastLog));

#ifndef LSE_BACKEND_NODEJS // NodeJs has its own functions below
    engine->set("setTimeout", Function::newFunction(SetTimeout));
    engine->set("setInterval", Function::newFunction(SetInterval));
    engine->set("clearInterval", Function::newFunction(ClearInterval));
#endif

    //////////////// 静态类 ////////////////

    engine->registerNativeClass(McClassBuilder);
    engine->registerNativeClass(SystemClassBuilder);
    engine->registerNativeClass(LoggerClassBuilder);
    engine->registerNativeClass(DataClassBuilder);
    engine->registerNativeClass(MoneyClassBuilder);
    engine->registerNativeClass(NetworkClassBuilder);
    engine->registerNativeClass(LlClassBuilder);
    engine->registerNativeClass(VersionClassBuilder);
    engine->registerNativeClass(NbtStaticBuilder);
    engine->registerNativeClass(TextClassBuilder);
    engine->registerNativeClass(ParticleColorBuilder);
    engine->registerNativeClass(DirectionBuilder);

    engine->registerNativeClass(PermissionStaticBuilder);
    engine->registerNativeClass(ParamTypeStaticBuilder);
    engine->registerNativeClass(ParamOptionStaticBuilder);
    engine->registerNativeClass(OriginTypeStaticBuilder);
    engine->registerNativeClass(DamageCauseEnumBuilder);
    engine->registerNativeClass(ActorDamageCauseEnumBuilder);

    engine->registerNativeClass(I18nClassBuilder);

    //////////////// 实例类 ////////////////

    engine->registerNativeClass<IntPos>(IntPosBuilder);
    engine->registerNativeClass<FloatPos>(FloatPosBuilder);
    engine->registerNativeClass<DirectionAngle>(DirectionAngleBuilder);
    engine->registerNativeClass<BlockClass>(BlockClassBuilder);
    engine->registerNativeClass<KVDBClass>(KVDBClassBuilder);
    engine->registerNativeClass<DBSessionClass>(DBSessionClassBuilder);
    engine->registerNativeClass<DBStmtClass>(DBStmtClassBuilder);
    engine->registerNativeClass<ConfJsonClass>(ConfJsonClassBuilder);
    engine->registerNativeClass<ConfIniClass>(ConfIniClassBuilder);
    engine->registerNativeClass<DeviceClass>(DeviceClassBuilder);
    engine->registerNativeClass<ContainerClass>(ContainerClassBuilder);
    engine->registerNativeClass<EntityClass>(EntityClassBuilder);
    engine->registerNativeClass<FileClass>(FileClassBuilder);
    engine->registerNativeClass<WSClientClass>(WSClientClassBuilder);
    engine->registerNativeClass<BlockEntityClass>(BlockEntityClassBuilder);
    engine->registerNativeClass<SimpleFormClass>(SimpleFormClassBuilder);
    engine->registerNativeClass<CustomFormClass>(CustomFormClassBuilder);
    engine->registerNativeClass<ItemClass>(ItemClassBuilder);
    engine->registerNativeClass<PlayerClass>(PlayerClassBuilder);
    engine->registerNativeClass<ObjectiveClass>(ObjectiveClassBuilder);
    engine->registerNativeClass<PacketClass>(PacketClassBuilder);
    engine->registerNativeClass<NbtByteClass>(NbtByteClassBuilder);
    engine->registerNativeClass<NbtShortClass>(NbtShortClassBuilder);
    engine->registerNativeClass<NbtIntClass>(NbtIntClassBuilder);
    engine->registerNativeClass<NbtLongClass>(NbtLongClassBuilder);
    engine->registerNativeClass<NbtFloatClass>(NbtFloatClassBuilder);
    engine->registerNativeClass<NbtDoubleClass>(NbtDoubleClassBuilder);
    engine->registerNativeClass<NbtStringClass>(NbtStringClassBuilder);
    engine->registerNativeClass<NbtByteArrayClass>(NbtByteArrayClassBuilder);
    engine->registerNativeClass<NbtListClass>(NbtListClassBuilder);
    engine->registerNativeClass<NbtCompoundClass>(NbtCompoundClassBuilder);
    engine->registerNativeClass<CommandClass>(CommandClassBuilder);
    engine->registerNativeClass<CommandOriginClass>(CommandOriginClassBuilder);
    engine->registerNativeClass<CommandOutputClass>(CommandOutputClassBuilder);
    engine->registerNativeClass<HttpServerClass>(HttpServerClassBuilder);
    engine->registerNativeClass<HttpRequestClass>(HttpRequestClassBuilder);
    engine->registerNativeClass<HttpResponseClass>(HttpResponseClassBuilder);
    engine->registerNativeClass<BinaryStreamClass>(BinaryStreamClassBuilder);
    engine->registerNativeClass<ParticleSpawner>(ParticleSpawnerBuilder);
}
