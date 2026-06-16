#include "histograms/HistogramUtilities.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

void HistogramCounts::Reset() noexcept
{
    m_values.fill(0);
}

Histogram NormalizeHistogram(
    const HistogramCounts& counts,
    std::size_t pixelCount)
{
    if (pixelCount == 0)
    {
        throw std::invalid_argument("Cannot normalize an empty image");
    }

    Histogram histogram;
    const float denominator = static_cast<float>(pixelCount);

    for (std::size_t channel = 0; channel < kChannelCount; ++channel)
    {
        for (std::size_t intensity = 0; intensity < kIntensityCount; ++intensity)
        {
            histogram.m_channels[channel][intensity] =
                static_cast<float>(counts.m_values[
                    GetBlockedHistogramIndex(
                        channel,
                        static_cast<std::uint8_t>(intensity))]) /
                denominator;
        }
    }

    return histogram;
}

void CheckHistogramCounterCapacity(std::size_t pixelCount)
{
    if (pixelCount > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::overflow_error(
            "The image has too many pixels for 32-bit histogram counters");
    }
}

void ValidateThreadStorage(
    std::size_t threadCount,
    const std::vector<std::jthread>& threads)
{
    if (threadCount == 0)
    {
        throw std::invalid_argument("Thread count must be greater than zero");
    }

    if (threads.size() != threadCount)
    {
        throw std::invalid_argument(
            "Thread storage size does not match thread count");
    }
}

std::pair<std::size_t, std::size_t> GetPixelRange(
    std::size_t pixelCount,
    std::size_t threadIndex,
    std::size_t threadCount) noexcept
{
    const std::size_t begin = pixelCount * threadIndex / threadCount;
    const std::size_t end = pixelCount * (threadIndex + 1) / threadCount;
    return {begin, end};
}

void JoinThreads(std::vector<std::jthread>& threads)
{
    for (std::jthread& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
}

bool AreHistogramsEqual(
    const Histogram& left,
    const Histogram& right,
    float tolerance) noexcept
{
    for (std::size_t channel = 0; channel < kChannelCount; ++channel)
    {
        for (std::size_t intensity = 0; intensity < kIntensityCount; ++intensity)
        {
            if (std::fabs(
                    left.m_channels[channel][intensity] -
                    right.m_channels[channel][intensity]) > tolerance)
            {
                return false;
            }
        }
    }

    return true;
}

bool HasValidChannelSums(
    const Histogram& histogram,
    float tolerance) noexcept
{
    const std::array<float, kChannelCount> sums = GetChannelSums(histogram);

    return std::all_of(
        sums.begin(),
        sums.end(),
        [tolerance](float sum)
        {
            return std::fabs(sum - 1.0F) <= tolerance;
        });
}

std::array<float, kChannelCount> GetChannelSums(
    const Histogram& histogram) noexcept
{
    std::array<float, kChannelCount> sums{};

    for (std::size_t channel = 0; channel < kChannelCount; ++channel)
    {
        for (float value : histogram.m_channels[channel])
        {
            sums[channel] += value;
        }
    }

    return sums;
}
