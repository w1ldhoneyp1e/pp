#include "histograms/SingleThreadHistogramBuilder.hpp"

#include "histograms/HistogramUtilities.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

Histogram SingleThreadHistogramBuilder::Build(const Image& image)
{
    CheckHistogramCounterCapacity(image.GetPixelCount());
    m_counts.Reset();

    const std::vector<std::uint8_t>& pixels = image.GetPixels();
    const std::size_t pixelCount = image.GetPixelCount();

    for (std::size_t pixelIndex = 0; pixelIndex < pixelCount; ++pixelIndex)
    {
        const std::size_t offset = pixelIndex * kChannelCount;
        ++m_counts.m_values[GetBlockedHistogramIndex(0, pixels[offset])];
        ++m_counts.m_values[GetBlockedHistogramIndex(1, pixels[offset + 1])];
        ++m_counts.m_values[GetBlockedHistogramIndex(2, pixels[offset + 2])];
    }

    return NormalizeHistogram(m_counts, pixelCount);
}
