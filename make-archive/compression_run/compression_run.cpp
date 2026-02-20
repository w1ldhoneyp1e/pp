#define _POSIX_C_SOURCE 200809L

#include "compression_run.h"
#include "../compression/compression.h"
#include <atomic>
#include <iostream>
#include <sys/stat.h>
#include <thread>
#include <vector>

static bool VerifyGzFiles(const std::vector<std::string> &inputFiles,
                          const std::vector<std::string> &gzPaths)
{
    for (size_t i = 0; i < gzPaths.size(); i++)
    {
        struct stat st;
        if (stat(gzPaths[i].c_str(), &st) != 0 || st.st_size == 0)
        {
            std::cerr << "make-archive: compression failed for "
                      << inputFiles[i] << std::endl;
            return false;
        }
    }
    return true;
}

static void ReportGzipFailure(const std::string &inputPath)
{
    std::cerr << "make-archive: gzip failed for " << inputPath << std::endl;
}

bool RunSequentialCompression(const std::vector<std::string> &inputFiles,
                              const std::vector<std::string> &gzPaths)
{
    for (size_t i = 0; i < inputFiles.size(); i++)
    {
        if (!CompressOne(inputFiles[i].c_str(), gzPaths[i].c_str()))
        {
            ReportGzipFailure(inputFiles[i]);
            return false;
        }
    }
    return true;
}

static size_t ComputeNumWorkers(size_t numProcesses, size_t numFiles)
{
    size_t numWorkers = numProcesses;
    if (numWorkers > numFiles)
    {
        numWorkers = numFiles;
    }
    return numWorkers;
}

static void RunWorkerPool(const std::vector<std::string> &inputFiles,
                          const std::vector<std::string> &gzPaths,
                          size_t numWorkers, std::atomic<size_t> &nextIndex,
                          std::atomic<bool> &anyFailed,
                          std::atomic<size_t> &failedIndex)
{
    auto worker = [&]()
    {
        while (true)
        {
            size_t i = nextIndex++;
            if (i >= inputFiles.size())
            {
                break;
            }
            if (!CompressOne(inputFiles[i].c_str(), gzPaths[i].c_str()))
            {
                anyFailed = true;
                size_t expected = inputFiles.size();
                failedIndex.compare_exchange_strong(expected, i);
            }
        }
    };

    std::vector<std::jthread> threads;
    threads.reserve(numWorkers);
    for (size_t t = 0; t < numWorkers; t++)
    {
        threads.emplace_back(worker);
    }
    threads.clear();
}

bool RunParallelCompression(const std::vector<std::string> &inputFiles,
                            const std::vector<std::string> &gzPaths,
                            int numProcesses)
{
    std::atomic<size_t> nextIndex(0);
    std::atomic<bool> anyFailed(false);
    std::atomic<size_t> failedIndex(inputFiles.size());

    size_t numWorkers =
        ComputeNumWorkers(static_cast<size_t>(numProcesses), inputFiles.size());

    RunWorkerPool(inputFiles, gzPaths, numWorkers, nextIndex, anyFailed,
                  failedIndex);

    if (anyFailed)
    {
        size_t idx = failedIndex.load();
        ReportGzipFailure(inputFiles[idx]);
        return false;
    }

    return VerifyGzFiles(inputFiles, gzPaths);
}
