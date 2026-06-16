#pragma once

#include "ImageTypes.hpp"

#include <cstddef>
#include <filesystem>
#include <vector>

struct ProgramOptions
{
    bool m_useSyntheticImage = false;
    std::filesystem::path m_imagePath;
    std::size_t m_syntheticWidth = 0;
    std::size_t m_syntheticHeight = 0;
    SyntheticPattern m_pattern = SyntheticPattern::Random;
    std::size_t m_repetitions = 5;
    std::vector<std::size_t> m_threadCounts{1, 2, 4, 8, 16};
    std::filesystem::path m_outputPath = "results/benchmark.csv";
};
