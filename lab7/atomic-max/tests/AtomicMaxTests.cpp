#include "AtomicMax.h"
#include "AtomicMaxWithLock.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <thread>
#include <vector>

namespace
{

void TestInitialValue()
{
    AtomicMax<int> maximum(42);

    assert(maximum.GetValue() == 42);
}

void TestSmallerValueDoesNotReplaceMaximum()
{
    AtomicMax<int> maximum(42);

    maximum.Update(10);

    assert(maximum.GetValue() == 42);
}

void TestLargerValueReplacesMaximum()
{
    AtomicMax<int> maximum(42);

    maximum.Update(100);

    assert(maximum.GetValue() == 100);
}

void TestNegativeValues()
{
    AtomicMax<int> maximum(-100);

    maximum.Update(-20);
    maximum.Update(-50);

    assert(maximum.GetValue() == -20);
}

void TestConcurrentUpdates()
{
    constexpr int threadCount = 16;
    constexpr int valuesPerThread = 10000;

    AtomicMax<int> maximum(std::numeric_limits<int>::lowest());
    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    int expectedMaximum = std::numeric_limits<int>::lowest();

    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back(
            [&maximum, threadIndex]()
            {
                for (int valueIndex = 0;
                     valueIndex < valuesPerThread;
                     ++valueIndex)
                {
                    const int value =
                        threadIndex * valuesPerThread + valueIndex;

                    maximum.Update(value);
                }
            });

        expectedMaximum = std::max(
            expectedMaximum,
            (threadIndex + 1) * valuesPerThread - 1);
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    assert(maximum.GetValue() == expectedMaximum);
}

void TestConcurrentUpdatesWithRepeatedValues()
{
    constexpr int threadCount = 12;
    constexpr int valuesPerThread = 5000;
    constexpr int expectedMaximum = 999;

    AtomicMax<int> maximum(0);
    std::vector<std::thread> threads;
    threads.reserve(threadCount);

    for (int threadIndex = 0; threadIndex < threadCount; ++threadIndex)
    {
        threads.emplace_back(
            [&maximum, threadIndex]()
            {
                for (int valueIndex = 0;
                     valueIndex < valuesPerThread;
                     ++valueIndex)
                {
                    const int value =
                        (valueIndex * 37 + threadIndex * 17) % 1000;

                    maximum.Update(value);
                }
            });
    }

    for (std::thread& thread : threads)
    {
        thread.join();
    }

    assert(maximum.GetValue() == expectedMaximum);
}

void TestLockBasedVersion()
{
    AtomicMaxWithLock<int> maximum(5);

    maximum.Update(3);
    maximum.Update(20);
    maximum.Update(10);

    assert(maximum.GetValue() == 20);
}

}

int main()
{
    TestInitialValue();
    TestSmallerValueDoesNotReplaceMaximum();
    TestLargerValueReplacesMaximum();
    TestNegativeValues();
    TestConcurrentUpdates();
    TestConcurrentUpdatesWithRepeatedValues();
    TestLockBasedVersion();

    std::cout << "All AtomicMax tests passed\n";
    return 0;
}
