#pragma once

#include <string>

class TarRunner
{
  public:
    bool Run(const std::string &archivePath,
             const std::string &tempDir) const;
};
