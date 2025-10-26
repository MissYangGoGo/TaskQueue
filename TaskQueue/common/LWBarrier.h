// 轻量级屏障 【lightweight barrier】 缩写
// 封装 mutex/condition_variable
// 适合一次性事件同步
// eg:
// LWBarrier barrier;
// queue->async([&barrier] {
//     ...
//     barrier.notify();
// });
// barrier.wait();

#ifndef __LWBarrier_H__
#define __LWBarrier_H__

#include <atomic>
#include <chrono>
namespace task
{
class LWBarrier
{
public:
    class Waiter;
    LWBarrier();
    ~LWBarrier();

    // 当前线程等待任务执行
    bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(-1));

    // 执行线程任务完成通知
    void notify();

    // 任务是否已完成
    bool isCompleted();

private:
    std::atomic<Waiter*> mOwner;
};
}  // namespace task

#endif  //__LWBarrier_H__
