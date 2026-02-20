#pragma once

#include <string>

std::string BasenameSafe(const std::string& path);
std::string UniqueGzName(const std::string& inputPath, int index);
std::string GetAbsoluteArchivePath(const std::string& archivePath);
