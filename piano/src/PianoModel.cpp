#include "PianoModel.h"

#include <cmath>
#include <stdexcept>

const std::array<PianoModel::KeyDef, PianoModel::KEY_COUNT> PianoModel::KEYS = {
    {
        {SDL_SCANCODE_Z, "Z", -9, false, -1},
        {SDL_SCANCODE_S, "S", -8, true, 0},
        {SDL_SCANCODE_X, "X", -7, false, -1},
        {SDL_SCANCODE_D, "D", -6, true, 1},
        {SDL_SCANCODE_C, "C", -5, false, -1},
        {SDL_SCANCODE_V, "V", -4, false, -1},
        {SDL_SCANCODE_G, "G", -3, true, 3},
        {SDL_SCANCODE_B, "B", -2, false, -1},
        {SDL_SCANCODE_H, "H", -1, true, 4},
        {SDL_SCANCODE_N, "N", 0, false, -1},
        {SDL_SCANCODE_J, "J", 1, true, 5},
        {SDL_SCANCODE_M, "M", 2, false, -1},
        {SDL_SCANCODE_Q, "Q", 3, false, -1},
        {SDL_SCANCODE_2, "2", 4, true, 7},
        {SDL_SCANCODE_W, "W", 5, false, -1},
        {SDL_SCANCODE_3, "3", 6, true, 8},
        {SDL_SCANCODE_E, "E", 7, false, -1},
        {SDL_SCANCODE_R, "R", 8, false, -1},
        {SDL_SCANCODE_5, "5", 9, true, 10},
        {SDL_SCANCODE_T, "T", 10, false, -1},
        {SDL_SCANCODE_6, "6", 11, true, 11},
        {SDL_SCANCODE_Y, "Y", 12, false, -1},
        {SDL_SCANCODE_7, "7", 13, true, 12},
        {SDL_SCANCODE_U, "U", 14, false, -1},
    }};

PianoModel::PianoModel()
{
    m_keyToIndex.fill(-1);
    for (int i = 0; i < KEY_COUNT; ++i)
    {
        m_keyToIndex[KEYS[i].scancode] = i;
    }
}

int PianoModel::keyIndexByScancode(SDL_Scancode scancode) const
{
    return m_keyToIndex[scancode];
}

PianoModel::Transition PianoModel::setKeyboardPressed(int keyIndex,
                                                      bool pressed)
{
    m_keyboardPressed[keyIndex] = pressed;

    return recomputeState(keyIndex);
}

PianoModel::Transition PianoModel::addMousePressed(int keyIndex)
{
    ++m_mousePressedCount[keyIndex];

    return recomputeState(keyIndex);
}

PianoModel::Transition PianoModel::removeMousePressed(int keyIndex)
{
    if (m_mousePressedCount[keyIndex] > 0)
    {
        --m_mousePressedCount[keyIndex];
    }

    return recomputeState(keyIndex);
}

bool PianoModel::isPressed(int keyIndex) const
{
    return m_logicalPressed[keyIndex];
}

double PianoModel::frequencyForKey(int keyIndex) const
{
    if (keyIndex < 0 || keyIndex >= KEY_COUNT)
    {
        throw std::out_of_range("Invalid key index");
    }

    return semitoneToFrequency(KEYS[keyIndex].semitoneOffset);
}

PianoModel::Transition PianoModel::recomputeState(int keyIndex)
{
    const bool nowPressed =
        m_keyboardPressed[keyIndex] || m_mousePressedCount[keyIndex] > 0;
    if (nowPressed == m_logicalPressed[keyIndex])
    {
        return Transition::None;
    }

    m_logicalPressed[keyIndex] = nowPressed;
    if (nowPressed)
    {
        return Transition::Pressed;
    }

    return Transition::Released;
}

double PianoModel::semitoneToFrequency(int semitoneOffset)
{
    return 440.0 * std::pow(2.0, static_cast<double>(semitoneOffset) / 12.0);
}
