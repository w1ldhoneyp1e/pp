#include "Piano.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

namespace
{

constexpr double DOUBLE_PI = 6.28318530717958647692;

bool pointInRect(int x, int y, const SDL_Rect &rect)
{
    return x >= rect.x && x < rect.x + rect.w && y >= rect.y &&
           y < rect.y + rect.h;
}

} // namespace

const std::array<Piano::KeyDef, 24> Piano::KEYS = {{
    {SDL_SCANCODE_Z, "Z", -9, false, -1}, {SDL_SCANCODE_S, "S", -8, true, 0},
    {SDL_SCANCODE_X, "X", -7, false, -1}, {SDL_SCANCODE_D, "D", -6, true, 1},
    {SDL_SCANCODE_C, "C", -5, false, -1}, {SDL_SCANCODE_V, "V", -4, false, -1},
    {SDL_SCANCODE_G, "G", -3, true, 3},   {SDL_SCANCODE_B, "B", -2, false, -1},
    {SDL_SCANCODE_H, "H", -1, true, 4},   {SDL_SCANCODE_N, "N", 0, false, -1},
    {SDL_SCANCODE_J, "J", 1, true, 5},    {SDL_SCANCODE_M, "M", 2, false, -1},
    {SDL_SCANCODE_Q, "Q", 3, false, -1},  {SDL_SCANCODE_2, "2", 4, true, 7},
    {SDL_SCANCODE_W, "W", 5, false, -1},  {SDL_SCANCODE_3, "3", 6, true, 8},
    {SDL_SCANCODE_E, "E", 7, false, -1},  {SDL_SCANCODE_R, "R", 8, false, -1},
    {SDL_SCANCODE_5, "5", 9, true, 10},   {SDL_SCANCODE_T, "T", 10, false, -1},
    {SDL_SCANCODE_6, "6", 11, true, 11},  {SDL_SCANCODE_Y, "Y", 12, false, -1},
    {SDL_SCANCODE_7, "7", 13, true, 12},  {SDL_SCANCODE_U, "U", 14, false, -1},
}};

void Piano::run()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    {
        throw std::runtime_error(std::string("SDL_Init failed: ") +
                                 SDL_GetError());
    }

    window = SDL_CreateWindow("Piano", SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                              WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") +
                                 SDL_GetError());
    }

    SDL_AudioSpec want{};
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = &Piano::audioCallback;
    want.userdata = this;

    SDL_AudioSpec have{};
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audioDevice == 0)
    {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_OpenAudioDevice failed: ") +
                                 SDL_GetError());
    }

    initMappings();
    initVisualKeys();
    SDL_PauseAudioDevice(audioDevice, 0);

    bool running = true;
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            handleEvent(event, running);
        }

        render();
    }

    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Piano::initMappings()
{
    keyToIndex.fill(-1);
    for (int i = 0; i < static_cast<int>(KEYS.size()); ++i)
    {
        keyToIndex[KEYS[i].scancode] = i;
    }
}

void Piano::initVisualKeys()
{
    whiteVisualKeys.clear();
    blackVisualKeys.clear();

    const int marginX = 20;
    const int top = 20;
    const int whiteHeight = WINDOW_HEIGHT - 40;
    const int whiteCount = 14;
    const int whiteWidth = (WINDOW_WIDTH - marginX * 2) / whiteCount;

    int whiteIndex = 0;
    for (int i = 0; i < static_cast<int>(KEYS.size()); ++i)
    {
        if (KEYS[i].isBlack)
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
    for (int i = 0; i < static_cast<int>(KEYS.size()); ++i)
    {
        if (!KEYS[i].isBlack)
        {
            continue;
        }

        const int after = KEYS[i].afterWhiteIndex;
        const int leftWhiteX = marginX + after * whiteWidth;
        const int x = leftWhiteX + whiteWidth - blackWidth / 2;
        SDL_Rect rect{x, top, blackWidth, blackHeight};
        blackVisualKeys.push_back({rect, i, true});
    }
}

void Piano::handleEvent(const SDL_Event &event, bool &running)
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
            handleKeyboard(event.key.keysym.scancode,
                           event.type == SDL_KEYDOWN);
        }

        return;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        handleMouseButton(event.button.x, event.button.y, true);

        return;
    }

    if (event.type == SDL_MOUSEBUTTONUP &&
        event.button.button == SDL_BUTTON_LEFT)
    {
        handleMouseButton(event.button.x, event.button.y, false);
    }
}

void Piano::handleKeyboard(SDL_Scancode scancode, bool pressed)
{
    const int keyIndex = keyToIndex[scancode];
    if (keyIndex < 0)
    {
        return;
    }

    keyboardPressed[keyIndex] = pressed;
    updateLogicalState(keyIndex);
}

