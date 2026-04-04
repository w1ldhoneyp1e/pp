#include "AudioEngine.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <string>

namespace
{

constexpr double DOUBLE_PI = 2 * M_PI;

}

AudioEngine::~AudioEngine() { shutdown(); }

void AudioEngine::init()
{
    SDL_AudioSpec want{};
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = 1024;
    want.callback = &AudioEngine::audioCallback;
    want.userdata = this;

    SDL_AudioSpec have{};
    m_audioDevice = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (m_audioDevice == 0)
    {
        throw std::runtime_error(std::string("SDL_OpenAudioDevice failed: ") +
                                 SDL_GetError());
    }

    SDL_PauseAudioDevice(m_audioDevice, 0);
}

void AudioEngine::shutdown()
{
    if (m_audioDevice == 0)
    {
        return;
    }

    SDL_CloseAudioDevice(m_audioDevice);
    m_audioDevice = 0;

    std::lock_guard<std::mutex> lock(m_voicesMutex);
    m_voices.clear();
}

void AudioEngine::noteOn(int keyIndex, double frequency)
{
    std::lock_guard<std::mutex> lock(m_voicesMutex);
    auto it = m_voices.find(keyIndex);
    if (it != m_voices.end())
    {
        Voice &v = it->second;
        v.frequency = frequency;
        v.pressed = true;
        v.releaseSamplesLeft = 0;

        return;
    }

    m_voices[keyIndex] = Voice{.frequency = frequency,
                               .phase = 0.0,
                               .amplitude = 0.0,
                               .pressed = true,
                               .releaseSamplesLeft = 0};
}

void AudioEngine::noteOff(int keyIndex)
{
    std::lock_guard<std::mutex> lock(m_voicesMutex);
    auto it = m_voices.find(keyIndex);
    if (it == m_voices.end())
    {
        return;
    }

    it->second.pressed = false;
    it->second.releaseSamplesLeft = (SAMPLE_RATE * RELEASE_MS) / 1000;
}

void AudioEngine::audioCallback(void *userdata, Uint8 *stream, int len)
{
    auto *engine = static_cast<AudioEngine *>(userdata);
    auto *out = reinterpret_cast<float *>(stream);
    const int samples = len / static_cast<int>(sizeof(float));
    const int frames = samples / 2;
    engine->renderAudio(out, frames);
}

void AudioEngine::renderAudio(float *out, int frames)
{
    std::fill(out, out + frames * 2, 0.0f);

    const int attackSamples = std::max(1, (SAMPLE_RATE * ATTACK_MS) / 1000);

    std::lock_guard<std::mutex> lock(m_voicesMutex);
    for (int frame = 0; frame < frames; ++frame)
    {
        double sample = 0.0;

        for (auto it = m_voices.begin(); it != m_voices.end();)
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
                it = m_voices.erase(it);
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
