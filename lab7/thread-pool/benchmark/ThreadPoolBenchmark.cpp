#include "../include/AsioThreadPool.h"
#include "../include/ThreadPool.h"
#include "../include/ThreadPoolWithLock.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>

namespace
{

void DoWork(
    std::atomic<std::uint64_t>& checksum,
    std::size_t taskIndex) noexcept
{
    std::uint64_t value =
        static_cast<std::uint64_t>(taskIndex) + 1U;

    for (int iteration = 0; iteration < 32; ++iteration)
    {
        value = value * 1664525U + 1013904223U;
    }

    checksum.fetch_add(value, std::memory_order_relaxed);
}

template <typename Pool>
double Measure(
    std::size_t threadCount,
    std::size_t taskCount)
{
    Pool pool(threadCount);
    std::atomic<std::uint64_t> checksum(0);

    const auto startTime =
        std::chrono::steady_clock::now();

    for (std::size_t taskIndex = 0;
         taskIndex < taskCount;
         ++taskIndex)
    {
        pool.Submit(
            [&checksum, taskIndex]()
            {
                DoWork(checksum, taskIndex);
            });
    }

    pool.Wait();

    const auto endTime =
        std::chrono::steady_clock::now();

    const std::uint64_t result =
        checksum.load(std::memory_order_relaxed);

    if (result == 0)
    {
        std::cerr << "Unexpected zero checksum\n";
    }

    return std::chrono::duration<double, std::milli>(
        endTime - startTime).count();
}

void PrintRow(
    std::size_t threadCount,
    std::string_view implementation,
    double milliseconds)
{
    std::cout
        << threadCount << ','
        << implementation << ','
        << std::fixed
        << std::setprecision(3)
        << milliseconds
        << '\n';
}

}

int main(int argc, char* argv[])
{
    try
    {
        const unsigned int detectedProcessorCount =
            std::thread::hardware_concurrency();

        const std::size_t processorCount =
            detectedProcessorCount == 0
                ? 1
                : detectedProcessorCount;

        const std::size_t defaultMaximumThreadCount =
            processorCount * 2;

        const std::size_t taskCount =
            argc >= 2
                ? std::stoull(argv[1])
                : 200000;

        const std::size_t maximumThreadCount =
            argc >= 3
                ? std::stoull(argv[2])
                : defaultMaximumThreadCount;

        if (taskCount == 0 || maximumThreadCount == 0)
        {
            throw std::invalid_argument(
                "Task and thread counts must be greater than zero");
        }

        std::cout << "threads,implementation,time_ms\n";

        for (std::size_t threadCount = 1;
             threadCount <= maximumThreadCount;
             ++threadCount)
        {
            PrintRow(
                threadCount,
                "lock_free",
                Measure<ThreadPool>(threadCount, taskCount));

            PrintRow(
                threadCount,
                "lock_based",
                Measure<ThreadPoolWithLock>(threadCount, taskCount));

            PrintRow(
                threadCount,
                "boost_asio",
                Measure<AsioThreadPool>(threadCount, taskCount));
        }

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}
