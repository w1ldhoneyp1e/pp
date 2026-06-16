#pragma once

#include "histograms/HistogramTypes.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <thread>
#include <utility>
#include <vector>

constexpr std::size_t GetBlockedHistogramIndex(
    std::size_t channel,
    std::uint8_t intensity) noexcept
{
    return channel * kIntensityCount + intensity;
}

constexpr std::size_t GetInterleavedHistogramIndex(
    std::size_t channel,
    std::uint8_t intensity) noexcept
{
    return static_cast<std::size_t>(intensity) * kChannelCount + channel;
}

Histogram NormalizeHistogram(
    const HistogramCounts& counts,
    std::size_t pixelCount);

void CheckHistogramCounterCapacity(std::size_t pixelCount);

void ValidateThreadStorage(
    std::size_t threadCount,
    const std::vector<std::jthread>& threads);

std::pair<std::size_t, std::size_t> GetPixelRange(
    std::size_t pixelCount,
    std::size_t threadIndex,
    std::size_t threadCount) noexcept;

void JoinThreads(std::vector<std::jthread>& threads);

bool AreHistogramsEqual(
    const Histogram& left,
    const Histogram& right,
    float tolerance = 1.0e-6F) noexcept;

bool HasValidChannelSums(
    const Histogram& histogram,
    float tolerance = 1.0e-4F) noexcept;

std::array<float, kChannelCount> GetChannelSums(
    const Histogram& histogram) noexcept;
