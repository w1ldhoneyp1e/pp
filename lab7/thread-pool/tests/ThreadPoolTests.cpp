#include "../include/AsioThreadPool.h"
#include "../include/ThreadPool.h"
#include "../include/ThreadPoolWithLock.h"

#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{

template <typename Pool>
void TestExecutesAllTasks()
{
    constexpr std::size_t taskCount = 10000;

    Pool pool(4);
    std::atomic<std::size_t> completedTasks(0);

    for (std::size_t taskIndex = 0;
         taskIndex < taskCount;
         ++taskIndex)
    {
        pool.Submit(
            [&completedTasks]()
            {
                completedTasks.fetch_add(
                    1,
                    std::memory_order_relaxed);
            });
    }

    pool.Wait();

    assert(completedTasks.load(std::memory_order_relaxed) == taskCount);
}

template <typename Pool>
void TestTasksRunOnWorkerThreads()
{
    Pool pool(4);

    const std::thread::id mainThreadId =
        std::this_thread::get_id();

    std::atomic<std::size_t> workerExecutions(0);

    for (std::size_t taskIndex = 0;
         taskIndex < 100;
         ++taskIndex)
    {
        pool.Submit(
            [&workerExecutions, mainThreadId]()
            {
                if (std::this_thread::get_id() != mainThreadId)
                {
                    workerExecutions.fetch_add(
                        1,
                        std::memory_order_relaxed);
                }
            });
    }

    pool.Wait();

    assert(workerExecutions.load(std::memory_order_relaxed) == 100);
}

template <typename Pool>
void TestConcurrentSubmitters()
{
    constexpr std::size_t submitterCount = 8;
    constexpr std::size_t tasksPerSubmitter = 2000;

    Pool pool(6);
    std::atomic<std::size_t> completedTasks(0);
    std::vector<std::thread> submitters;

    submitters.reserve(submitterCount);

    for (std::size_t submitterIndex = 0;
         submitterIndex < submitterCount;
         ++submitterIndex)
    {
        submitters.emplace_back(
            [&pool, &completedTasks]()
            {
                for (std::size_t taskIndex = 0;
                     taskIndex < tasksPerSubmitter;
                     ++taskIndex)
                {
                    pool.Submit(
                        [&completedTasks]()
                        {
                            completedTasks.fetch_add(
                                1,
                                std::memory_order_relaxed);
                        });
                }
            });
    }

    for (std::thread& submitter : submitters)
    {
        submitter.join();
    }

    pool.Wait();

    assert(
        completedTasks.load(std::memory_order_relaxed) ==
        submitterCount * tasksPerSubmitter);
}

template <typename Pool>
void TestWaitWaitsForLongTask()
{
    Pool pool(2);
    std::atomic<bool> taskFinished(false);

    pool.Submit(
        [&taskFinished]()
        {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(20));

            taskFinished.store(
                true,
                std::memory_order_release);
        });

    pool.Wait();

    assert(taskFinished.load(std::memory_order_acquire));
}

template <typename Pool>
void TestTaskExceptionDoesNotStopPool()
{
    Pool pool(2);
    std::atomic<std::size_t> completedTasks(0);

    pool.Submit(
        []()
        {
            throw std::runtime_error("test exception");
        });

    pool.Submit(
        [&completedTasks]()
        {
            completedTasks.fetch_add(
                1,
                std::memory_order_relaxed);
        });

    pool.Wait();

    assert(completedTasks.load(std::memory_order_relaxed) == 1);
}

void TestInvalidThreadCount()
{
    bool exceptionThrown = false;

    try
    {
        ThreadPool pool(0);
    }
    catch (const std::invalid_argument&)
    {
        exceptionThrown = true;
    }

    assert(exceptionThrown);
}

template <typename Pool>
void RunCommonTests()
{
    TestExecutesAllTasks<Pool>();
    TestTasksRunOnWorkerThreads<Pool>();
    TestConcurrentSubmitters<Pool>();
    TestWaitWaitsForLongTask<Pool>();
    TestTaskExceptionDoesNotStopPool<Pool>();
}

}

int main()
{
    RunCommonTests<ThreadPool>();
    RunCommonTests<ThreadPoolWithLock>();
    RunCommonTests<AsioThreadPool>();

    TestInvalidThreadCount();

    ThreadPool pool(2);
    std::cout
        << "Boost.LockFree queue is lock-free: "
        << std::boolalpha
        << pool.IsQueueLockFree()
        << '\n';

    std::cout << "All ThreadPool tests passed\n";
    return 0;
}
