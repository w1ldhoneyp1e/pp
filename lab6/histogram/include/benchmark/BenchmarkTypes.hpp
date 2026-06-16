#pragma once

#include <cstddef>
#include <string>

struct BenchmarkStatistics
{
    double m_medianMilliseconds = 0.0;
    double m_meanMilliseconds = 0.0;
    double m_minMilliseconds = 0.0;
    double m_maxMilliseconds = 0.0;
};

struct BenchmarkResult
{
    std::string m_implementation;
    std::size_t m_threadCount = 1;
    BenchmarkStatistics m_statistics;
    double m_speedup = 1.0;
};
