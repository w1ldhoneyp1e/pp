#include "OpenCLUtils.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <vector>

cl::Device SelectGpuDevice()
{
    std::vector<cl::Platform> platforms;
    try {
        cl::Platform::get(&platforms);
    } catch (const cl::Error &error) {
        if (error.err() == CL_PLATFORM_NOT_FOUND_KHR) {
            throw std::runtime_error("No OpenCL platforms found. Install an OpenCL GPU runtime/driver.");
        }
        throw;
    }

    if (platforms.empty()) {
        throw std::runtime_error("No OpenCL platforms found. Install an OpenCL GPU runtime/driver.");
    }

    for (const auto &platform : platforms) {
        std::vector<cl::Device> devices;
        platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
        if (!devices.empty()) {
            return devices.front();
        }
    }

    throw std::runtime_error("No OpenCL GPU devices found");
}

std::string LoadTextFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::ostringstream out;
    out << file.rdbuf();
    return out.str();
}

cl::Program BuildProgram(const cl::Context &context,
                         const cl::Device &device,
                         const std::string &source)
{
    cl::Program program(context, source);

    try {
        program.build({device}, "-cl-fast-relaxed-math");
    } catch (const cl::Error &) {
        const std::string log = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
        throw std::runtime_error("OpenCL program build failed:\n" + log);
    }

    return program;
}
