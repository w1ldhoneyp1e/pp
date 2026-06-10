#pragma once

#include "Image.h"
#include "OpenCLHeaders.h"

#include <string>
#include <vector>

class MedianFilter
{
public:
    explicit MedianFilter(const std::string &kernelPath);

    const std::string &DeviceName() const { return m_deviceName; }
    Image Apply(const Image &source, int radius);

private:
    static constexpr int LocalWidth = 16;
    static constexpr int LocalHeight = 16;
    static constexpr int MaxRadius = 7;

    cl::Device m_device;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    cl::Kernel m_kernel;
    std::string m_deviceName;
};
