#include "GravitySimulation.h"

#include "OpenCLUtils.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr float Pi = 3.1415926535f;

float RandomRange(std::mt19937 &rng, float minValue, float maxValue)
{
    return std::uniform_real_distribution<float>(minValue, maxValue)(rng);
}
}

GravitySimulation::GravitySimulation(cl::Context context,
                                     cl::CommandQueue queue,
                                     const std::string &kernelPath,
                                     std::size_t initialCount)
    : m_context(std::move(context)),
      m_queue(std::move(queue)),
      m_program(BuildProgram(m_context,
                             m_context.getInfo<CL_CONTEXT_DEVICES>().front(),
                             LoadTextFile(kernelPath))),
      m_kernel(m_program, "step_nbody"),
      m_rng(42)
{
    Reset(initialCount);
}

void GravitySimulation::Step(float dt, float gravity, float softening)
{
    if (m_bodyData.empty()) {
        return;
    }

    const int particleCount = static_cast<int>(m_bodyData.size());
    m_kernel.setArg(0, m_positionsBuffer);
    m_kernel.setArg(1, m_velocitiesBuffer);
    m_kernel.setArg(2, particleCount);
    m_kernel.setArg(3, dt);
    m_kernel.setArg(4, gravity);
    m_kernel.setArg(5, softening);

    constexpr std::size_t localSize = 128;
    const std::size_t globalSize = ((m_bodyData.size() + localSize - 1) / localSize) * localSize;

    m_queue.enqueueNDRangeKernel(m_kernel,
                                 cl::NullRange,
                                 cl::NDRange(globalSize),
                                 cl::NDRange(localSize));
    m_queue.finish();
    ReadPositions();
}

void GravitySimulation::AddParticles(std::size_t count)
{
    ReadPositions();
    ReadVelocities();
    AppendGalaxy(count);
    Upload();
}

void GravitySimulation::RemoveParticles(std::size_t count)
{
    if (m_bodyData.size() <= count) {
        return;
    }

    ReadPositions();
    ReadVelocities();
    m_bodyData.resize(m_bodyData.size() - count);
    m_velocityData.resize(m_velocityData.size() - count);
    Upload();
}

void GravitySimulation::Reset(std::size_t count)
{
    m_bodyData.clear();
    m_velocityData.clear();
    AppendGalaxy(count);
    Upload();
}

const std::vector<Body> &GravitySimulation::Bodies() const
{
    return m_bodyData;
}

std::size_t GravitySimulation::Count() const
{
    return m_bodyData.size();
}

void GravitySimulation::AppendGalaxy(std::size_t count)
{
    const std::size_t oldCount = m_bodyData.size();
    m_bodyData.resize(oldCount + count);
    m_velocityData.resize(oldCount + count);

    for (std::size_t i = oldCount; i < m_bodyData.size(); ++i) {
        const float radius = std::sqrt(RandomRange(m_rng, 0.0f, 1.0f)) * 95.0f + 5.0f;
        const float angle = RandomRange(m_rng, 0.0f, 2.0f * Pi);
        const float height = RandomRange(m_rng, -12.0f, 12.0f);

        m_bodyData[i] = {
            std::cos(angle) * radius,
            height,
            std::sin(angle) * radius,
            RandomRange(m_rng, 0.4f, 2.2f),
        };

        const float speed = 3.8f / std::sqrt(radius * 0.04f + 1.0f);
        m_velocityData[i] = {
            -std::sin(angle) * speed,
            RandomRange(m_rng, -0.08f, 0.08f),
            std::cos(angle) * speed,
            0.0f,
        };
    }
}

void GravitySimulation::Upload()
{
    if (m_bodyData.empty()) {
        m_positionsBuffer = cl::Buffer();
        m_velocitiesBuffer = cl::Buffer();
        return;
    }

    m_positionsBuffer = cl::Buffer(m_context,
                                   CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                   sizeof(Body) * m_bodyData.size(),
                                   m_bodyData.data());
    m_velocitiesBuffer = cl::Buffer(m_context,
                                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                    sizeof(Velocity) * m_velocityData.size(),
                                    m_velocityData.data());
}

void GravitySimulation::ReadPositions()
{
    if (m_bodyData.empty()) {
        return;
    }

    m_queue.enqueueReadBuffer(m_positionsBuffer,
                              CL_TRUE,
                              0,
                              sizeof(Body) * m_bodyData.size(),
                              m_bodyData.data());
}

void GravitySimulation::ReadVelocities()
{
    if (m_velocityData.empty()) {
        return;
    }

    m_queue.enqueueReadBuffer(m_velocitiesBuffer,
                              CL_TRUE,
                              0,
                              sizeof(Velocity) * m_velocityData.size(),
                              m_velocityData.data());
}
