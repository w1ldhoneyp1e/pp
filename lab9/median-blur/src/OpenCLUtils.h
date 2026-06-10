#pragma once

#include "OpenCLHeaders.h"

#include <string>

cl::Device SelectGpuDevice();
std::string LoadTextFile(const std::string &path);
cl::Program BuildProgram(const cl::Context &context,
                         const cl::Device &device,
                         const std::string &source);
