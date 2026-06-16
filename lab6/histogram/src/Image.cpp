#include "Image.hpp"

#include <png.h>

#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace
{
std::size_t GetRgbBufferSize(std::size_t width, std::size_t height)
{
    if (width == 0 || height == 0)
    {
        throw std::invalid_argument("Image dimensions must be greater than zero");
    }

    constexpr std::size_t bytesPerPixel = 3;
    const std::size_t maximum = std::numeric_limits<std::size_t>::max();

    if (width > maximum / height || width * height > maximum / bytesPerPixel)
    {
        throw std::overflow_error("Image dimensions are too large");
    }

    return width * height * bytesPerPixel;
}
}

Image Image::LoadPng(const std::filesystem::path& path)
{
    png_image image{};
    image.version = PNG_IMAGE_VERSION;

    const std::string pathString = path.string();
    if (png_image_begin_read_from_file(&image, pathString.c_str()) == 0)
    {
        throw std::runtime_error(
            "Cannot open PNG image '" + pathString + "': " + image.message);
    }

    image.format = PNG_FORMAT_RGB;

    try
    {
        const std::size_t width = image.width;
        const std::size_t height = image.height;
        std::vector<std::uint8_t> pixels(GetRgbBufferSize(width, height));

        if (png_image_finish_read(
                &image,
                nullptr,
                pixels.data(),
                0,
                nullptr) == 0)
        {
            throw std::runtime_error(
                "Cannot decode PNG image '" + pathString + "': " + image.message);
        }

        png_image_free(&image);
        return Image(width, height, std::move(pixels));
    }
    catch (...)
    {
        png_image_free(&image);
        throw;
    }
}

Image Image::Generate(
    std::size_t width,
    std::size_t height,
    SyntheticPattern pattern)
{
    std::vector<std::uint8_t> pixels(GetRgbBufferSize(width, height));

    if (pattern == SyntheticPattern::Random)
    {
        std::mt19937 generator(42U);
        std::uniform_int_distribution<int> distribution(0, 255);

        for (std::uint8_t& value : pixels)
        {
            value = static_cast<std::uint8_t>(distribution(generator));
        }
    }
    else if (pattern == SyntheticPattern::Solid)
    {
        for (std::size_t pixelIndex = 0; pixelIndex < width * height; ++pixelIndex)
        {
            const std::size_t offset = pixelIndex * 3;
            pixels[offset] = 128;
            pixels[offset + 1] = 64;
            pixels[offset + 2] = 192;
        }
    }
    else
    {
        for (std::size_t y = 0; y < height; ++y)
        {
            for (std::size_t x = 0; x < width; ++x)
            {
                const std::size_t offset = (y * width + x) * 3;
                pixels[offset] = static_cast<std::uint8_t>(x % 256);
                pixels[offset + 1] = static_cast<std::uint8_t>(y % 256);
                pixels[offset + 2] = static_cast<std::uint8_t>((x + y) % 256);
            }
        }
    }

    return Image(width, height, std::move(pixels));
}

std::size_t Image::GetWidth() const noexcept
{
    return m_width;
}

std::size_t Image::GetHeight() const noexcept
{
    return m_height;
}

std::size_t Image::GetPixelCount() const noexcept
{
    return m_width * m_height;
}

const std::vector<std::uint8_t>& Image::GetPixels() const noexcept
{
    return m_pixels;
}

Image::Image(
    std::size_t width,
    std::size_t height,
    std::vector<std::uint8_t> pixels)
    : m_width(width),
      m_height(height),
      m_pixels(std::move(pixels))
{
}
