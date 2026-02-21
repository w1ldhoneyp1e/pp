#include "Archive/Archive.h"
#include "Args/Args.h"
#include "CompressionRun/CompressionRun.h"
#include "Path/Path.h"
#include "Temp/Temp.h"
#include "Time/Time.h"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <vector>

static std::string CreateTempDir()
{
    char buf[] = "/tmp/make-archive.XXXXXX";
    if (mkdtemp(buf) == nullptr)
    {
        std::cerr << "make-archive: mkdtemp failed: " << std::strerror(errno)
                  << std::endl;
        return "";
    }
    return std::string(buf);
}

static std::vector<std::string>
BuildGzPaths(const std::string &tempDir,
             const std::vector<std::string> &inputFiles)
{
    std::vector<std::string> gzPaths;
    gzPaths.reserve(inputFiles.size());
    for (size_t i = 0; i < inputFiles.size(); i++)
    {
        gzPaths.push_back(tempDir + "/" +
                          UniqueGzName(inputFiles[i], static_cast<int>(i)));
    }
    return gzPaths;
}

static bool RunCompression(const ParsedArgs &args,
                           const std::vector<std::string> &gzPaths)
{
    if (args.sequential)
    {
        return RunSequentialCompression(args.inputFiles, gzPaths);
    }
    return RunParallelCompression(args.inputFiles, gzPaths, args.numProcesses);
}

struct PipelineResult
{
    bool success;
    double totalTime;
    double tarTime;
};

static PipelineResult RunPipeline(const ParsedArgs &args)
{
    PipelineResult result = {false, 0.0, 0.0};

    std::string tempDir = CreateTempDir();
    if (tempDir.empty())
    {
        return result;
    }

    std::vector<std::string> gzPaths = BuildGzPaths(tempDir, args.inputFiles);

    double startTime = NowMonotonic();
    if (startTime < 0)
    {
        RemoveTempFiles(tempDir, gzPaths);
        return result;
    }

    if (!RunCompression(args, gzPaths))
    {
        RemoveTempFiles(tempDir, gzPaths);
        return result;
    }

    double timeBeforeTar = NowMonotonic();
    if (!RunTar(args.archiveName, tempDir))
    {
        std::cerr << "make-archive: tar failed" << std::endl;
        RemoveTempFiles(tempDir, gzPaths);
        return result;
    }
    double timeAfterTar = NowMonotonic();

    RemoveTempFiles(tempDir, gzPaths);

    double endTime = NowMonotonic();
    result.success = true;
    result.totalTime = endTime - startTime;
    result.tarTime = timeAfterTar - timeBeforeTar;
    return result;
}

static void PrintTiming(double totalTime, double tarTime)
{
    std::cout << "Total time: " << totalTime << " s" << std::endl;
    std::cout << "Sequential section (archive generation): " << tarTime << " s"
              << std::endl;
}

int main(int argc, char **argv)
{
    ParsedArgs args;
    if (!ParseArgs(argc, argv, args))
    {
        return 1;
    }

    PipelineResult result = RunPipeline(args);
    if (!result.success)
    {
        return 1;
    }

    PrintTiming(result.totalTime, result.tarTime);
    return 0;
}
