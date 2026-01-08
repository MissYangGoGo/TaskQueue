#include "Semaphore.h"
#include <atomic>
#include "SysUtils.h"

namespace task
{
Semaphore::Semaphore(int count)
    : mCount(count)
    , mWaiter(0)
{
}

bool Semaphore::tryAcquire()
{
    auto old_cnt = mCount.load(std::memory_order_acquire);
    do
    {
        // 没有信号量, 直接返回
        if (0 == old_cnt)
        {
            return false;
        }
        // 如果有信号量，无锁获取并减一, 尝试直到无信号为止
        if (mCount.compare_exchange_weak(old_cnt, old_cnt - 1, std::memory_order_acq_rel))
        {
            return true;
        }
    } while (true);
}

bool Semaphore::spinAcquire(int count)
{
    for (int i = 0; i < count; ++i)
    {
        // 先尝试获取信号量
        if (tryAcquire())
        {
            return true;
        }

        // 如果没获取到，自旋等待
        while (0 == mCount.load(std::memory_order_acquire) && i < count)
        {
            // 让出CPU
            SysUtils::cpuYield();

            // 次数增加
            ++i;
        }
    }
    return false;
}

bool Semaphore::waitAcquire(std::chrono::milliseconds timeout)
{
    struct WaiterScope
    {
        explicit WaiterScope(std::atomic<int>& waiters)
            : mWaiters(waiters)
        {
            mWaiters.fetch_add(1, std::memory_order_release);
        }
        ~WaiterScope()
        {
            mWaiters.fetch_sub(1, std::memory_order_release);
        }
        WaiterScope(const WaiterScope&) = delete;
        WaiterScope(WaiterScope&&)      = delete;
        WaiterScope& operator=(const WaiterScope&) = delete;
        WaiterScope& operator=(WaiterScope&&) = delete;

    private:
        std::atomic<int>& mWaiters;
    };

    // 1.尝试获取一次
    if (tryAcquire())
    {
        return true;
    }

    // 2.失败了，加锁再尝试一下，避免加锁前后状态发生变化
    std::unique_lock<std::mutex> lock(mMutex);
    if (tryAcquire())
    {
        return true;
    }

    // 3.还是失败了，条件变量等待, 等待数量+1
    WaiterScope scope(mWaiter);
    return mCondVar.wait_for(lock, timeout, [this]() { return tryAcquire(); });
}

void Semaphore::release(int count)
{
    // 增加信号量
    mCount.fetch_add(count, std::memory_order_seq_cst);

    // 是否有等待线程
    if (0 != mWaiter.load(std::memory_order_acquire))
    {
        std::lock_guard<std::mutex> lock(mMutex);
        if (1 == count)
        {
            mCondVar.notify_one();
        }
        else
        {
            mCondVar.notify_all();
        }
    }
}

}  // namespace task