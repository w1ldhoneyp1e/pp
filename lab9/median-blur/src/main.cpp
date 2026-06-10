#include "App.h"
#include "Image.h"
#include "OpenCLHeaders.h"

#include <exception>
#include <iostream>

namespace {
void PrintUsage(const char *program)
{
    std::cerr << "Usage: " << program << " <image.png|image.jpg|image.jpeg>\n";
}
}

int main(int argc, char **argv)
{
    try {
        if (argc != 2) {
            PrintUsage(argv[0]);
            return 2;
        }

        Image image = LoadImage(argv[1]);
        App app(std::move(image));
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
