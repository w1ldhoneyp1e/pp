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

    window = SDL_CreateWindow("Piano", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, PianoView::WINDOW_WIDTH,
                              PianoView::WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
    {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") +
                                 SDL_GetError());
    }

    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer)
    {
        cleanup();
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") +
                                 SDL_GetError());
    }

    try
    {
        view.init();
        audioEngine.init();
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

        view.render(renderer, model);
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
                model.keyIndexByScancode(event.key.keysym.scancode);
            if (keyIndex >= 0)
            {
                const bool pressed = event.type == SDL_KEYDOWN;
                const auto transition =
                    model.setKeyboardPressed(keyIndex, pressed);
                applyTransition(keyIndex, transition);
            }
        }

        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        if (activeMouseKey >= 0)
        {
            const auto oldTransition = model.removeMousePressed(activeMouseKey);
            applyTransition(activeMouseKey, oldTransition);
            activeMouseKey = -1;
        }

        const int keyIndex =
            view.pickKeyByPoint(event.button.x, event.button.y);
        if (keyIndex < 0)
        {
            return;
        }

        const auto transition = model.addMousePressed(keyIndex);
        applyTransition(keyIndex, transition);
        activeMouseKey = keyIndex;

        return;
    }

    if (event.type == SDL_MOUSEBUTTONUP &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        if (activeMouseKey < 0)
        {
            return;
        }

        const auto transition = model.removeMousePressed(activeMouseKey);
        applyTransition(activeMouseKey, transition);
        activeMouseKey = -1;
    }
}

void PianoController::applyTransition(int keyIndex,
                                      PianoModel::Transition transition)
{
    if (transition == PianoModel::Transition::Pressed)
    {
        audioEngine.noteOn(keyIndex, model.frequencyForKey(keyIndex));

        return;
    }

    if (transition == PianoModel::Transition::Released)
    {
        audioEngine.noteOff(keyIndex);
    }
}

void PianoController::cleanup()
{
    audioEngine.shutdown();

    if (renderer)
    {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window)
    {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}
