

#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;
// clang++ -o test TestTaskQueue.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    // 独占线程
    printf("-------------------------------------- 独占线程 --------------------------------------\n");
    auto queue = TaskQueueFactory::GetInstance().createSerialTaskQueue("test_execute", WorkThreadPriority::WTP_Normal, true);
    assert(queue != nullptr);
    queue->async([]() {
        printf("test_execute 1: thread_id: %d\n", std::this_thread::get_id());
    });

    queue->async([]() {
        printf("test_execute 2: thread_id: %d\n", std::this_thread::get_id());
    });

    queue->async([]() {
        printf("test_execute 3: thread_id: %d\n", std::this_thread::get_id());
    });

    // 共享线程
    printf("-------------------------------------- 共享线程 --------------------------------------\n");
    auto queue2 = TaskQueueFactory::GetInstance().createSerialTaskQueue("test_execute2", WorkThreadPriority::WTP_Normal, false);
    assert(queue2 != nullptr);
    queue2->async([]() {
        printf("test_execute 4: thread_id: %d\n", std::this_thread::get_id());
    });
    queue2->async([]() {
        printf("test_execute 5: thread_id: %d\n", std::this_thread::get_id());
    });

    // 同步任务
    printf("-------------------------------------- 同步任务 --------------------------------------\n");
    auto queue3 = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    queue3->sync([]() {
        printf("test_execute 8: thread_id: %d\n", std::this_thread::get_id());
    });
    queue3->sync([]() {
        printf("test_execute 9: thread_id: %d\n", std::this_thread::get_id());
    });
    queue3->sync([]() {
        printf("test_execute 10: thread_id: %d\n", std::this_thread::get_id());
    });

    printf("-------------sync wait\n");

    // 延时任务
    printf("---------------------------延时任务----------------------------\n");
    auto queue4 = TaskQueueFactory::GetInstance().globalConcurrencyQueue(TaskQueuePriority::TQP_Normal);
    queue4->after(std::chrono::milliseconds(3000), []() {
        printf("test_execute 11: thread_id: %d\n", std::this_thread::get_id());
    });

    printf("delay 3s\n");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    getchar();
    return 0;
}
