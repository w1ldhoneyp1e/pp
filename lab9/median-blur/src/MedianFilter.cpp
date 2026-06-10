#include "MedianFilter.h"

#include "OpenCLUtils.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>

MedianFilter::MedianFilter(const std::string &kernelPath)
    : m_device(SelectGpuDevice()),
      m_context(m_device),
      m_queue(m_context, m_device, CL_QUEUE_PROFILING_ENABLE),
      m_program(BuildProgram(m_context, m_device, LoadTextFile(kernelPath))),
      m_kernel(m_program, "median_blur"),
      m_deviceName(m_device.getInfo<CL_DEVICE_NAME>())
{
}

Image MedianFilter::Apply(const Image &source, int radius)
{
    if (source.Width <= 0 || source.Height <= 0 || source.Pixels.empty()) {
        throw std::runtime_error("Source image is empty");
    }

    radius = std::clamp(radius, 1, MaxRadius);

    Image result;
    result.Width = source.Width;
    result.Height = source.Height;
    result.Pixels.resize(source.Pixels.size());

    const std::size_t byteSize = source.Pixels.size() * sizeof(std::uint8_t);
    cl::Buffer input(m_context,
                     CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                     byteSize,
                     const_cast<std::uint8_t *>(source.Pixels.data()));
    cl::Buffer output(m_context, CL_MEM_WRITE_ONLY, byteSize);

    const std::size_t tileWidth = LocalWidth + static_cast<std::size_t>(radius) * 2;
    const std::size_t tileHeight = LocalHeight + static_cast<std::size_t>(radius) * 2;
    const std::size_t tileBytes = tileWidth * tileHeight * 4;
    const std::size_t windowBytes = static_cast<std::size_t>(radius * 2 + 1)
                                  * static_cast<std::size_t>(radius * 2 + 1)
                                  * sizeof(cl_int2);

    int arg = 0;
    m_kernel.setArg(arg++, input);
    m_kernel.setArg(arg++, output);
    m_kernel.setArg(arg++, source.Width);
    m_kernel.setArg(arg++, source.Height);
    m_kernel.setArg(arg++, radius);
    m_kernel.setArg(arg++, cl::Local(tileBytes));
    m_kernel.setArg(arg++, cl::Local(windowBytes));

    const std::size_t globalX = ((static_cast<std::size_t>(source.Width) + LocalWidth - 1) / LocalWidth) * LocalWidth;
    const std::size_t globalY = ((static_cast<std::size_t>(source.Height) + LocalHeight - 1) / LocalHeight) * LocalHeight;

    m_queue.enqueueNDRangeKernel(m_kernel,
                                 cl::NullRange,
                                 cl::NDRange(globalX, globalY),
                                 cl::NDRange(LocalWidth, LocalHeight));
    m_queue.enqueueReadBuffer(output, CL_TRUE, 0, byteSize, result.Pixels.data());

    return result;
}
