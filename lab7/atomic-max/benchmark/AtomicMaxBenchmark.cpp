#include "AtomicMax.h"
#include "AtomicMaxWithLock.h"

#include <barrier>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

namespace
{

constexpr std::size_t updatesPerThread = 500000;

std::uint32_t NextRandom(std::uint32_t& state) noexcept
{
    state = state * 1664525U + 1013904223U;
    return state;
}

template <typename Maximum>
double Measure(int threadCount)
{
    Maximum maximum(std::numeric_limits<std::uint32_t>::lowest());
    std::barrier startBarrier(threadCount + 1);

    std::vector<std::thread> threads;
    threads.reserve(static_cast<std::size_t>(threadCount));

    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back(
            [&maximum, &startBarrier, threadIndex]()
            {
                std::uint32_t randomState =
                    123456789U +
                    static_cast<std::uint32_t>(threadIndex);

                startBarrier.arrive_and_wait();

                for (std::size_t updateIndex = 0;
                     updateIndex < updatesPerThread;
                     ++updateIndex)
                {
                    maximum.Update(NextRandom(randomState));
                }
            });
    }

    startBarrier.arrive_and_wait();
    const auto startTime = std::chrono::steady_clock::now();

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    const auto endTime = std::chrono::steady_clock::now();

    volatile std::uint32_t result = maximum.GetValue();
    (void)result;

    return std::chrono::duration<double, std::milli>(
        endTime - startTime).count();
}

}

int main()
{
    std::cout
        << std::left
        << std::setw(10) << "Threads"
        << std::setw(20) << "Lock-free, ms"
        << std::setw(20) << "Mutex, ms"
        << std::setw(15) << "Speedup"
        << '\n';

    for (int threadCount = 1; threadCount <= 30; ++threadCount)
    {
        const double lockFreeTime =
            Measure<AtomicMax<std::uint32_t>>(threadCount);

        const double mutexTime =
            Measure<AtomicMaxWithLock<std::uint32_t>>(threadCount);

        const double speedup = mutexTime / lockFreeTime;

        std::cout
            << std::left
            << std::setw(10) << threadCount
            << std::setw(20)
            << std::fixed << std::setprecision(3)
            << lockFreeTime
            << std::setw(20) << mutexTime
            << std::setw(15) << speedup
            << '\n';
    }

    return 0;
}