void Piano::handleMouseButton(int x, int y, bool pressed)
{
    if (pressed)
    {
        if (activeMouseKey >= 0)
        {
            mousePressedCount[activeMouseKey] =
                std::max(0, mousePressedCount[activeMouseKey] - 1);
            updateLogicalState(activeMouseKey);
            activeMouseKey = -1;
        }

        const int keyIndex = pickKeyByPoint(x, y);
        if (keyIndex < 0)
        {
            return;
        }

        ++mousePressedCount[keyIndex];
        activeMouseKey = keyIndex;
        updateLogicalState(keyIndex);

        return;
    }

    if (activeMouseKey < 0)
    {
        return;
    }

    mousePressedCount[activeMouseKey] =
        std::max(0, mousePressedCount[activeMouseKey] - 1);
    updateLogicalState(activeMouseKey);
    activeMouseKey = -1;
}

void Piano::updateLogicalState(int keyIndex)
{
    const bool nowPressed =
        keyboardPressed[keyIndex] || mousePressedCount[keyIndex] > 0;
    if (nowPressed == logicalPressed[keyIndex])
    {
        return;
    }

    logicalPressed[keyIndex] = nowPressed;
    if (nowPressed)
    {
        noteOn(keyIndex);

        return;
    }

    noteOff(keyIndex);
}

void Piano::noteOn(int keyIndex)
{
    std::lock_guard<std::mutex> lock(voicesMutex);
    voices[keyIndex] =
        Voice{.frequency = semitoneToFrequency(KEYS[keyIndex].semitoneOffset),
              .phase = 0.0,
              .amplitude = 0.0,
              .pressed = true,
              .releaseSamplesLeft = 0};
}

void Piano::noteOff(int keyIndex)
{
    std::lock_guard<std::mutex> lock(voicesMutex);
    auto it = voices.find(keyIndex);
    if (it == voices.end())
    {
        return;
    }

    it->second.pressed = false;
    it->second.releaseSamplesLeft = (SAMPLE_RATE * RELEASE_MS) / 1000;
}

int Piano::pickKeyByPoint(int x, int y) const
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

void Piano::render()
{
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderClear(renderer);

    for (const auto &key : whiteVisualKeys)
    {
        const bool pressed = logicalPressed[key.keyIndex];
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
        const bool pressed = logicalPressed[key.keyIndex];
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

double Piano::semitoneToFrequency(int semitoneOffset)
{
    return 440.0 * std::pow(2.0, static_cast<double>(semitoneOffset) / 12.0);
}

void Piano::audioCallback(void *userdata, Uint8 *stream, int len)
{
    auto *app = static_cast<Piano *>(userdata);
    auto *out = reinterpret_cast<float *>(stream);
    const int samples = len / static_cast<int>(sizeof(float));
    const int frames = samples / 2;
    app->renderAudio(out, frames);
}

void Piano::renderAudio(float *out, int frames)
{
    std::fill(out, out + frames * 2, 0.0f);

    const int attackSamples = std::max(1, (SAMPLE_RATE * ATTACK_MS) / 1000);

    std::lock_guard<std::mutex> lock(voicesMutex);
    for (int frame = 0; frame < frames; ++frame)
    {
        double sample = 0.0;

        for (auto it = voices.begin(); it != voices.end();)
        {
            Voice &voice = it->second;

            if (voice.pressed)
            {
                const double step = (1.0 - voice.amplitude) /
                                    static_cast<double>(attackSamples);
                voice.amplitude = std::min(1.0, voice.amplitude + step);
            }
            else if (voice.releaseSamplesLeft > 0)
            {
                const double factor = std::pow(
                    0.0005,
                    1.0 / static_cast<double>(voice.releaseSamplesLeft));
                voice.amplitude *= factor;
                --voice.releaseSamplesLeft;
            }

            voice.phase +=
                DOUBLE_PI * voice.frequency / static_cast<double>(SAMPLE_RATE);
            if (voice.phase >= DOUBLE_PI)
            {
                voice.phase -= DOUBLE_PI;
            }

            sample += std::sin(voice.phase) * voice.amplitude;

            if (!voice.pressed && voice.releaseSamplesLeft <= 0)
            {
                it = voices.erase(it);
            }
            else
            {
                ++it;
            }
        }

        const float clipped =
            static_cast<float>(std::tanh(sample * 0.5) * MASTER_GAIN);
        out[frame * 2] = clipped;
        out[frame * 2 + 1] = clipped;
    }
}
