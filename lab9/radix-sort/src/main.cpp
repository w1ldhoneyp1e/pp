#include "RadixSorter.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <execution>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

struct Options
{
    std::size_t Count = 1'000'000;
    std::uint32_t Seed = 42;
};

Options ParseOptions(int argc, char **argv)
{
    Options options;

    if (argc > 1)
    {
        options.Count = std::stoull(argv[1]);
    }
    if (argc > 2)
    {
        options.Seed = static_cast<std::uint32_t>(std::stoul(argv[2]));
    }

    return options;
}

std::vector<std::int32_t> GenerateValues(std::size_t count, std::uint32_t seed)
{
    std::mt19937 generator(seed);
    std::uniform_int_distribution<std::int32_t> distribution(
        std::numeric_limits<std::int32_t>::min(),
        std::numeric_limits<std::int32_t>::max());

    std::vector<std::int32_t> values(count);
    for (auto &value : values)
    {
        value = distribution(generator);
    }

    return values;
}

template <typename Func>
double MeasureMilliseconds(Func &&func)
{
    const auto start = std::chrono::steady_clock::now();
    func();
    const auto finish = std::chrono::steady_clock::now();

    return std::chrono::duration<double, std::milli>(finish - start).count();
}

int main(int argc, char **argv)
{
    try
    {
        const Options options = ParseOptions(argc, argv);
        const std::vector<std::int32_t> values = GenerateValues(options.Count, options.Seed);

        RadixSorter sorter;

        std::vector<std::int32_t> gpuSorted;
        const double gpuMs = MeasureMilliseconds([&]()
        {
            gpuSorted = sorter.Sort(values);
        });

        std::vector<std::int32_t> cpuSorted = values;
        const double cpuMs = MeasureMilliseconds([&]()
        {
            std::sort(std::execution::par, cpuSorted.begin(), cpuSorted.end());
        });

        if (gpuSorted != cpuSorted)
        {
            throw std::runtime_error("GPU result differs from CPU result");
        }

        std::cout << "Device: " << sorter.DeviceName() << '\n';
        std::cout << "Elements: " << options.Count << '\n';
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "GPU Radix Sort: " << gpuMs << " ms\n";
        std::cout << "CPU parallel std::sort: " << cpuMs << " ms\n";
        if (gpuMs > 0.0)
        {
            std::cout << "CPU/GPU ratio: " << cpuMs / gpuMs << '\n';
        }
        std::cout << "Check: OK\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
