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
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    PianoModel model;
    PianoView view;
    AudioEngine audioEngine;
    int activeMouseKey = -1;

    void handleEvent(const SDL_Event &event, bool &running);
    void applyTransition(int keyIndex, PianoModel::Transition transition);
    void cleanup();
};
