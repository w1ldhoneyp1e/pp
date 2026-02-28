#pragma once

#include <optional>
#include <string>
#include <vector>

struct ParsedArgs
{
    bool sequential = false;
    int numProcesses = 0;
    std::string archiveName;
    std::vector<std::string> inputFiles;
};

namespace ArgsParser
{
    std::optional<ParsedArgs> Parse(int argc, char **argv);
}
