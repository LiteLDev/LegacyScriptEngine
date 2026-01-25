#include "lse/api/Thread.h"

#include "ll/api/service/Bedrock.h"
#include "mc/server/ServerInstance.h"

namespace lse::api::thread {
bool isServerThread() {
    auto instance = ll::service::getServerInstance();
    return instance && std::this_thread::get_id() == instance->mServerInstanceThread->get_id();
}
} // namespace lse::api::thread
