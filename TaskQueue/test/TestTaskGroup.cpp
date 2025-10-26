#include "../TaskDispatch.h"
#include <cassert>
#include <chrono>
#include <cstdio>
#include <stdio.h>
#include <thread>
using namespace task;
// clang++ -o test TestTaskGroup.cpp -L../build/lib -ldispatch_queue -Wl,-rpath,@loader_path/../build/lib -g -O0

int main(int argc, char* argv[])
{
    // 任务组
    printf("-------------------------------------- 任务组 --------------------------------------\n");
    auto group = TaskQueueFactory::GetInstance().createTaskGroup();
    assert(group != nullptr);
    group->async([]() {
        printf("test_execute 6: thread_id: %d\n", std::this_thread::get_id());
        std::this_thread::sleep_for(std::chrono::seconds(3));
    });
    group->async([]() {
        printf("test_execute 7: thread_id: %d\n", std::this_thread::get_id());
    });

    printf("wait before...\n");
    group->wait();
    printf("wait after...\n");

    // notify
    group->async([]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        printf("notify 8: thread_id: %d\n", std::this_thread::get_id());
    });
    group->async([]() {
        printf("notify 9: thread_id: %d\n", std::this_thread::get_id());
    });

    auto queue = TaskQueueFactory::GetInstance().globalSerialQueue();
    group->notify([queue]() {
        printf("notify 10: thread_id: %d\n", std::this_thread::get_id());
    },
                  queue);

    // 指定队列
    auto queue2 = TaskQueueFactory::GetInstance().globalSerialQueue();
    group->asyncQueue([queue2]() {
        printf("notify 11: thread_id: %d\n", std::this_thread::get_id());
    },
                      queue2);

    group->asyncQueue([queue2]() {
        printf("notify 12: thread_id: %d\n", std::this_thread::get_id());
    },
                      queue2);

    group->asyncQueue([queue2]() {
        printf("notify 13: thread_id: %d\n", std::this_thread::get_id());
    },
                      queue2);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    getchar();
    return 0;
}