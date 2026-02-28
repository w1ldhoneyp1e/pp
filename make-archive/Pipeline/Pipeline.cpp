#include "Pipeline.h"
#include "../Archive/Archive.h"
#include "../CompressionRun/CompressionRun.h"
#include "../Path/Path.h"
#include "../Temp/Temp.h"
#include "../Time/Time.h"
#include <iostream>
#include <vector>

void PipelineResult::PrintTiming() const
{
    std::cout << "Total time: " << totalTime << " s" << std::endl;
    std::cout << "Sequential section (archive generation): " << tarTime << " s"
              << std::endl;
}

Pipeline::Pipeline(const ParsedArgs &args) : args_(args) {}

PipelineResult Pipeline::Run()
{
    PipelineResult result;

    auto tempOpt = TempDir::Create();
    if (!tempOpt)
    {
        return result;
    }
    TempDir &tempDir = *tempOpt;

    std::vector<std::string> gzPaths =
        BuildGzPaths(tempDir.Path(), args_.inputFiles);

    double startTime = Clock::Now();
    if (startTime < 0)
    {
        tempDir.Remove(gzPaths);
        return result;
    }

    CompressionRunner runner;
    bool compressionOk =
        args_.sequential
            ? runner.RunSequential(args_.inputFiles, gzPaths)
            : runner.RunParallel(args_.inputFiles, gzPaths, args_.numProcesses);
    if (!compressionOk)
    {
        tempDir.Remove(gzPaths);
        return result;
    }

    double timeBeforeTar = Clock::Now();
    TarRunner tarRunner;
    if (!tarRunner.Run(args_.archiveName, tempDir.Path()))
    {
        std::cerr << "make-archive: tar failed" << std::endl;
        tempDir.Remove(gzPaths);
        return result;
    }
    double timeAfterTar = Clock::Now();

    tempDir.Remove(gzPaths);

    double endTime = Clock::Now();
    result.success = true;
    result.totalTime = endTime - startTime;
    result.tarTime = timeAfterTar - timeBeforeTar;
    return result;
}

std::vector<std::string>
Pipeline::BuildGzPaths(const std::string &tempDir,
                       const std::vector<std::string> &inputFiles)
{
    std::vector<std::string> gzPaths;
    gzPaths.reserve(inputFiles.size());
    for (size_t i = 0; i < inputFiles.size(); i++)
    {
        gzPaths.push_back(
            tempDir + "/" +
            PathHelper::UniqueGzName(inputFiles[i], static_cast<int>(i)));
    }
    return gzPaths;
}
