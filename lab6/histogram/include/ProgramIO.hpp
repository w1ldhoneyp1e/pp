#pragma once

#include "Image.hpp"
#include "benchmark/BenchmarkTypes.hpp"
#include "ProgramOptions.hpp"

#include <filesystem>
#include <vector>

ProgramOptions ParseProgramOptions(int argc, char** argv);
Image LoadInputImage(const ProgramOptions& options);

void SaveBenchmarkResults(
    const std::filesystem::path& outputPath,
    const Image& image,
    std::size_t repetitions,
    const std::vector<BenchmarkResult>& results);
