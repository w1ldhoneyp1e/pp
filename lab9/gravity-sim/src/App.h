#pragma once

#include "Camera.h"
#include "GravitySimulation.h"
#include "OpenCLHeaders.h"
#include "Renderer.h"
#include "Types.h"

#include <SDL.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

class App {
public:
    explicit App(std::size_t initialParticleCount);
    ~App();

    App(const App &) = delete;
    App &operator=(const App &) = delete;

    void Run();

private:
    void InitSdl();
    void HandleEvent(const SDL_Event &event);
    void UpdateTitle();
    float UpdateFrameTime();

    std::size_t m_initialParticleCount;
    cl::Device m_device;
    cl::Context m_context;
    cl::CommandQueue m_queue;
    std::unique_ptr<GravitySimulation> m_simulation;

    SDL_Window *m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    bool m_running = true;
    bool m_rotatingCamera = false;

    Camera m_camera;
    Renderer m_renderer;
    SimulationParameters m_parameters;
    std::chrono::steady_clock::time_point m_previousFrame;
};
