#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Image
{
    int Width = 0;
    int Height = 0;
    std::vector<std::uint8_t> Pixels;
};

Image LoadImage(const std::string &path);
