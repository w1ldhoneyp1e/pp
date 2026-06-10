#pragma once

#include "Image.h"
#include "MedianFilter.h"
#include "Renderer.h"

#include <SDL.h>

#include <memory>
#include <string>

class App
{
public:
    explicit App(Image source);
    ~App();

    void Run();

private:
    static constexpr int MinRadius = 1;
    static constexpr int MaxRadius = 7;

    void InitSdl();
    void HandleEvent(const SDL_Event &event);
    void Render();
    void ChangeRadius(int delta);
    void Recompute();
    void UpdateTitle();

    Image m_source;
    Image m_filtered;
    MedianFilter m_filter;
    std::unique_ptr<Renderer> m_renderer;
    SDL_Window *m_window = nullptr;
    SDL_GLContext m_glContext = nullptr;
    bool m_running = true;
    int m_radius = 2;
};
