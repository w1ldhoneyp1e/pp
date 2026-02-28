#pragma once

#include <string>

class GzipCompressor
{
  public:
    bool CompressOne(const std::string &inputPath,
                     const std::string &gzPath) const;
};
