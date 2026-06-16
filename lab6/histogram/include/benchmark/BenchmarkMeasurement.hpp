#pragma once

#include "benchmark/BenchmarkTypes.hpp"
#include "histograms/HistogramTypes.hpp"

#include <chrono>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

void ValidateBenchmarkHistogram(
    const Histogram& histogram,
    const Histogram& reference,
    std::string_view implementation,
    std::size_t threadCount);

void ConsumeBenchmarkHistogram(const Histogram& histogram) noexcept;

BenchmarkStatistics CalculateBenchmarkStatistics(
    std::vector<double> measurements);

template <typename Operation>
BenchmarkStatistics MeasureBenchmark(
    std::size_t repetitions,
    Operation&& operation,
    const Histogram& reference,
    std::string_view implementation,
    std::size_t threadCount)
{
    Histogram warmupResult = operation();
    ValidateBenchmarkHistogram(
        warmupResult,
        reference,
        implementation,
        threadCount);
    ConsumeBenchmarkHistogram(warmupResult);

    std::vector<double> measurements;
    measurements.reserve(repetitions);

    for (std::size_t repetition = 0; repetition < repetitions; ++repetition)
    {
        const auto start = std::chrono::steady_clock::now();
        Histogram result = operation();
        const auto finish = std::chrono::steady_clock::now();

        ValidateBenchmarkHistogram(
            result,
            reference,
            implementation,
            threadCount);
        ConsumeBenchmarkHistogram(result);

        const double milliseconds =
            std::chrono::duration<double, std::milli>(finish - start).count();
        measurements.push_back(milliseconds);
    }

    return CalculateBenchmarkStatistics(std::move(measurements));
}
