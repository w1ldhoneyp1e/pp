#pragma once

#include "PianoModel.h"

#include <SDL2/SDL.h>

#include <vector>

class PianoView
{
  public:
    struct VisualKey
    {
        SDL_Rect rect;
        int keyIndex;
        bool isBlack;
    };

    static constexpr int WINDOW_WIDTH = 1120;
    static constexpr int WINDOW_HEIGHT = 360;

    void init();
    void render(SDL_Renderer *renderer, const PianoModel &model) const;
    int pickKeyByPoint(int x, int y) const;

  private:
    std::vector<VisualKey> whiteVisualKeys;
    std::vector<VisualKey> blackVisualKeys;
};
