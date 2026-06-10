#include "RadixSorter.h"

#include "OpenCLUtils.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>

RadixSorter::RadixSorter()
    : m_device(SelectGpuDevice()),
      m_context(m_device),
      m_queue(m_context, m_device, CL_QUEUE_PROFILING_ENABLE),
      m_program(BuildProgram(m_context, m_device, LoadTextFile("kernels/radix_sort.cl"))),
      m_histogramKernel(m_program, "BuildHistogram"),
      m_scatterKernel(m_program, "ScatterByDigit")
{
}

std::vector<std::int32_t> RadixSorter::Sort(const std::vector<std::int32_t> &values)
{
    if (values.empty())
    {
        return {};
    }

    if (values.size() > static_cast<std::size_t>(std::numeric_limits<cl_uint>::max()))
    {
        throw std::runtime_error("Array is too large for 32-bit OpenCL offsets");
    }

    const cl_uint count = static_cast<cl_uint>(values.size());
    const std::size_t blockCount = (values.size() + m_blockSize - 1) / m_blockSize;
    const std::size_t tableSize = blockCount * m_radix;
    const cl::NDRange globalSize(blockCount * m_blockSize);
    const cl::NDRange localSize(m_blockSize);

    std::vector<cl_uint> histograms(tableSize);
    std::vector<cl_uint> offsets(tableSize);
    std::vector<std::int32_t> result(values.size());

    cl::Buffer inputBuffer(m_context,
                           CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                           values.size() * sizeof(std::int32_t),
                           const_cast<std::int32_t *>(values.data()));
    cl::Buffer outputBuffer(m_context,
                            CL_MEM_READ_WRITE,
                            values.size() * sizeof(std::int32_t));
    cl::Buffer histogramBuffer(m_context,
                               CL_MEM_READ_WRITE,
                               tableSize * sizeof(cl_uint));
    cl::Buffer offsetBuffer(m_context,
                            CL_MEM_READ_ONLY,
                            tableSize * sizeof(cl_uint));

    for (cl_uint shift = 0; shift < m_passes * 8; shift += 8)
    {
        m_histogramKernel.setArg(0, inputBuffer);
        m_histogramKernel.setArg(1, histogramBuffer);
        m_histogramKernel.setArg(2, count);
        m_histogramKernel.setArg(3, shift);
        m_queue.enqueueNDRangeKernel(m_histogramKernel, cl::NullRange, globalSize, localSize);
        m_queue.enqueueReadBuffer(histogramBuffer,
                                  CL_TRUE,
                                  0,
                                  histograms.size() * sizeof(cl_uint),
                                  histograms.data());

        BuildOffsets(histograms, blockCount, offsets);
        m_queue.enqueueWriteBuffer(offsetBuffer,
                                   CL_TRUE,
                                   0,
                                   offsets.size() * sizeof(cl_uint),
                                   offsets.data());

        m_scatterKernel.setArg(0, inputBuffer);
        m_scatterKernel.setArg(1, outputBuffer);
        m_scatterKernel.setArg(2, offsetBuffer);
        m_scatterKernel.setArg(3, count);
        m_scatterKernel.setArg(4, shift);
        m_queue.enqueueNDRangeKernel(m_scatterKernel, cl::NullRange, globalSize, localSize);

        std::swap(inputBuffer, outputBuffer);
    }

    m_queue.enqueueReadBuffer(inputBuffer,
                              CL_TRUE,
                              0,
                              result.size() * sizeof(std::int32_t),
                              result.data());
    return result;
}

std::string RadixSorter::DeviceName() const
{
    return m_device.getInfo<CL_DEVICE_NAME>();
}

void RadixSorter::BuildOffsets(const std::vector<cl_uint> &histograms,
                               std::size_t blockCount,
                               std::vector<cl_uint> &offsets) const
{
    cl_uint total = 0;

    for (std::size_t digit = 0; digit < m_radix; ++digit)
    {
        cl_uint digitStart = total;

        for (std::size_t block = 0; block < blockCount; ++block)
        {
            const std::size_t index = block * m_radix + digit;
            offsets[index] = digitStart;
            digitStart += histograms[index];
        }

        total = digitStart;
    }
}
