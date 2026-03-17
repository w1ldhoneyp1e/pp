#include "PianoView.h"

namespace
{

bool pointInRect(int x, int y, const SDL_Rect &rect)
{
    return x >= rect.x && x < rect.x + rect.w && y >= rect.y &&
           y < rect.y + rect.h;
}

} // namespace

void PianoView::init()
{
    whiteVisualKeys.clear();
    blackVisualKeys.clear();

    const int marginX = 20;
    const int top = 20;
    const int whiteHeight = WINDOW_HEIGHT - 40;
    const int whiteCount = 14;
    const int whiteWidth = (WINDOW_WIDTH - marginX * 2) / whiteCount;

    int whiteIndex = 0;
    for (int i = 0; i < PianoModel::KEY_COUNT; ++i)
    {
        if (PianoModel::KEYS[i].isBlack)
        {
            continue;
        }

        SDL_Rect rect{marginX + whiteIndex * whiteWidth, top, whiteWidth - 2,
                      whiteHeight};
        whiteVisualKeys.push_back({rect, i, false});
        ++whiteIndex;
    }

    const int blackWidth = static_cast<int>(whiteWidth * 0.62);
    const int blackHeight = static_cast<int>(whiteHeight * 0.62);
    for (int i = 0; i < PianoModel::KEY_COUNT; ++i)
    {
        if (!PianoModel::KEYS[i].isBlack)
        {
            continue;
        }

        const int after = PianoModel::KEYS[i].afterWhiteIndex;
        const int leftWhiteX = marginX + after * whiteWidth;
        const int x = leftWhiteX + whiteWidth - blackWidth / 2;
        SDL_Rect rect{x, top, blackWidth, blackHeight};
        blackVisualKeys.push_back({rect, i, true});
    }
}

void PianoView::render(SDL_Renderer *renderer, const PianoModel &model) const
{
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderClear(renderer);

    for (const auto &key : whiteVisualKeys)
    {
        const bool pressed = model.isPressed(key.keyIndex);
        if (pressed)
        {
            SDL_SetRenderDrawColor(renderer, 242, 188, 195, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }

        SDL_RenderFillRect(renderer, &key.rect);
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderDrawRect(renderer, &key.rect);
    }

    for (const auto &key : blackVisualKeys)
    {
        const bool pressed = model.isPressed(key.keyIndex);
        if (pressed)
        {
            SDL_SetRenderDrawColor(renderer, 215, 0, 122, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }

        SDL_RenderFillRect(renderer, &key.rect);
        SDL_SetRenderDrawColor(renderer, 35, 35, 35, 255);
        SDL_RenderDrawRect(renderer, &key.rect);
    }

    SDL_RenderPresent(renderer);
}

int PianoView::pickKeyByPoint(int x, int y) const
{
    for (const auto &key : blackVisualKeys)
    {
        if (pointInRect(x, y, key.rect))
        {
            return key.keyIndex;
        }
    }

    for (const auto &key : whiteVisualKeys)
    {
        if (pointInRect(x, y, key.rect))
        {
            return key.keyIndex;
        }
    }

    return -1;
}
