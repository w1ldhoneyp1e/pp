#pragma once

#include "ImageTypes.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <vector>


class Image
{
public:
    static Image LoadPng(const std::filesystem::path& path);
    static Image Generate(
        std::size_t width,
        std::size_t height,
        SyntheticPattern pattern);

    std::size_t GetWidth() const noexcept;
    std::size_t GetHeight() const noexcept;
    std::size_t GetPixelCount() const noexcept;
    const std::vector<std::uint8_t>& GetPixels() const noexcept;

private:
    Image(
        std::size_t width,
        std::size_t height,
        std::vector<std::uint8_t> pixels);

    std::size_t m_width;
    std::size_t m_height;
    std::vector<std::uint8_t> m_pixels;
};
