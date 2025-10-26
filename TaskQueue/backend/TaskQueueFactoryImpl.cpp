#include "TaskQueueFactoryImpl.h"
#include "GroupImpl.h"
#include "IQueueImpl.h"
#include "IThreadPool.h"
#include "SerialQueueImpl.h"
#include "ConcurrencyQueueImpl.h"
#include "TaskGroup.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <memory>

#include "common/LogHelper.h"
namespace task
{
// 全局初始化并行队列
static std::array<TaskQueuePtr, uint32_t(TaskQueuePriority::TQP_Count)> sParallelQueues = {
    std::make_shared<TaskQueue>("queue.global.parallel_low",
                                std::make_shared<ConcurrencyQueueImpl>(TaskQueuePriority::TQP_Low, IThreadPool::parallelThreadPool())),
    std::make_shared<TaskQueue>("queue.global.parallel_medium",
                                std::make_shared<ConcurrencyQueueImpl>(TaskQueuePriority::TQP_Normal, IThreadPool::parallelThreadPool())),
    std::make_shared<TaskQueue>("queue.global.parallel_high",
                                std::make_shared<ConcurrencyQueueImpl>(TaskQueuePriority::TQP_High, IThreadPool::parallelThreadPool())),
};

// 全局串行队列【方便抛一个任务类型，不用创建队列】
static TaskQueuePtr sSerialQueue = std::make_shared<TaskQueue>("queue.global_serial",
                                                               std::make_shared<SerialQueueImpl>("queue.global_serial.backend",
                                                                                                 false,
                                                                                                 IThreadPool::parallelThreadPool(),
                                                                                                 WorkThreadPriority::WTP_Normal));

// 创建串行队列
TaskQueuePtr TaskQueueFactoryImpl::createSerialQueue(const std::string& label, WorkThreadPriority priority, bool isExclusive)
{
    LOGE("[HY] TaskQueueFactoryImpl::%s, label: %s, priority: %d, isExclusive: %d\n", __func__, label.c_str(), priority, isExclusive);
    auto threadPool   = isExclusive ? IThreadPool::serialThreadPool() : IThreadPool::parallelThreadPool();
    auto backendQueue = std::make_shared<SerialQueueImpl>(label, isExclusive, threadPool, priority);
    return std::make_shared<TaskQueue>(label, backendQueue);
}

// 创建并行队列
TaskQueuePtr TaskQueueFactoryImpl::createConcurrencyQueue(const std::string& label, TaskQueuePriority priority)
{
    LOGE("[HY] TaskQueueFactoryImpl::%s, label: %s, priority: %d\n", __func__, label.c_str(), priority);
    return std::make_shared<TaskQueue>(label,
                                       std::make_shared<ConcurrencyQueueImpl>(priority, IThreadPool::parallelThreadPool()));
}

// 获取队列 【全局初始化并行队列】
TaskQueuePtr& TaskQueueFactoryImpl::getConcurrencyQueue(TaskQueuePriority priority)
{
    assert(priority >= TaskQueuePriority::TQP_Low && priority < TaskQueuePriority::TQP_Count);
    return sParallelQueues[uint32_t(priority)];
}

TaskQueuePtr& TaskQueueFactoryImpl::getSerialQueue()
{
    return sSerialQueue;
}

TaskGroupPtr TaskQueueFactoryImpl::createTaskGroup()
{
    return std::make_shared<TaskGroup>(
        std::make_shared<GroupImpl>(IThreadPool::parallelThreadPool()));
}
}  // namespace task