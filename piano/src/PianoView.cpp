#include "PianoView.h"

namespace
{

bool pointInRect(int x, int y, const SDL_Rect &rect)
{
    return x >= rect.x && x < rect.x + rect.w && y >= rect.y &&
           y < rect.y + rect.h;
}

const float BLACK_WIDTH_RATIO = 0.62f;
const float BLACK_HEIGHT_RATIO = 0.62f;

} // namespace

void PianoView::init()
{
    m_whiteVisualKeys.clear();
    m_blackVisualKeys.clear();

    const int marginX = 20;
    const int marginY = 20;
    const int whiteHeight = WINDOW_HEIGHT - marginY * 2;
    const int whiteCount = 14;
    const int whiteWidth = (WINDOW_WIDTH - marginX * 2) / whiteCount;

    int whiteIndex = 0;
    for (int i = 0; i < PianoModel::KEY_COUNT; ++i)
    {
        if (PianoModel::KEYS[i].isBlack)
        {
            continue;
        }

        SDL_Rect rect{marginX + whiteIndex * whiteWidth, marginY,
                      whiteWidth - 2, whiteHeight};
        m_whiteVisualKeys.push_back({rect, i, false});
        ++whiteIndex;
    }

    const int blackWidth = static_cast<int>(whiteWidth * BLACK_WIDTH_RATIO);
    const int blackHeight = static_cast<int>(whiteHeight * BLACK_HEIGHT_RATIO);
    for (int i = 0; i < PianoModel::KEY_COUNT; ++i)
    {
        if (!PianoModel::KEYS[i].isBlack)
        {
            continue;
        }

        const int after = PianoModel::KEYS[i].afterWhiteIndex;
        const int leftWhiteX = marginX + after * whiteWidth;
        const int x = leftWhiteX + whiteWidth - blackWidth / 2;
        SDL_Rect rect{x, marginY, blackWidth, blackHeight};
        m_blackVisualKeys.push_back({rect, i, true});
    }
}

void PianoView::render(SDL_Renderer *renderer, const PianoModel &model) const
{
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderClear(renderer);

    for (const auto &key : m_whiteVisualKeys)
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

    for (const auto &key : m_blackVisualKeys)
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
    for (const auto &key : m_blackVisualKeys)
    {
        if (pointInRect(x, y, key.rect))
        {
            return key.keyIndex;
        }
    }

    for (const auto &key : m_whiteVisualKeys)
    {
        if (pointInRect(x, y, key.rect))
        {
            return key.keyIndex;
        }
    }

    return -1;
}
