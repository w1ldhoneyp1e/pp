#include "App.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>

App::App(Image source)
    : m_source(std::move(source)),
      m_filter("kernels/median_blur.cl")
{
    std::cout << "Selected device: " << m_filter.DeviceName() << '\n';
    InitSdl();

    m_renderer = std::make_unique<Renderer>();
    m_renderer->SetSourceImage(m_source);

    Recompute();
}

App::~App()
{
    m_renderer.reset();

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

        Render();
        SDL_GL_SwapWindow(m_window);
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

    m_window = SDL_CreateWindow("OpenCL median blur",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                1200,
                                760,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window) {
        throw std::runtime_error(SDL_GetError());
    }

    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        throw std::runtime_error(SDL_GetError());
    }

    SDL_GL_SetSwapInterval(1);
    std::cout << "Controls: click +/- to change radius, Esc to quit\n";
}

void App::HandleEvent(const SDL_Event &event)
{
    if (event.type == SDL_QUIT) {
        m_running = false;
        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
        int windowHeight = 1;
        SDL_GetWindowSize(m_window, nullptr, &windowHeight);
        switch (m_renderer->HitTestControls(event.button.x, event.button.y, windowHeight)) {
        case Renderer::ControlAction::DecreaseRadius:
            ChangeRadius(-1);
            break;
        case Renderer::ControlAction::IncreaseRadius:
            ChangeRadius(1);
            break;
        case Renderer::ControlAction::None:
            break;
        }
        return;
    }

    if (event.type != SDL_KEYDOWN) {
        return;
    }

    switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
        m_running = false;
        break;
    default:
        break;
    }
}

void App::Render()
{
    int windowWidth = 1;
    int windowHeight = 1;
    SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);
    m_renderer->Render(windowWidth, windowHeight, m_radius, MinRadius, MaxRadius);
}

void App::ChangeRadius(int delta)
{
    const int nextRadius = std::clamp(m_radius + delta, MinRadius, MaxRadius);

    if (nextRadius != m_radius) {
        m_radius = nextRadius;
        Recompute();
    }
}

void App::Recompute()
{
    m_filtered = m_filter.Apply(m_source, m_radius);
    m_renderer->SetFilteredImage(m_filtered);
    UpdateTitle();
}

void App::UpdateTitle()
{
    std::ostringstream title;
    const int matrixSize = m_radius * 2 + 1;
    title << "OpenCL median blur | radius=" << m_radius
          << " | matrix=" << matrixSize << "x" << matrixSize
          << " | " << m_source.Width << "x" << m_source.Height
          << " | " << m_filter.DeviceName();
    SDL_SetWindowTitle(m_window, title.str().c_str());
}
