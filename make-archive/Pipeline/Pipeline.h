#pragma once

#include "../Args/Args.h"

struct PipelineResult
{
    bool success = false;
    double totalTime = 0.0;
    double tarTime = 0.0;

    void PrintTiming() const;
};

class Pipeline
{
  public:
    explicit Pipeline(const ParsedArgs &args);

    PipelineResult Run();

  private:
    static std::vector<std::string>
    BuildGzPaths(const std::string &tempDir,
                 const std::vector<std::string> &inputFiles);

    ParsedArgs args_;
};
