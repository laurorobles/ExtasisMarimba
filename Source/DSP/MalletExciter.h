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
        isActive = false;
    }

    void trigger(float velocity, float hardness, float clickAmt)
    {
        vel = std::max(0.01f, std::min(1.0f, velocity));
        malletHardness = std::max(0.0f, std::min(1.0f, hardness));
        clickAmount = std::max(0.0f, std::min(1.0f, clickAmt));

        // Excitation duration: shorter for harder mallets, longer for softer felt mallets
        // Approx 0.8ms to 4.5ms
        float durationMs = 4.5f - malletHardness * 3.7f;
        totalSamples = static_cast<int>((durationMs * 0.001f) * currentSampleRate);
        totalSamples = std::max(4, totalSamples);

        samplePhase = 0;
        isActive = true;
    }

    float getNextSample()
    {
        if (!isActive)
            return 0.0f;

        float progress = static_cast<float>(samplePhase) / static_cast<float>(totalSamples);
        
        // Raised cosine / half-sine window for smooth mallet strike impulse
        float impulse = std::sin(progress * 3.14159265f);
        
        // Add subtle high-frequency transient click for extra stick/mallet definition
        float noise = ((float)(rand() % 2000 - 1000) / 1000.0f);
        float click = noise * std::exp(-progress * 8.0f) * clickAmount;

        // Shape pulse sharpness based on hardness
        float shapedImpulse = std::pow(impulse, 1.0f + (1.0f - malletHardness) * 2.0f);
        float out = (shapedImpulse + click * 0.8f) * vel;

        samplePhase++;
        if (samplePhase >= totalSamples)
        {
            isActive = false;
        }

        return out;
    }

    bool isPlaying() const { return isActive; }

private:
    double currentSampleRate = 44100.0;
    int samplePhase = 0;
    int totalSamples = 0;
    bool isActive = false;
    float vel = 1.0f;
    float malletHardness = 0.5f;
    float clickAmount = 0.3f;
};

} // namespace ExtasisDSP
