#pragma once

#include <string>
#include <vector>

struct ParsedArgs
{
    bool sequential = false;
    int numProcesses = 0;
    std::string archiveName;
    std::vector<std::string> inputFiles;
};

bool ParseArgs(int argc, char **argv, ParsedArgs &out);
