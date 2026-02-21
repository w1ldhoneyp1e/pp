#define _POSIX_C_SOURCE 200809L

#include "Args.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>

bool ParseArgs(int argc, char **argv, ParsedArgs &out)
{
    int opt;
    while ((opt = getopt(argc, argv, "SP:")) != -1)
    {
        switch (opt)
        {
        case 'S':
            out.sequential = true;
            break;
        case 'P':
            out.numProcesses = atoi(optarg);
            if (out.numProcesses <= 0)
            {
                std::cerr << "make-archive: -P requires positive number\n";
                return false;
            }
            break;
        default:
            std::cerr << "Usage: make-archive -S ARCHIVE-NAME [INPUT-FILES]\n"
                      << "       make-archive -P NUM-PROCESSES ARCHIVE-NAME "
                         "[INPUT-FILES]\n";
            return false;
        }
    }

    if (!out.sequential && out.numProcesses == 0)
    {
        std::cerr << "make-archive: specify -S or -P NUM-PROCESSES\n";
        return false;
    }
    if (out.sequential && out.numProcesses != 0)
    {
        std::cerr << "make-archive: use either -S or -P, not both\n";
        return false;
    }

    if (optind >= argc)
    {
        std::cerr << "make-archive: ARCHIVE-NAME required\n";
        return false;
    }

    out.archiveName = argv[optind++];
    for (int i = optind; i < argc; i++)
    {
        out.inputFiles.push_back(argv[i]);
    }

    if (out.inputFiles.empty())
    {
        std::cerr << "make-archive: at least one INPUT-FILE required\n";
        return false;
    }

    return true;
}
