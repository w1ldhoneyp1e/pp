#include "Piano.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
#include <string>

namespace
{

constexpr double kTwoPi = 6.28318530717958647692;

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

    window_ = SDL_CreateWindow("Piano", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, kWindowWidth,
                               kWindowHeight, SDL_WINDOW_SHOWN);
    if (!window_)
    {
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateWindow failed: ") +
                                 SDL_GetError());
    }

    renderer_ = SDL_CreateRenderer(
        window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_)
    {
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_CreateRenderer failed: ") +
                                 SDL_GetError());
    }

    SDL_AudioSpec want{};
    want.freq = kSampleRate;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = &Piano::audioCallback;
    want.userdata = this;

    SDL_AudioSpec have{};
    audioDevice_ = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (audioDevice_ == 0)
    {
        SDL_DestroyRenderer(renderer_);
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error(std::string("SDL_OpenAudioDevice failed: ") +
                                 SDL_GetError());
    }

    initMappings();
    initVisualKeys();
    SDL_PauseAudioDevice(audioDevice_, 0);

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

    SDL_CloseAudioDevice(audioDevice_);
    SDL_DestroyRenderer(renderer_);
    SDL_DestroyWindow(window_);
    SDL_Quit();
}

void Piano::initMappings()
{
    keyToIndex_.fill(-1);
    for (int i = 0; i < static_cast<int>(KEYS.size()); ++i)
    {
        keyToIndex_[KEYS[i].scancode] = i;
    }
}

void Piano::initVisualKeys()
{
    whiteVisualKeys_.clear();
    blackVisualKeys_.clear();

    const int marginX = 20;
    const int top = 20;
    const int whiteHeight = kWindowHeight - 40;
    const int whiteCount = 14;
    const int whiteWidth = (kWindowWidth - marginX * 2) / whiteCount;

    int whiteIndex = 0;
    for (int i = 0; i < static_cast<int>(KEYS.size()); ++i)
    {
        if (KEYS[i].isBlack)
        {
            continue;
        }

        SDL_Rect rect{marginX + whiteIndex * whiteWidth, top, whiteWidth - 2,
                      whiteHeight};
        whiteVisualKeys_.push_back({rect, i, false});
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
        blackVisualKeys_.push_back({rect, i, true});
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
    const int keyIndex = keyToIndex_[scancode];
    if (keyIndex < 0)
    {
        return;
    }

    keyboardPressed_[keyIndex] = pressed;
    updateLogicalState(keyIndex);
}

void Piano::handleMouseButton(int x, int y, bool pressed)
{
    if (pressed)
    {
        if (activeMouseKey_ >= 0)
        {
            mousePressedCount_[activeMouseKey_] =
                std::max(0, mousePressedCount_[activeMouseKey_] - 1);
            updateLogicalState(activeMouseKey_);
            activeMouseKey_ = -1;
        }

        const int keyIndex = pickKeyByPoint(x, y);
        if (keyIndex < 0)
        {
            return;
        }

        ++mousePressedCount_[keyIndex];
        activeMouseKey_ = keyIndex;
        updateLogicalState(keyIndex);

        return;
    }

    if (activeMouseKey_ < 0)
    {
        return;
    }

    mousePressedCount_[activeMouseKey_] =
        std::max(0, mousePressedCount_[activeMouseKey_] - 1);
    updateLogicalState(activeMouseKey_);
    activeMouseKey_ = -1;
}

void Piano::updateLogicalState(int keyIndex)
{
    const bool nowPressed =
        keyboardPressed_[keyIndex] || mousePressedCount_[keyIndex] > 0;
    if (nowPressed == logicalPressed_[keyIndex])
    {
        return;
    }

    logicalPressed_[keyIndex] = nowPressed;
    if (nowPressed)
    {
        noteOn(keyIndex);

        return;
    }

    noteOff(keyIndex);
}

void Piano::noteOn(int keyIndex)
{
    std::lock_guard<std::mutex> lock(voicesMutex_);
    voices_[keyIndex] =
        Voice{.frequency = semitoneToFrequency(KEYS[keyIndex].semitoneOffset),
              .phase = 0.0,
              .amplitude = 0.0,
              .pressed = true,
              .releaseSamplesLeft = 0};
}

void Piano::noteOff(int keyIndex)
{
    std::lock_guard<std::mutex> lock(voicesMutex_);
    auto it = voices_.find(keyIndex);
    if (it == voices_.end())
    {
        return;
    }

    it->second.pressed = false;
    it->second.releaseSamplesLeft = (kSampleRate * kReleaseMs) / 1000;
}

int Piano::pickKeyByPoint(int x, int y) const
{
    for (const auto &key : blackVisualKeys_)
    {
        if (pointInRect(x, y, key.rect))
        {
            return key.keyIndex;
        }
    }

    for (const auto &key : whiteVisualKeys_)
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
    SDL_SetRenderDrawColor(renderer_, 230, 230, 230, 255);
    SDL_RenderClear(renderer_);

    for (const auto &key : whiteVisualKeys_)
    {
        const bool pressed = logicalPressed_[key.keyIndex];
        if (pressed)
        {
            SDL_SetRenderDrawColor(renderer_, 242, 188, 195, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
        }
        SDL_RenderFillRect(renderer_, &key.rect);
        SDL_SetRenderDrawColor(renderer_, 40, 40, 40, 255);
        SDL_RenderDrawRect(renderer_, &key.rect);
    }

    for (const auto &key : blackVisualKeys_)
    {
        const bool pressed = logicalPressed_[key.keyIndex];
        if (pressed)
        {
            SDL_SetRenderDrawColor(renderer_, 215, 0, 122, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        }
        SDL_RenderFillRect(renderer_, &key.rect);
        SDL_SetRenderDrawColor(renderer_, 35, 35, 35, 255);
        SDL_RenderDrawRect(renderer_, &key.rect);
    }

    SDL_RenderPresent(renderer_);
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

    const int attackSamples = std::max(1, (kSampleRate * kAttackMs) / 1000);

    std::lock_guard<std::mutex> lock(voicesMutex_);
    for (int frame = 0; frame < frames; ++frame)
    {
        double sample = 0.0;

        for (auto it = voices_.begin(); it != voices_.end();)
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
                kTwoPi * voice.frequency / static_cast<double>(kSampleRate);
            if (voice.phase >= kTwoPi)
            {
                voice.phase -= kTwoPi;
            }

            sample += std::sin(voice.phase) * voice.amplitude;

            if (!voice.pressed && voice.releaseSamplesLeft <= 0)
            {
                it = voices_.erase(it);
            }
            else
            {
                ++it;
            }
        }

        const float clipped =
            static_cast<float>(std::tanh(sample * 0.5) * kMasterGain);
        out[frame * 2] = clipped;
        out[frame * 2 + 1] = clipped;
    }
}
