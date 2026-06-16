#include "ProgramIO.hpp"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace
{
void PrintUsage(const char* executable)
{
    std::cout
        << "Usage:\n"
        << "  " << executable
        << " --image FILE.png [options]\n"
        << "  " << executable
        << " --synthetic WIDTHxHEIGHT [options]\n\n"
        << "Options:\n"
        << "  --pattern random|solid|gradient\n"
        << "  --repetitions N\n"
        << "  --threads 1,2,4,8,16\n"
        << "  --output results/benchmark.csv\n"
        << "  --help\n";
}

std::size_t ParsePositiveSize(
    const std::string& text,
    std::string_view argumentName)
{
    std::size_t parsedCharacters = 0;
    const unsigned long long value = std::stoull(text, &parsedCharacters);

    if (parsedCharacters != text.size() || value == 0)
    {
        throw std::invalid_argument(
            std::string(argumentName) + " must be a positive integer");
    }

    return static_cast<std::size_t>(value);
}

std::pair<std::size_t, std::size_t> ParseDimensions(const std::string& text)
{
    const std::size_t separator = text.find_first_of("xX");

    if (separator == std::string::npos)
    {
        throw std::invalid_argument(
            "Synthetic dimensions must have the form WIDTHxHEIGHT");
    }

    return {
        ParsePositiveSize(text.substr(0, separator), "width"),
        ParsePositiveSize(text.substr(separator + 1), "height")};
}

std::vector<std::size_t> ParseThreadCounts(const std::string& text)
{
    std::vector<std::size_t> result;
    std::stringstream stream(text);
    std::string item;

    while (std::getline(stream, item, ','))
    {
        result.push_back(ParsePositiveSize(item, "thread count"));
    }

    if (result.empty())
    {
        throw std::invalid_argument("At least one thread count is required");
    }

    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
}

SyntheticPattern ParsePattern(const std::string& text)
{
    if (text == "random")
    {
        return SyntheticPattern::Random;
    }

    if (text == "solid")
    {
        return SyntheticPattern::Solid;
    }

    if (text == "gradient")
    {
        return SyntheticPattern::Gradient;
    }

    throw std::invalid_argument("Unknown synthetic pattern: " + text);
}
}

ProgramOptions ParseProgramOptions(int argc, char** argv)
{
    ProgramOptions options;
    bool sourceWasSpecified = false;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        auto RequireValue = [&]() -> std::string
        {
            if (index + 1 >= argc)
            {
                throw std::invalid_argument("Missing value after " + argument);
            }

            ++index;
            return argv[index];
        };

        if (argument == "--help")
        {
            PrintUsage(argv[0]);
            std::exit(0);
        }
        else if (argument == "--image")
        {
            if (sourceWasSpecified)
            {
                throw std::invalid_argument(
                    "Specify either --image or --synthetic, not both");
            }

            options.m_imagePath = RequireValue();
            options.m_useSyntheticImage = false;
            sourceWasSpecified = true;
        }
        else if (argument == "--synthetic")
        {
            if (sourceWasSpecified)
            {
                throw std::invalid_argument(
                    "Specify either --image or --synthetic, not both");
            }

            const auto [width, height] = ParseDimensions(RequireValue());
            options.m_syntheticWidth = width;
            options.m_syntheticHeight = height;
            options.m_useSyntheticImage = true;
            sourceWasSpecified = true;
        }
        else if (argument == "--pattern")
        {
            options.m_pattern = ParsePattern(RequireValue());
        }
        else if (argument == "--repetitions")
        {
            options.m_repetitions = ParsePositiveSize(
                RequireValue(),
                "repetitions");
        }
        else if (argument == "--threads")
        {
            options.m_threadCounts = ParseThreadCounts(RequireValue());
        }
        else if (argument == "--output")
        {
            options.m_outputPath = RequireValue();
        }
        else
        {
            throw std::invalid_argument("Unknown argument: " + argument);
        }
    }

    if (!sourceWasSpecified)
    {
        throw std::invalid_argument(
            "Specify an image with --image or generate one with --synthetic");
    }

    return options;
}

Image LoadInputImage(const ProgramOptions& options)
{
    if (options.m_useSyntheticImage)
    {
        return Image::Generate(
            options.m_syntheticWidth,
            options.m_syntheticHeight,
            options.m_pattern);
    }

    return Image::LoadPng(options.m_imagePath);
}

void SaveBenchmarkResults(
    const std::filesystem::path& outputPath,
    const Image& image,
    std::size_t repetitions,
    const std::vector<BenchmarkResult>& results)
{
    const std::filesystem::path parent = outputPath.parent_path();

    if (!parent.empty())
    {
        std::filesystem::create_directories(parent);
    }

    std::ofstream output(outputPath);

    if (!output)
    {
        throw std::runtime_error(
            "Cannot open output file: " + outputPath.string());
    }

    output
        << "width,height,pixels,implementation,threads,repetitions,"
        << "median_ms,mean_ms,min_ms,max_ms,speedup\n";

    output << std::fixed << std::setprecision(6);

    for (const BenchmarkResult& result : results)
    {
        output
            << image.GetWidth() << ','
            << image.GetHeight() << ','
            << image.GetPixelCount() << ','
            << result.m_implementation << ','
            << result.m_threadCount << ','
            << repetitions << ','
            << result.m_statistics.m_medianMilliseconds << ','
            << result.m_statistics.m_meanMilliseconds << ','
            << result.m_statistics.m_minMilliseconds << ','
            << result.m_statistics.m_maxMilliseconds << ','
            << result.m_speedup << '\n';
    }
}
