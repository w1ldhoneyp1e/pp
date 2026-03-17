#include "../include/Cli.h"

#include <stdexcept>
#include <string>

CliOptions ParseArgs(int argc, char **argv)
{
    CliOptions options;
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];
        if (arg == "--mode" && i + 1 < argc)
        {
            const std::string mode = argv[++i];
            if (mode == "single")
            {
                options.parallel = false;
                continue;
            }

            if (mode == "parallel")
            {
                options.parallel = true;
                continue;
            }

            throw std::invalid_argument("Unknown mode: " + mode);
        }

        if (arg == "--seconds" && i + 1 < argc)
        {
            const int seconds = std::stoi(argv[++i]);
            if (seconds <= 0)
            {
                throw std::out_of_range("seconds must be positive");
            }

            options.duration = std::chrono::seconds(seconds);
            continue;
        }

        if (arg == "--quiet")
        {
            options.verbose = false;
            continue;
        }

        throw std::invalid_argument("Unknown argument: " + arg);
    }

    return options;
}
