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

    static constexpr int kWindowWidth = 1120;
    static constexpr int kWindowHeight = 360;
    static constexpr int kSampleRate = 48000;
    static constexpr int kReleaseMs = 500;
    static constexpr int kAttackMs = 10;
    static constexpr float kMasterGain = 0.22f;

    static const std::array<KeyDef, 24> KEYS;

    SDL_Window *window_ = nullptr;
    SDL_Renderer *renderer_ = nullptr;
    SDL_AudioDeviceID audioDevice_ = 0;
    std::array<int, SDL_NUM_SCANCODES> keyToIndex_{};
    std::array<bool, 24> keyboardPressed_{};
    std::array<int, 24> mousePressedCount_{};
    std::array<bool, 24> logicalPressed_{};
    int activeMouseKey_ = -1;

    std::vector<VisualKey> whiteVisualKeys_;
    std::vector<VisualKey> blackVisualKeys_;

    std::mutex voicesMutex_;
    std::unordered_map<int, Voice> voices_;

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
