#pragma once

#include "histograms/HistogramTypes.hpp"
#include "Image.hpp"

class SingleThreadHistogramBuilder
{
public:
    Histogram Build(const Image& image);

private:
    HistogramCounts m_counts;
};
