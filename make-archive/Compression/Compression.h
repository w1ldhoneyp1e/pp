#pragma once

#include <string>

struct GzipCompressor
{
    bool CompressOne(const std::string &inputPath,
                    const std::string &gzPath) const;
};
