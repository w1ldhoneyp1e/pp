#pragma once

#include <SDL2/SDL.h>

#include <array>
#include <mutex>
#include <unordered_map>
#include <vector>

class Piano
{
  public:
    void run();

  private:
    struct KeyDef
    {
        SDL_Scancode scancode;
        const char *label;
        int semitoneOffset;
        bool isBlack;
        int afterWhiteIndex;
    };

    struct Voice
    {
        double frequency;
        double phase;
        double amplitude;
        bool pressed;
        int releaseSamplesLeft;
    };

    struct VisualKey
    {
        SDL_Rect rect;
        int keyIndex;
        bool isBlack;
    };

    static constexpr int WINDOW_WIDTH = 1120;
    static constexpr int WINDOW_HEIGHT = 360;
    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int RELEASE_MS = 500;
    static constexpr int ATTACK_MS = 10;
    static constexpr float MASTER_GAIN = 0.22f;

    static const std::array<KeyDef, 24> KEYS;

    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    SDL_AudioDeviceID audioDevice = 0;
    std::array<int, SDL_NUM_SCANCODES> keyToIndex{};
    std::array<bool, 24> keyboardPressed{};
    std::array<int, 24> mousePressedCount{};
    std::array<bool, 24> logicalPressed{};
    int activeMouseKey = -1;

    std::vector<VisualKey> whiteVisualKeys;
    std::vector<VisualKey> blackVisualKeys;

    std::mutex voicesMutex;
    std::unordered_map<int, Voice> voices;

    void initMappings();
    void initVisualKeys();
    void handleEvent(const SDL_Event &event, bool &running);
    void handleKeyboard(SDL_Scancode scancode, bool pressed);
    void handleMouseButton(int x, int y, bool pressed);
    void updateLogicalState(int keyIndex);
    void noteOn(int keyIndex);
    void noteOff(int keyIndex);
    int pickKeyByPoint(int x, int y) const;
    void render();

    static double semitoneToFrequency(int semitoneOffset);
    static void audioCallback(void *userdata, Uint8 *stream, int len);
    void renderAudio(float *out, int frames);
};
