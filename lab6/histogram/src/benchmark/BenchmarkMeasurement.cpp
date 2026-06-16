#include "benchmark/BenchmarkMeasurement.hpp"

#include "histograms/HistogramUtilities.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <string>

namespace
{
std::atomic<std::uint64_t> g_resultSink{0};
}

void ValidateBenchmarkHistogram(
    const Histogram& histogram,
    const Histogram& reference,
    std::string_view implementation,
    std::size_t threadCount)
{
    if (!AreHistogramsEqual(histogram, reference))
    {
        throw std::runtime_error(
            std::string(implementation) +
            " produced a result different from the reference at " +
            std::to_string(threadCount) + " threads");
    }

    if (!HasValidChannelSums(histogram))
    {
        throw std::runtime_error(
            std::string(implementation) +
            " produced channel sums different from 1");
    }
}

void ConsumeBenchmarkHistogram(const Histogram& histogram) noexcept
{
    std::uint64_t checksum = 0;

    for (std::size_t channel = 0; channel < kChannelCount; ++channel)
    {
        for (std::size_t intensity = 0; intensity < kIntensityCount; ++intensity)
        {
            checksum += static_cast<std::uint64_t>(
                histogram.m_channels[channel][intensity] * 1'000'000.0F) *
                (channel + 1) *
                (intensity + 1);
        }
    }

    g_resultSink.fetch_xor(checksum, std::memory_order_relaxed);
}

BenchmarkStatistics CalculateBenchmarkStatistics(
    std::vector<double> measurements)
{
    if (measurements.empty())
    {
        throw std::invalid_argument(
            "At least one benchmark measurement is required");
    }

    std::sort(measurements.begin(), measurements.end());

    BenchmarkStatistics statistics;
    statistics.m_minMilliseconds = measurements.front();
    statistics.m_maxMilliseconds = measurements.back();
    statistics.m_meanMilliseconds = std::accumulate(
        measurements.begin(),
        measurements.end(),
        0.0) /
        static_cast<double>(measurements.size());

    const std::size_t middle = measurements.size() / 2;

    if (measurements.size() % 2 == 0)
    {
        statistics.m_medianMilliseconds =
            (measurements[middle - 1] + measurements[middle]) / 2.0;
    }
    else
    {
        statistics.m_medianMilliseconds = measurements[middle];
    }

    return statistics;
}
