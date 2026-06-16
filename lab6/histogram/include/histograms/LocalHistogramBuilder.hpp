#pragma once

#include "histograms/HistogramTypes.hpp"
#include "Image.hpp"

#include <cstddef>
#include <thread>
#include <vector>

class LocalHistogramBuilder
{
public:
    explicit LocalHistogramBuilder(std::size_t threadCount);

    Histogram Build(const Image& image);

private:
    void Reset() noexcept;
    void Merge();

    std::size_t m_threadCount;
    std::vector<HistogramCounts> m_partialHistograms;
    HistogramCounts m_mergedCounts;
    std::vector<std::jthread> m_threads;
};
