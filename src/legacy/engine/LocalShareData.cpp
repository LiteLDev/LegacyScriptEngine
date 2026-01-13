#include "engine/LocalShareData.h"

#include <mutex>

// DLL本地共享数据
std::unique_ptr<LocalDataType> localShareData = nullptr;

// 命令延迟注册队列
std::vector<RegCmdQueue> toRegCmdQueue;

// 线程池
ll::thread::ThreadPoolExecutor pool("LSE_POOL", LLSE_POOL_THREAD_COUNT);

// std::mutex messageLoopLock;

void InitLocalShareData() {
    srand(clock());
    localShareData = std::make_unique<LocalDataType>();
}
