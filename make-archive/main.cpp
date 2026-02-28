#include "./Pipeline/Pipeline.h"
#include "Args/Args.h"

int main(int argc, char **argv)
{
    auto argsOpt = ArgsParser::Parse(argc, argv);
    if (!argsOpt)
    {
        return 1;
    }

    Pipeline pipeline(*argsOpt);
    PipelineResult result = pipeline.Run();
    if (!result.success)
    {
        return 1;
    }

    result.PrintTiming();
    return 0;
}
