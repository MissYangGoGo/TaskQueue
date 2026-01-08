#include "LWBarrier.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace task
{
// 无任务owner
static constexpr LWBarrier::Waiter* kNoOwner{ nullptr };

// 已完成owner
static char                     kInvalidAddressUsedForCompleted = 0;
static LWBarrier::Waiter* const kCompleted{ reinterpret_cast<LWBarrier::Waiter*>(&kInvalidAddressUsedForCompleted) };

// mutex + condition_variable封装
class LWBarrier::Waiter
{
public:
    // 等待任务完成
    bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(-1))
    {
        auto predicate = [this] { return mCompleted; };

        std::unique_lock<std::mutex> loc(mLock);
        if (!predicate())
        {
            // 超时等待
            if (timeout != std::chrono::milliseconds(-1))
            {
                return mCond.wait_for(loc, timeout, predicate);
            }

            // 无限等待
            mCond.wait(loc, predicate);
        }

        return true;
    }

    // 任务完成通知
    void notify()
    {
        std::lock_guard<std::mutex> lock(mLock);
        mCompleted = true;
        mCond.notify_all();
    }

    // 任务是否已完成
    inline bool isCompleted() const
    {
        return mCompleted;
    }

private:
    bool                    mCompleted{ false };
    std::mutex              mLock;
    std::condition_variable mCond;
};

LWBarrier::LWBarrier()
    : mOwner(kNoOwner)
{
}

LWBarrier::~LWBarrier()
{
    // 释放资源
    auto* owner = mOwner.load(std::memory_order_acquire);
    if (owner && owner != kCompleted)
    {
        owner->notify();
        delete owner;
    }
}

bool LWBarrier::wait(std::chrono::milliseconds timeout)
{
    // 首先检查屏障是否已完成
    Waiter* prev = mOwner.load(std::memory_order_acquire);
    if (prev == kCompleted)
    {
        return true;
    }

    prev = kNoOwner;

    // 尝试成为第一个等待者(第一个创建waiter对象)
    auto waiter = std::unique_ptr<Waiter>(new Waiter());
    if (mOwner.compare_exchange_strong(prev, waiter.get(), std::memory_order_seq_cst))
    {
        // 成功，进行等待（释放所有权，由mOwner管理）
        auto* barrier = waiter.release();
        return barrier->wait(timeout);
    }

    // 失败，说明已经有其他线程在等待， 再一次判断状态
    if (prev == kCompleted)
    {
        // 屏障已完成直接返回
        return true;
    }

    // 直接在已等待者上等待 [notify使用notify_all通知所有waiter]
    return prev->wait(timeout);
}

void LWBarrier::notify()
{
    auto* prev = kNoOwner;
    if (mOwner.compare_exchange_strong(prev, kCompleted, std::memory_order_seq_cst))
    {
        // 没有人等待，将mOwener标记为kCompleted
    }
    else if (prev == kCompleted)
    {
        // 如果已经完成，直接返回
    }
    else if (prev != kNoOwner)
    {
        // 通知所有等待者
        prev->notify();
    }
}

bool LWBarrier::isCompleted()
{
    Waiter* prev = mOwner.load(std::memory_order_acquire);
    if (prev == kCompleted)
    {
        return true;
    }

    if (prev != kNoOwner)
    {
        return prev->isCompleted();
    }

    return false;
}

}  // namespace task