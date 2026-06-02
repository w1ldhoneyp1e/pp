#include "App.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>

namespace {
int ParseParticleCount(int argc, char **argv)
{
    if (argc < 2) {
        return 20000;
    }

    return std::clamp(std::atoi(argv[1]), 1, 200000);
}
}

int main(int argc, char **argv)
{
    try {
        App app(static_cast<std::size_t>(ParseParticleCount(argc, argv)));
        app.Run();
    } catch (const cl::Error &error) {
        std::cerr << "OpenCL error: " << error.what() << " (" << error.err() << ")\n";
        return 1;
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
