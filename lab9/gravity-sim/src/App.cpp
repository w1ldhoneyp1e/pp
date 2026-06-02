#include "App.h"

#include "OpenCLUtils.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
constexpr float Softening = 2.5f;
}

App::App(std::size_t initialParticleCount)
    : m_initialParticleCount(initialParticleCount),
      m_device(SelectGpuDevice()),
      m_context(m_device),
      m_queue(m_context, m_device),
      m_previousFrame(std::chrono::steady_clock::now())
{
    std::cout << "Selected device: " << m_device.getInfo<CL_DEVICE_NAME>() << '\n';
    m_simulation = std::make_unique<GravitySimulation>(m_context,
                                                       m_queue,
                                                       "kernels/nbody.cl",
                                                       m_initialParticleCount);
    InitSdl();
}

App::~App()
{
    m_simulation.reset();

    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
    }
    if (m_window) {
        SDL_DestroyWindow(m_window);
    }
    SDL_Quit();
}

void App::Run()
{
    while (m_running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            HandleEvent(event);
        }

        const float frameSeconds = UpdateFrameTime();

        if (!m_parameters.paused) {
            const float dt = std::min(frameSeconds, 0.033f) * m_parameters.timeScale;
            m_simulation->Step(dt, m_parameters.gravity, Softening);
        }

        int width = 1;
        int height = 1;
        SDL_GetWindowSize(m_window, &width, &height);
        m_renderer.Render(m_simulation->Bodies(), m_camera, width, height);
        SDL_GL_SwapWindow(m_window);
        UpdateTitle();
    }
}

void App::InitSdl()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    m_window = SDL_CreateWindow("gravity",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                1280,
                                800,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        throw std::runtime_error(SDL_GetError());
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_SetSwapInterval(1);

    std::cout << "Controls:\n"
              << "  mouse drag: rotate camera, mouse wheel: zoom\n"
              << "  Space: pause\n"
              << "  +/-: time speed, [/]: gravity\n"
              << "  N: add 1024 particles, M: remove 1024, R: reset, Esc: quit\n";
}

void App::HandleEvent(const SDL_Event &event)
{
    if (event.type == SDL_QUIT) {
        m_running = false;
        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        m_rotatingCamera = true;
        return;
    }

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT) {
        m_rotatingCamera = false;
        return;
    }

    if (event.type == SDL_MOUSEMOTION && m_rotatingCamera) {
        m_camera.Rotate(static_cast<float>(event.motion.xrel), static_cast<float>(event.motion.yrel));
        return;
    }

    if (event.type == SDL_MOUSEWHEEL) {
        m_camera.Zoom(static_cast<float>(event.wheel.y));
        return;
    }

    if (event.type != SDL_KEYDOWN) {
        return;
    }

    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
        m_running = false;
        break;
    case SDLK_SPACE:
        m_parameters.paused = !m_parameters.paused;
        break;
    case SDLK_EQUALS:
    case SDLK_PLUS:
        m_parameters.timeScale = std::min(m_parameters.timeScale * 1.25f, 20.0f);
        break;
    case SDLK_MINUS:
        m_parameters.timeScale = std::max(m_parameters.timeScale / 1.25f, 0.02f);
        break;
    case SDLK_LEFTBRACKET:
        m_parameters.gravity = std::max(m_parameters.gravity / 1.2f, 0.01f);
        break;
    case SDLK_RIGHTBRACKET:
        m_parameters.gravity = std::min(m_parameters.gravity * 1.2f, 500.0f);
        break;
    case SDLK_n:
        m_simulation->AddParticles(1024);
        break;
    case SDLK_m:
        m_simulation->RemoveParticles(1024);
        break;
    case SDLK_r:
        m_simulation->Reset(m_initialParticleCount);
        break;
    default:
        break;
    }
}

void App::UpdateTitle()
{
    std::ostringstream title;
    title << "OpenCL gravity | N=" << m_simulation->Count()
          << " | dt x" << m_parameters.timeScale
          << " | G=" << m_parameters.gravity
          << " | " << (m_parameters.paused ? "paused" : "running")
          << " | " << m_device.getInfo<CL_DEVICE_NAME>();
    SDL_SetWindowTitle(m_window, title.str().c_str());
}

float App::UpdateFrameTime()
{
    const auto now = std::chrono::steady_clock::now();
    const float frameSeconds = std::chrono::duration<float>(now - m_previousFrame).count();
    m_previousFrame = now;
    return frameSeconds;
}
