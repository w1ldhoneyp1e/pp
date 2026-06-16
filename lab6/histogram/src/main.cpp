#include "benchmark/Benchmark.hpp"
#include "ProgramIO.hpp"

#include <exception>
#include <iostream>
#include <thread>
#include <vector>

int main(int argc, char** argv)
{
    try
    {
        const ProgramOptions options = ParseProgramOptions(argc, argv);

        std::cout << "Loading or generating image outside the benchmark...\n";
        const Image image = LoadInputImage(options);

        std::cout
            << "Image: " << image.GetWidth() << 'x' << image.GetHeight()
            << " (" << image.GetPixelCount() << " pixels)\n"
            << "Hardware concurrency: " << std::thread::hardware_concurrency()
            << "\n"
            << "Repetitions: " << options.m_repetitions << "\n\n";

        const std::vector<BenchmarkResult> results = RunBenchmarks(
            image,
            options.m_repetitions,
            options.m_threadCounts);

        SaveBenchmarkResults(
            options.m_outputPath,
            image,
            options.m_repetitions,
            results);

        std::cout
            << "\nAll implementations passed correctness checks.\n"
            << "Results saved to " << options.m_outputPath << '\n';

        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Error: " << exception.what() << '\n';
        return 1;
    }
}
