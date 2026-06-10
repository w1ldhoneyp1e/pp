#pragma once

#include "OpenCLHeaders.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class RadixSorter
{
public:
    RadixSorter();

    std::vector<std::int32_t> Sort(const std::vector<std::int32_t> &values);
    std::string DeviceName() const;

private:
    static constexpr std::size_t m_radix = 256;
    static constexpr std::size_t m_blockSize = 256;
    static constexpr std::size_t m_passes = 4;

    void BuildOffsets(const std::vector<cl_uint> &histograms,
                      std::size_t blockCount,
                      std::vector<cl_uint> &offsets) const;

    cl::Device m_device;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    cl::Kernel m_histogramKernel;
    cl::Kernel m_scatterKernel;
};
