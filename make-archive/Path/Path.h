#pragma once

#include <string>

struct PathHelper
{
    static std::string BasenameSafe(const std::string &path);
    static std::string UniqueGzName(const std::string &inputPath, int index);
    static std::string GetAbsoluteArchivePath(const std::string &archivePath);
};
