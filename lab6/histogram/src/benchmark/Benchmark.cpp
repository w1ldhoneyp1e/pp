#include "benchmark/Benchmark.hpp"

#include "histograms/AtomicBlockedHistogramBuilder.hpp"
#include "histograms/AtomicInterleavedHistogramBuilder.hpp"
#include "benchmark/BenchmarkMeasurement.hpp"
#include "benchmark/BenchmarkReporter.hpp"
#include "histograms/HistogramUtilities.hpp"
#include "histograms/LocalHistogramBuilder.hpp"
#include "histograms/SingleThreadHistogramBuilder.hpp"

#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
template <typename Builder>
BenchmarkResult RunBenchmarkCase(
    Builder& builder,
    const Image& image,
    std::size_t repetitions,
    const Histogram& reference,
    std::string_view implementation,
    std::size_t threadCount,
    double baselineMilliseconds)
{
    BenchmarkResult result;
    result.m_implementation = implementation;
    result.m_threadCount = threadCount;
    result.m_statistics = MeasureBenchmark(
        repetitions,
        [&builder, &image]()
        {
            return builder.Build(image);
        },
        reference,
        implementation,
        threadCount);

    if (baselineMilliseconds > 0.0)
    {
        result.m_speedup =
            baselineMilliseconds / result.m_statistics.m_medianMilliseconds;
    }

    return result;
}

void AddAndPrintResult(
    std::vector<BenchmarkResult>& results,
    BenchmarkResult result)
{
    PrintBenchmarkResult(result);
    results.push_back(std::move(result));
}
}

std::vector<BenchmarkResult> RunBenchmarks(
    const Image& image,
    std::size_t repetitions,
    const std::vector<std::size_t>& threadCounts)
{
    SingleThreadHistogramBuilder referenceBuilder;
    const Histogram reference = referenceBuilder.Build(image);

    if (!HasValidChannelSums(reference))
    {
        throw std::runtime_error(
            "The reference implementation produced invalid channel sums");
    }

    PrintCorrectnessSummary(reference);
    PrintBenchmarkTableHeader();

    std::vector<BenchmarkResult> results;
    results.reserve(1 + threadCounts.size() * 3);

    SingleThreadHistogramBuilder singleThreadBuilder;
    BenchmarkResult singleResult = RunBenchmarkCase(
        singleThreadBuilder,
        image,
        repetitions,
        reference,
        "single_thread",
        1,
        0.0);
    singleResult.m_speedup = 1.0;

    const double baselineMilliseconds =
        singleResult.m_statistics.m_medianMilliseconds;
    AddAndPrintResult(results, std::move(singleResult));

    for (std::size_t threadCount : threadCounts)
    {
        AtomicInterleavedHistogramBuilder interleavedBuilder(threadCount);
        AddAndPrintResult(
            results,
            RunBenchmarkCase(
                interleavedBuilder,
                image,
                repetitions,
                reference,
                "atomic_interleaved",
                threadCount,
                baselineMilliseconds));

        AtomicBlockedHistogramBuilder blockedBuilder(threadCount);
        AddAndPrintResult(
            results,
            RunBenchmarkCase(
                blockedBuilder,
                image,
                repetitions,
                reference,
                "atomic_blocked",
                threadCount,
                baselineMilliseconds));

        LocalHistogramBuilder localBuilder(threadCount);
        AddAndPrintResult(
            results,
            RunBenchmarkCase(
                localBuilder,
                image,
                repetitions,
                reference,
                "local_histograms",
                threadCount,
                baselineMilliseconds));
    }

    return results;
}
