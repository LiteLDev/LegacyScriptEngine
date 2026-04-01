#include "legacy/engine/LocalShareData.h"

// DLL本地共享数据
std::unique_ptr<LocalDataType> localShareData = nullptr;

// 线程池
ll::thread::ThreadPoolExecutor pool("LSE_POOL", LLSE_POOL_THREAD_COUNT);

void InitLocalShareData() {
    srand(clock());
    localShareData = std::make_unique<LocalDataType>();
}
