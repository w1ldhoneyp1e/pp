#pragma once

#include "Compression/Compression.h"
#include <atomic>
#include <string>
#include <vector>

struct CompressionRunner
{
    bool RunSequential(const std::vector<std::string> &inputFiles,
                      const std::vector<std::string> &gzPaths) const;
    bool RunParallel(const std::vector<std::string> &inputFiles,
                     const std::vector<std::string> &gzPaths,
                     int numProcesses) const;

private:
    bool VerifyGzFiles(const std::vector<std::string> &inputFiles,
                       const std::vector<std::string> &gzPaths) const;
    static void ReportGzipFailure(const std::string &inputPath);
    static size_t ComputeNumWorkers(size_t numProcesses, size_t numFiles);
    void RunWorkerPool(const std::vector<std::string> &inputFiles,
                       const std::vector<std::string> &gzPaths,
                       size_t numWorkers, std::atomic<size_t> &nextIndex,
                       std::atomic<bool> &anyFailed,
                       std::atomic<size_t> &failedIndex) const;

    GzipCompressor compressor_;
};
