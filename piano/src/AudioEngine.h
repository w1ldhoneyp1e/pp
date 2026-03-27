#pragma once

#include <SDL2/SDL.h>

#include <mutex>
#include <unordered_map>

class AudioEngine
{
  public:
    AudioEngine() = default;
    ~AudioEngine();

    void init();
    void shutdown();
    void noteOn(int keyIndex, double frequency);
    void noteOff(int keyIndex);

  private:
    struct Voice
    {
        double frequency;
        double phase;
        double amplitude;
        bool pressed;
        int releaseSamplesLeft;
    };

    static constexpr int SAMPLE_RATE = 48000;
    static constexpr int RELEASE_MS = 500;
    static constexpr int ATTACK_MS = 10;
    static constexpr float MASTER_GAIN = 0.22f;

    SDL_AudioDeviceID m_audioDevice = 0;
    std::mutex m_voicesMutex;
    std::unordered_map<int, Voice> m_voices;

    static void audioCallback(void *userdata, Uint8 *stream, int len);
    void renderAudio(float *out, int frames);
};
