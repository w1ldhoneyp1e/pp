#pragma once

#include "benchmark/BenchmarkTypes.hpp"
#include "Image.hpp"

#include <cstddef>
#include <vector>

std::vector<BenchmarkResult> RunBenchmarks(
    const Image& image,
    std::size_t repetitions,
    const std::vector<std::size_t>& threadCounts);
