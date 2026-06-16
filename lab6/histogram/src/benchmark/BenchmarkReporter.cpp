#include "benchmark/BenchmarkReporter.hpp"

#include "histograms/HistogramUtilities.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <string>

void PrintCorrectnessSummary(const Histogram& reference)
{
    const std::array<float, kChannelCount> sums = GetChannelSums(reference);

    std::cout
        << "Correctness check: R=" << sums[0]
        << ", G=" << sums[1]
        << ", B=" << sums[2] << "\n\n";
}

void PrintBenchmarkTableHeader()
{
    std::cout
        << std::left << std::setw(22) << "implementation"
        << std::right << std::setw(9) << "threads"
        << std::setw(14) << "median, ms"
        << std::setw(14) << "mean, ms"
        << std::setw(12) << "speedup"
        << '\n'
        << std::string(71, '-') << '\n';
}

void PrintBenchmarkResult(const BenchmarkResult& result)
{
    std::cout
        << std::left << std::setw(22) << result.m_implementation
        << std::right << std::setw(9) << result.m_threadCount
        << std::setw(14) << std::fixed << std::setprecision(3)
        << result.m_statistics.m_medianMilliseconds
        << std::setw(14) << result.m_statistics.m_meanMilliseconds
        << std::setw(12) << std::setprecision(3) << result.m_speedup
        << '\n';
}
