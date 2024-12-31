#include "engine/LocalShareData.h"

#include <ll/api/thread/ThreadPoolExecutor.h>
#include <mutex>

// DLL本地共享数据
LocalDataType* localShareData;

// 命令延迟注册队列
std::vector<RegCmdQueue> toRegCmdQueue;

// 线程池
ll::thread::ThreadPoolExecutor pool("LSE_POOL", 4);

// std::mutex messageLoopLock;

void InitLocalShareData() {
    srand(clock());
    localShareData = new LocalDataType;
}
