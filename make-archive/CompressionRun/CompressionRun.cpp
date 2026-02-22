#include "CompressionRun.h"
#include <iostream>
#include <sys/stat.h>
#include <thread>

bool CompressionRunner::VerifyGzFiles(
    const std::vector<std::string> &inputFiles,
    const std::vector<std::string> &gzPaths) const
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

void CompressionRunner::ReportGzipFailure(const std::string &inputPath)
{
    std::cerr << "make-archive: gzip failed for " << inputPath << std::endl;
}

size_t CompressionRunner::ComputeNumWorkers(size_t numProcesses, size_t numFiles)
{
    size_t numWorkers = numProcesses;
    if (numWorkers > numFiles)
    {
        numWorkers = numFiles;
    }
    return numWorkers;
}

void CompressionRunner::RunWorkerPool(
    const std::vector<std::string> &inputFiles,
    const std::vector<std::string> &gzPaths, size_t numWorkers,
    std::atomic<size_t> &nextIndex, std::atomic<bool> &anyFailed,
    std::atomic<size_t> &failedIndex) const
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
            if (!compressor_.CompressOne(inputFiles[i], gzPaths[i]))
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

bool CompressionRunner::RunSequential(
    const std::vector<std::string> &inputFiles,
    const std::vector<std::string> &gzPaths) const
{
    for (size_t i = 0; i < inputFiles.size(); i++)
    {
        if (!compressor_.CompressOne(inputFiles[i], gzPaths[i]))
        {
            ReportGzipFailure(inputFiles[i]);
            return false;
        }
    }
    return true;
}

bool CompressionRunner::RunParallel(
    const std::vector<std::string> &inputFiles,
    const std::vector<std::string> &gzPaths, int numProcesses) const
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
