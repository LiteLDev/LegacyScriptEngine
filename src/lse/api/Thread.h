#pragma once

namespace lse::api::thread {
bool        isServerThread();
inline bool checkClientIsServerThread() {
#ifdef LL_PLAT_C
    return isServerThread();
#else
    return true;
#endif
}
} // namespace lse::api::thread
