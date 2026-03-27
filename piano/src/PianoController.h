#pragma once

#include "AudioEngine.h"
#include "PianoModel.h"
#include "PianoView.h"

#include <SDL2/SDL.h>

class PianoController
{
  public:
    void run();

  private:
    SDL_Window *m_window = nullptr;
    SDL_Renderer *m_renderer = nullptr;
    PianoModel m_model;
    PianoView m_view;
    AudioEngine m_audioEngine;
    int m_activeMouseKey = -1;

    void handleEvent(const SDL_Event &event, bool &running);
    void applyTransition(int keyIndex, PianoModel::Transition transition);
    void cleanup();
};
