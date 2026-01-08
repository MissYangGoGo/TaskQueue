#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;

// clang++ -o test TestTaskQueueFactory.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    printf("-------------------------------------- TaskQueueFactory测试 --------------------------------------\n");
    
    // 获取工厂实例
    auto& factory = TaskQueueFactory::GetInstance();
    printf("工厂实例获取成功\n");
    
    // 测试创建串行队列（独占线程）
    auto serialQueueExclusive = factory.createSerialTaskQueue("test_serial_exclusive", WorkThreadPriority::WTP_Normal, true);
    assert(serialQueueExclusive != nullptr);
    printf("独占线程串行队列创建成功: %s\n", serialQueueExclusive->label().c_str());
    
    // 测试创建串行队列（共享线程）
    auto serialQueueShared = factory.createSerialTaskQueue("test_serial_shared", WorkThreadPriority::WTP_Normal, false);
    assert(serialQueueShared != nullptr);
    printf("共享线程串行队列创建成功: %s\n", serialQueueShared->label().c_str());
    
    // 测试创建并行队列
    auto concurrencyQueue = factory.createConcurrencyTaskQueue("test_concurrency", TaskQueuePriority::TQP_Normal);
    assert(concurrencyQueue != nullptr);
    printf("并行队列创建成功: %s\n", concurrencyQueue->label().c_str());
    
    // 测试获取全局并发队列
    auto globalLowQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Low);
    assert(globalLowQueue != nullptr);
    printf("全局低优先级并发队列获取成功\n");
    
    auto globalNormalQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    assert(globalNormalQueue != nullptr);
    printf("全局普通优先级并发队列获取成功\n");
    
    auto globalHighQueue = factory.globalConcurrencyQueue(TaskQueuePriority::TQP_High);
    assert(globalHighQueue != nullptr);
    printf("全局高优先级并发队列获取成功\n");
    
    // 测试获取全局串行队列
    auto globalSerialQueue = factory.globalSerialQueue();
    assert(globalSerialQueue != nullptr);
    printf("全局串行队列获取成功\n");
    
    // 测试创建任务组
    auto taskGroup = factory.createTaskGroup();
    assert(taskGroup != nullptr);
    printf("任务组创建成功\n");
    
    // 验证不同队列的标签
    assert(serialQueueExclusive->label() == "test_serial_exclusive");
    assert(serialQueueShared->label() == "test_serial_shared");
    assert(concurrencyQueue->label() == "test_concurrency");
    printf("队列标签验证成功\n");
    
    // 测试在不同队列上执行任务
    serialQueueExclusive->async([]() {
        printf("独占串行队列任务执行, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    serialQueueShared->async([]() {
        printf("共享串行队列任务执行, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    concurrencyQueue->async([]() {
        printf("并行队列任务执行, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    globalLowQueue->async([]() {
        printf("全局低优先级队列任务执行, thread_id: %lu\n", std::this_thread::get_id());
    });
    
    printf("-------------TaskQueueFactory测试完成-------------\n");
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    getchar();
    return 0;
}