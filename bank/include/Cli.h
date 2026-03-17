#pragma once

#include <chrono>

struct CliOptions
{
    bool parallel = false;
    bool verbose = true;
    std::chrono::seconds duration = std::chrono::seconds(5);
};

CliOptions ParseArgs(int argc, char** argv);
