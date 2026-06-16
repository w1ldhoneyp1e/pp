#include "histograms/AtomicBlockedHistogramBuilder.hpp"

#include "histograms/HistogramUtilities.hpp"

#include <stdexcept>

AtomicBlockedHistogramBuilder::AtomicBlockedHistogramBuilder(
    std::size_t threadCount)
    : m_threadCount(threadCount),
      m_threads(threadCount)
{
    if (threadCount == 0)
    {
        throw std::invalid_argument("Thread count must be greater than zero");
    }

    Reset();
}

Histogram AtomicBlockedHistogramBuilder::Build(const Image& image)
{
    CheckHistogramCounterCapacity(image.GetPixelCount());
    ValidateThreadStorage(m_threadCount, m_threads);
    Reset();

    const std::vector<std::uint8_t>& pixels = image.GetPixels();
    const std::size_t pixelCount = image.GetPixelCount();

    for (std::size_t threadIndex = 0; threadIndex < m_threadCount; ++threadIndex)
    {
        const auto [begin, end] = GetPixelRange(
            pixelCount,
            threadIndex,
            m_threadCount);

        m_threads[threadIndex] = std::jthread(
            [this, &pixels, begin, end]()
            {
                for (std::size_t pixelIndex = begin; pixelIndex < end; ++pixelIndex)
                {
                    const std::size_t offset = pixelIndex * kChannelCount;
                    Increment(0, pixels[offset]);
                    Increment(1, pixels[offset + 1]);
                    Increment(2, pixels[offset + 2]);
                }
            });
    }

    JoinThreads(m_threads);
    return NormalizeHistogram(CopyCounts(), pixelCount);
}

void AtomicBlockedHistogramBuilder::Reset() noexcept
{
    for (std::atomic<std::uint32_t>& value : m_counts)
    {
        value.store(0, std::memory_order_relaxed);
    }
}

void AtomicBlockedHistogramBuilder::Increment(
    std::size_t channel,
    std::uint8_t intensity) noexcept
{
    m_counts[GetBlockedHistogramIndex(channel, intensity)].fetch_add(
        1,
        std::memory_order_relaxed);
}

HistogramCounts AtomicBlockedHistogramBuilder::CopyCounts() const noexcept
{
    HistogramCounts result;

    for (std::size_t index = 0; index < kHistogramValueCount; ++index)
    {
        result.m_values[index] = m_counts[index].load(std::memory_order_relaxed);
    }

    return result;
}
