#pragma once

#include <string>
#include <vector>

bool RunSequentialCompression(const std::vector<std::string> &inputFiles,
                              const std::vector<std::string> &gzPaths);

bool RunParallelCompression(const std::vector<std::string> &inputFiles,
                            const std::vector<std::string> &gzPaths,
                            int numProcesses);
