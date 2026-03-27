#include "PianoController.h"

#include <stdexcept>
#include <string>

void PianoController::run()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        throw std::runtime_error(std::string("SDL_Init failed: ") +
                                 SDL_GetError());
    }

    m_window = SDL_CreateWindow("Piano", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, PianoView::WINDOW_WIDTH,
                                PianoView::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!m_window)
    {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") +
                                 SDL_GetError());
    }

    m_renderer = SDL_CreateRenderer(
        m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!m_renderer)
    {
        cleanup();
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") +
                                 SDL_GetError());
    }

    try
    {
        m_view.init();
        m_audioEngine.init();
    }
    catch (...)
    {
        cleanup();
        throw;
    }

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            handleEvent(event, running);
        }

        m_view.render(m_renderer, m_model);
    }

    cleanup();
}

void PianoController::handleEvent(const SDL_Event &event, bool &running)
{
    if (event.type == SDL_QUIT)
    {
        running = false;

        return;
    }

    if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
    {
        if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE &&
            event.type == SDL_KEYDOWN)
        {
            running = false;

            return;
        }

        if (event.key.repeat == 0)
        {
            const int keyIndex =
                m_model.keyIndexByScancode(event.key.keysym.scancode);
            if (keyIndex >= 0)
            {
                const bool pressed = event.type == SDL_KEYDOWN;
                const auto transition =
                    m_model.setKeyboardPressed(keyIndex, pressed);
                applyTransition(keyIndex, transition);
            }
        }

        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        if (m_activeMouseKey >= 0)
        {
            const auto oldTransition =
                m_model.removeMousePressed(m_activeMouseKey);
            applyTransition(m_activeMouseKey, oldTransition);
            m_activeMouseKey = -1;
        }

        const int keyIndex =
            m_view.pickKeyByPoint(event.button.x, event.button.y);
        if (keyIndex < 0)
        {
            return;
        }

        const auto transition = m_model.addMousePressed(keyIndex);
        applyTransition(keyIndex, transition);
        m_activeMouseKey = keyIndex;

        return;
    }

    if (event.type == SDL_MOUSEBUTTONUP &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        if (m_activeMouseKey < 0)
        {
            return;
        }

        const auto transition = m_model.removeMousePressed(m_activeMouseKey);
        applyTransition(m_activeMouseKey, transition);
        m_activeMouseKey = -1;
    }
}

void PianoController::applyTransition(int keyIndex,
                                      PianoModel::Transition transition)
{
    if (transition == PianoModel::Transition::Pressed)
    {
        m_audioEngine.noteOn(keyIndex, m_model.frequencyForKey(keyIndex));

        return;
    }

    if (transition == PianoModel::Transition::Released)
    {
        m_audioEngine.noteOff(keyIndex);
    }
}

void PianoController::cleanup()
{
    m_audioEngine.shutdown();

    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}
