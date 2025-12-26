#include <iostream>
#include <atomic>
#include "../test.h"
#include "ThreadPool.h"

TEST_CASE("DynamicArray operations")
{
    DynamicArray<int> test;
    for(int i = 0; i < 12; i++)
    {
        test.PushBack(i);
    }

    REQUIRE(test.size == 12);
    REQUIRE(test[0] == 0);
    REQUIRE(test[11] == 11);

    test.RemoveRange(1, 5);
    REQUIRE(test.size == 7);
    REQUIRE(test[0] == 0);
    REQUIRE(test[1] == 6);
}

TEST_CASE("ThreadPool executes all jobs")
{
    ThreadPool* pool = CreateThreadPool(4);

    std::atomic<int> counter{0};
    const int num_jobs = 100;

    for(int i = 0; i < num_jobs; i++)
    {
        pool->AddJob(new std::function([&counter](){
            counter.fetch_add(1, std::memory_order_relaxed);
        }));
    }

    pool->StartAndWaitForFinish();

    REQUIRE(counter.load() == num_jobs);

    DestroyThreadPool(pool);
}
