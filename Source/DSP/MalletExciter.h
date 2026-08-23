#pragma once
#include <cmath>
#include <cstdlib>
#include <algorithm>

namespace ExtasisDSP
{

class MalletExciter
{
public:
    MalletExciter() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        samplePhase = 0;
        totalSamples = 0;
        noisePhase = 0;
        totalNoiseSamples = 0;
        isActive = false;
        bpS1 = 0.0f;
        bpS2 = 0.0f;
    }

    void trigger(float velocity, float hardness, float clickAmt, float noiseAmt)
    {
        vel = std::max(0.01f, std::min(1.0f, velocity));
        malletHardness = std::max(0.0f, std::min(1.0f, hardness));
        clickAmount = std::max(0.0f, std::min(1.0f, clickAmt));
        noiseAmount = std::max(0.0f, std::min(1.0f, noiseAmt));

        // 1. Strike Impulse duration: 0.8ms (hard) to 4.2ms (soft rubber)
        float durationMs = 4.2f - malletHardness * 3.4f;
        totalSamples = static_cast<int>((durationMs * 0.001f) * currentSampleRate);
        totalSamples = std::max(4, totalSamples);

        // 2. Rubber Mallet Noise burst: 8ms to 24ms
        float noiseDurationMs = 24.0f - malletHardness * 14.0f;
        totalNoiseSamples = static_cast<int>((noiseDurationMs * 0.001f) * currentSampleRate);
        totalNoiseSamples = std::max(10, totalNoiseSamples);

        samplePhase = 0;
        noisePhase = 0;
        isActive = true;

        // Bandpass filter centered at 3.5 kHz for natural rubber/wood strike texture
        float bpFreq = 2500.0f + malletHardness * 3000.0f;
        bpG = std::tan(3.14159265f * bpFreq / static_cast<float>(currentSampleRate));
        bpR = 2.0f * (1.0f - 0.45f); // Q approx 1.8
        bpH = 1.0f / (1.0f + bpR * bpG + bpG * bpG);
    }

    float getNextSample()
    {
        if (!isActive)
            return 0.0f;

        float out = 0.0f;

        // 1. Tonal/Physical Impulse Pulse
        if (samplePhase < totalSamples)
        {
            float progress = static_cast<float>(samplePhase) / static_cast<float>(totalSamples);
            float impulse = std::sin(progress * 3.14159265f);
            float shapedImpulse = std::pow(impulse, 1.0f + (1.0f - malletHardness) * 2.0f);
            out += shapedImpulse * 1.4f;
            samplePhase++;
        }

        // 2. Bandpassed Rubber Mallet Noise Texture
        if (noisePhase < totalNoiseSamples && noiseAmount > 0.001f)
        {
            float nProgress = static_cast<float>(noisePhase) / static_cast<float>(totalNoiseSamples);
            float noiseEnv = std::exp(-nProgress * 4.5f);
            float rawNoise = ((float)(rand() % 4000 - 2000) / 2000.0f);

            // SVF Bandpass
            float hp = (rawNoise - bpR * bpS1 - bpG * bpS1 - bpS2) * bpH;
            float bp = bpG * hp + bpS1;
            bpS1 = bpG * hp + bp;
            float lp = bpG * bp + bpS2;
            bpS2 = bpG * bp + lp;

            out += bp * noiseEnv * noiseAmount * 1.8f;
            noisePhase++;
        }

        // 3. Transient Click Snap
        if (clickAmount > 0.001f && samplePhase < totalSamples)
        {
            float cProgress = static_cast<float>(samplePhase) / static_cast<float>(totalSamples);
            float clickEnv = std::exp(-cProgress * 8.0f);
            float clickSig = ((float)(rand() % 2000 - 1000) / 1000.0f) * clickEnv * clickAmount;
            out += clickSig * 0.9f;
        }

        if (samplePhase >= totalSamples && noisePhase >= totalNoiseSamples)
        {
            isActive = false;
        }

        return out * vel;
    }

    bool isPlaying() const { return isActive; }

private:
    double currentSampleRate = 44100.0;
    int samplePhase = 0;
    int totalSamples = 0;
    int noisePhase = 0;
    int totalNoiseSamples = 0;
    bool isActive = false;

    float vel = 1.0f;
    float malletHardness = 0.5f;
    float clickAmount = 0.3f;
    float noiseAmount = 0.4f;

    // Bandpass filter states for noise
    float bpS1 = 0.0f;
    float bpS2 = 0.0f;
    float bpG = 0.1f;
    float bpR = 1.0f;
    float bpH = 1.0f;
};

} // namespace ExtasisDSP
