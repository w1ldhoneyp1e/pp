#pragma once

#include "benchmark/BenchmarkTypes.hpp"
#include "histograms/HistogramTypes.hpp"

void PrintCorrectnessSummary(const Histogram& reference);
void PrintBenchmarkTableHeader();
void PrintBenchmarkResult(const BenchmarkResult& result);
