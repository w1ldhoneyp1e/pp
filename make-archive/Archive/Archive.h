#pragma once

#include <string>

struct TarRunner
{
    bool Run(const std::string &archivePath,
             const std::string &tempDir) const;
};
