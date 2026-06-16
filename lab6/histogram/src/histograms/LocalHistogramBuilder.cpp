#include "histograms/LocalHistogramBuilder.hpp"

#include "histograms/HistogramUtilities.hpp"

#include <cstdint>
#include <stdexcept>

LocalHistogramBuilder::LocalHistogramBuilder(std::size_t threadCount)
    : m_threadCount(threadCount),
      m_partialHistograms(threadCount),
      m_threads(threadCount)
{
    if (threadCount == 0)
    {
        throw std::invalid_argument("Thread count must be greater than zero");
    }
}

Histogram LocalHistogramBuilder::Build(const Image& image)
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
            [this, &pixels, threadIndex, begin, end]()
            {
                HistogramCounts& local = m_partialHistograms[threadIndex];

                for (std::size_t pixelIndex = begin; pixelIndex < end; ++pixelIndex)
                {
                    const std::size_t offset = pixelIndex * kChannelCount;
                    ++local.m_values[GetBlockedHistogramIndex(0, pixels[offset])];
                    ++local.m_values[GetBlockedHistogramIndex(1, pixels[offset + 1])];
                    ++local.m_values[GetBlockedHistogramIndex(2, pixels[offset + 2])];
                }
            });
    }

    JoinThreads(m_threads);
    Merge();
    return NormalizeHistogram(m_mergedCounts, pixelCount);
}

void LocalHistogramBuilder::Reset() noexcept
{
    for (HistogramCounts& histogram : m_partialHistograms)
    {
        histogram.Reset();
    }

    m_mergedCounts.Reset();
}

void LocalHistogramBuilder::Merge()
{
    for (std::size_t index = 0; index < kHistogramValueCount; ++index)
    {
        std::uint64_t sum = 0;

        for (const HistogramCounts& partial : m_partialHistograms)
        {
            sum += partial.m_values[index];
        }

        m_mergedCounts.m_values[index] = static_cast<std::uint32_t>(sum);
    }
}
