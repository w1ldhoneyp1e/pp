#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr std::size_t kChannelCount = 3;
constexpr std::size_t kIntensityCount = 256;
constexpr std::size_t kHistogramValueCount = kChannelCount * kIntensityCount;

struct Histogram
{
    std::array<std::array<float, kIntensityCount>, kChannelCount> m_channels{};
};

struct alignas(64) HistogramCounts
{
    void Reset() noexcept;

    std::array<std::uint32_t, kHistogramValueCount> m_values{};
};
