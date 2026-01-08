#include "SerialThreadPool.h"
#include "WorkThreadBase.h"
#include "WorkThreadSerial.h"
#include "SysUtils.h"
#include "common/LogHelper.h"
#include "TaskQueueReporter.h"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>
namespace task
{
void SerialThreadPool::registerWorkThread(const std::shared_ptr<WorkThreadBase>& thread)
{
    if (thread == nullptr)
    {
        LOGE("[TASK]SerialThreadPool::registerWorkThread, thread is nullptr");
        return;
    }

    task::WriteLock lock(mExclusiveThreadsLock);
    mExclusiveThreads[thread->threadId()] = thread;
    TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_ThreadCountChanged,
                                                  _threadPoolInfo("threadid:" + std::to_string(thread->threadId()) + " created"));
}

void SerialThreadPool::unregisterWorkThread(const std::shared_ptr<WorkThreadBase>& thread)
{
    if (thread == nullptr)
    {
        LOGE("[TASK]SerialThreadPool::unregisterWorkThread, thread is nullptr");
        return;
    }
    
    task::WriteLock lock(mExclusiveThreadsLock);
    mExpiredThreads.push_back(thread);
    mExclusiveThreads.erase(thread->threadId());
    // 释放一条线程
    TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_ThreadCountChanged,
                                                  _threadPoolInfo("threadid:" + std::to_string(thread->threadId()) + " released"));
}

int32_t SerialThreadPool::attachOneThread(const std::string& name, WorkThreadPriority prio)
{
    int32_t index = -1;
    {
        task::WriteLock lock(mExclusiveThreadsLock);
        for (auto it = mExclusiveThreads.begin(); it != mExclusiveThreads.end();)
        {
            // 卡顿检查
            if (it->second && it->second->isBlocked())
            {
                TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_TaskDurationExceedThreshold, it->second->blockedInfo());
                LOGE("[TASK]SerialThreadPool::attachOneThread, threadId: %d is blocked, blockedInfo: %s \n", it->second->threadId(), it->second->blockedInfo().c_str());
                // 尝试cancel 与 删除
                it->second->cancel();
                it = mExclusiveThreads.erase(it);  // erase返回下一个有效迭代器
                continue;                          // 跳过当前循环，继续下一个元素
            }

            // 非active且没有running (可能存在线程卡住一直running的情况，此时切换线程)
            if (it->second && !it->second->isActive() && !it->second->isRunning())
            {
                it->second->setActive(true);
                it->second->setName(name);
                it->second->setPriority(prio);
                index = it->second->threadId();
                break;
            }

            ++it;  // 手动递增迭代器
        }
    }

    if (index == -1)
    {
        task::WriteLock lock(mExclusiveThreadsLock);
        // 已经达到最大线程数量限制
        if (mExclusiveThreads.size() >= SysUtils::cpuCount())
        {
            LOGE("[TASK]SerialThreadPool::attachOneThread, reach max thread count, max: %d \n", SysUtils::cpuCount());
            TaskQueueReporter::GetInstance().notifyReport(TaskQueueReporterType::TQRT_ThreadCountChanged, _threadPoolInfo("reach_max_thread_count"));
#ifdef _DEBUG
            assert(false);
#endif
        }

        // 没有找到合适的线程，创建新线程
        auto thread = std::make_shared<WorkThreadSerial>(shared_from_this());
        thread->setActive(true);
        thread->setPriority(prio);
        thread->setName(name);
        index                    = thread->threadId();
        mExclusiveThreads[index] = std::static_pointer_cast<WorkThreadBase>(thread);

        LOGE("[TASK]SerialThreadPool::attachOneThread, create new thread, threadId: %d \n", index);
    }

    LOGE("[TASK]SerialThreadPool::attachOneThread, name: %s, prio: %d, threadId: %d \n", name.c_str(), prio, index);

    return index;
}

void SerialThreadPool::detachOneThread(int32_t threadID)
{
    LOGE("[TASK]SerialThreadPool::detachOneThread, threadID: %d\n", threadID);
    task::WriteLock lock(mExclusiveThreadsLock);
    auto            it = mExclusiveThreads.find(threadID);
    if (it != mExclusiveThreads.end())
    {
        it->second->setActive(false);
    }
}

void SerialThreadPool::execute(const TaskOperatorPtr& task, int32_t threadId)
{
    std::vector<std::shared_ptr<WorkThreadBase>> expiredThreads;
    {
        task::ReadLock lock(mExclusiveThreadsLock);
        auto           it = mExclusiveThreads.find(threadId);
        if (it != mExclusiveThreads.end())
        {
            it->second->commit(task);
        }

        // 删除上一帧结束的线程对象
        expiredThreads.swap(mExpiredThreads);
    }
}

// 此方法调用处已在锁保护范围内
std::string SerialThreadPool::_threadPoolInfo(std::string reason) const
{
    std::string info = "SerialPool: t_cnt: "
                       + std::to_string(mExclusiveThreads.size())
                       + ", active: "
                       + std::to_string(_activeThreadCount())
                       + ", max: "
                       + std::to_string(SysUtils::cpuCount())
                       + ", reason: "
                       + reason;

    LOGE("[TASK] %s", info.c_str());
    return info;
}

int32_t SerialThreadPool::_activeThreadCount() const
{
    int32_t        _cnt = 0;
    for (auto& item : mExclusiveThreads)
    {
        if (item.second && item.second->isActive())
        {
            _cnt++;
        }
    }

    return _cnt;
}

}  // namespace task