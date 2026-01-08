#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;

// clang++ -o test TestTaskQueueComprehensive.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    printf("-------------------------------------- TaskQueue综合功能测试 --------------------------------------\n");
    
    auto& factory = TaskQueueFactory::GetInstance();
    
    // 测试串行队列的顺序执行
    printf("-------------------- 串行队列顺序执行测试 --------------------\n");
    auto serialQueue = factory.createSerialTaskQueue("serial_test", WorkThreadPriority::WTP_Normal, false);
    assert(serialQueue != nullptr);
    
    int counter = 0;
    serialQueue->async([&counter]() {
        counter = 1;
        printf("串行任务1执行, counter: %d, thread_id: %lu\n", counter, std::this_thread::get_id());
    });
    
    serialQueue->async([&counter]() {
        counter = 2;
        printf("串行任务2执行, counter: %d, thread_id: %lu\n", counter, std::this_thread::get_id());
    });
    
    serialQueue->async([&counter]() {
        counter = 3;
        printf("串行任务3执行, counter: %d, thread_id: %lu\n", counter, std::this_thread::get_id());
    });
    
    // 等待一段时间确保任务执行完成
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    printf("串行队列最终counter值: %d (期望值: 3)\n", counter);
    
    // 测试并发队列的并发执行
    printf("-------------------- 并发队列并发执行测试 --------------------\n");
    auto concurrentQueue = factory.createConcurrencyTaskQueue("concurrent_test", TaskQueuePriority::TQP_Normal);
    assert(concurrentQueue != nullptr);
    
    concurrentQueue->async([]() {
        printf("并发任务1开始执行, thread_id: %lu\n", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        printf("并发任务1执行完成, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    concurrentQueue->async([]() {
        printf("并发任务2开始执行, thread_id: %lu\n", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        printf("并发任务2执行完成, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    concurrentQueue->async([]() {
        printf("并发任务3开始执行, thread_id: %lu\n", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        printf("并发任务3执行完成, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    // 测试同步任务
    printf("-------------------- 同步任务测试 --------------------\n");
    int syncCounter = 0;
    concurrentQueue->sync([&syncCounter]() {
        syncCounter = 100;
        printf("同步任务执行, counter: %d, thread_id: %lu\n", syncCounter, std::this_thread::get_id());
    });
    
    printf("同步任务执行后counter值: %d (期望值: 100)\n", syncCounter);
    
    // 测试延时任务
    printf("-------------------- 延时任务测试 --------------------\n");
    auto delayQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    auto startTime = std::chrono::steady_clock::now();
    
    delayQueue->after(std::chrono::milliseconds(1000), [&startTime]() {
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        printf("延时任务执行, 实际延时: %lld ms, thread_id: %lu\n", duration.count(), std::this_thread::get_id());
    });
    
    printf("延时任务已提交，等待执行...\n");
    
    // 测试队列标签
    printf("-------------------- 队列标签测试 --------------------\n");
    auto labeledQueue = factory.createSerialTaskQueue("my_custom_queue", WorkThreadPriority::WTP_High, true);
    assert(labeledQueue != nullptr);
    printf("自定义队列标签: %s\n", labeledQueue->label().c_str());
    assert(labeledQueue->label() == "my_custom_queue");
    printf("队列标签测试通过\n");
    
    printf("-------------TaskQueue综合功能测试完成，请等待所有异步任务执行完毕-------------\n");
    
    // 等待所有异步任务完成
    std::this_thread::sleep_for(std::chrono::seconds(3));
    getchar();
    return 0;
}