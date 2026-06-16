#pragma once

#include "histograms/HistogramTypes.hpp"
#include "Image.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <vector>

class AtomicBlockedHistogramBuilder
{
public:
    explicit AtomicBlockedHistogramBuilder(std::size_t threadCount);

    Histogram Build(const Image& image);

private:
    void Reset() noexcept;
    void Increment(std::size_t channel, std::uint8_t intensity) noexcept;
    HistogramCounts CopyCounts() const noexcept;

    std::size_t m_threadCount;
    std::array<std::atomic<std::uint32_t>, kHistogramValueCount> m_counts;
    std::vector<std::jthread> m_threads;
};
