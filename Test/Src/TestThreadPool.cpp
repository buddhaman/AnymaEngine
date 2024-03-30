#include <iostream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include <catch2/catch_approx.hpp>

#include "../../src/ThreadPool.h"

TEST_CASE("Setup worker threads")
{
    DynamicArray<int> test;
    for(int i = 0; i < 12; i++)
    {
        test.PushBack(i);
    }
    PrintArray(test);
    test.RemoveRange(1,5);
    PrintArray(test);

    ThreadPool* pool = CreateThreadPool(4);

    for(int i = 0; i < 100; i++)
    {
        pool->AddJob(new std::function([i](){
            std::cout << "Hello from function " << i << std::endl;
        }));
    }

    pool->WaitForFinish();
    std::cout << "Gracefully finished every job" << std::endl;
    DestroyThreadPool(pool);
}
