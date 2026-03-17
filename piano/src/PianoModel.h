#pragma once

#include <SDL2/SDL.h>

#include <array>

class PianoModel
{
  public:
    struct KeyDef
    {
        SDL_Scancode scancode;
        const char *label;
        int semitoneOffset;
        bool isBlack;
        int afterWhiteIndex;
    };

    enum class Transition
    {
        None,
        Pressed,
        Released
    };

    static constexpr int KEY_COUNT = 24;
    static const std::array<KeyDef, KEY_COUNT> KEYS;

    PianoModel();

    int keyIndexByScancode(SDL_Scancode scancode) const;
    Transition setKeyboardPressed(int keyIndex, bool pressed);
    Transition addMousePressed(int keyIndex);
    Transition removeMousePressed(int keyIndex);
    bool isPressed(int keyIndex) const;
    double frequencyForKey(int keyIndex) const;

  private:
    std::array<int, SDL_NUM_SCANCODES> keyToIndex_{};
    std::array<bool, KEY_COUNT> keyboardPressed_{};
    std::array<int, KEY_COUNT> mousePressedCount_{};
    std::array<bool, KEY_COUNT> logicalPressed_{};

    Transition recomputeState(int keyIndex);
    static double semitoneToFrequency(int semitoneOffset);
};
