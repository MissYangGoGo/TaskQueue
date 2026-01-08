

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <chrono>
#include <atomic>
#include <mutex>
#include <condition_variable>

// 信号量
namespace task
{
class Semaphore final
{
public:
    // 初始信号量数量
    explicit Semaphore(int count = 0);

    ~Semaphore() = default;

    // 尝试获取信号量, 成功返回true, 失败返回false
    bool tryAcquire();

    // 自旋获取信号量(尝试一定次数), 成功返回true, 失败返回false
    bool spinAcquire(int count);

    // 等待获取信号量(超时), 成功返回true, 失败返回false
    bool waitAcquire(std::chrono::milliseconds timeout);

    // 释放信号量
    void release(int count = 1);

private:
    std::atomic<int>        mCount;
    std::atomic<int>        mWaiter;
    std::mutex              mMutex;
    std::condition_variable mCondVar;
};

}  // namespace task

#endif  // __SEMAPHORE_H__