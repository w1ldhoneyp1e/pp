#pragma once

#include "OpenCLHeaders.h"
#include "Types.h"

#include <cstddef>
#include <random>
#include <string>
#include <vector>

class GravitySimulation {
public:
    GravitySimulation(cl::Context context,
                      cl::CommandQueue queue,
                      const std::string &kernelPath,
                      std::size_t initialCount);

    void Step(float dt, float gravity, float softening);
    void AddParticles(std::size_t count);
    void RemoveParticles(std::size_t count);
    void Reset(std::size_t count);

    const std::vector<Body> &Bodies() const;
    std::size_t Count() const;

private:
    void AppendGalaxy(std::size_t count);
    void Upload();
    void ReadPositions();
    void ReadVelocities();

    cl::Context m_context;
    cl::CommandQueue m_queue;
    cl::Program m_program;
    cl::Kernel m_kernel;
    cl::Buffer m_positionsBuffer;
    cl::Buffer m_velocitiesBuffer;

    std::vector<Body> m_bodyData;
    std::vector<Velocity> m_velocityData;
    std::mt19937 m_rng;
};
