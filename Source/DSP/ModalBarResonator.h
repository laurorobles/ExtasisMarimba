#pragma once
#include <cmath>
#include <array>
#include <algorithm>

namespace ExtasisDSP
{

struct ModalBand
{
    float y1 = 0.0f;
    float y2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
    float gain = 1.0f;

    void reset()
    {
        y1 = 0.0f;
        y2 = 0.0f;
    }

    void setCoefficients(float freq, float decayTimeSec, float amplitude, double sampleRate)
    {
        if (freq >= sampleRate * 0.48f)
        {
            freq = static_cast<float>(sampleRate * 0.48f);
        }
        if (freq < 20.0f) freq = 20.0f;

        float omega = 2.0f * 3.14159265f * freq / static_cast<float>(sampleRate);
        
        // Damping factor r calculated from decay time T60
        float decaySamples = std::max(10.0f, decayTimeSec * static_cast<float>(sampleRate));
        float r = std::exp(-6.907755f / decaySamples);
        r = std::min(0.99995f, std::max(0.0f, r));

        a1 = 2.0f * r * std::cos(omega);
        a2 = -r * r;
        
        // Impulse response gain normalization
        // For an impulse excitation of amplitude 1.0, this yields clear, rich acoustic body output
        gain = amplitude * 1.8f;
    }

    inline float process(float input)
    {
        float y0 = input * gain + a1 * y1 + a2 * y2;
        y2 = y1;
        y1 = y0;
        return y0;
    }

    bool hasEnergy() const
    {
        return (std::abs(y1) > 0.00002f || std::abs(y2) > 0.00002f);
    }
};

class ModalBarResonator
{
public:
    static constexpr int NUM_MODES = 5;

    ModalBarResonator() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        for (auto& mode : modes)
        {
            mode.reset();
        }
        pipeResonator.reset();
    }

    void update(float fundamentalFreq, float decayParam, float materialParam, float overtoneMix, float pipeAmount)
    {
        // Decay time: 0.15s to 4.5s
        float baseDecay = 0.15f + decayParam * decayParam * 4.2f;

        // Material interpolation:
        // 0.0 = Deep Wooden Marimba (ratios 1.0, 4.0, 9.2, 16.0)
        // 0.5 = Balafon / Kalimba (ratios 1.0, 3.85, 8.7, 14.5)
        // 1.0 = Glass / Vibraphone (ratios 1.0, 2.76, 5.4, 8.93)
        float mat = std::max(0.0f, std::min(1.0f, materialParam));
        
        std::array<float, NUM_MODES> modeRatios;
        modeRatios[0] = 1.0f;
        modeRatios[1] = 4.0f * (1.0f - mat) + 2.76f * mat;
        modeRatios[2] = 9.2f * (1.0f - mat) + 5.40f * mat;
        modeRatios[3] = 16.0f * (1.0f - mat) + 8.93f * mat;
        modeRatios[4] = 24.5f * (1.0f - mat) + 13.34f * mat;

        // Relative modal amplitudes
        std::array<float, NUM_MODES> modeGains;
        modeGains[0] = 1.0f;
        modeGains[1] = 0.55f * overtoneMix;
        modeGains[2] = 0.28f * overtoneMix * (1.0f + mat * 0.5f);
        modeGains[3] = 0.12f * overtoneMix * (1.0f + mat * 0.8f);
        modeGains[4] = 0.05f * overtoneMix * (1.0f + mat * 1.0f);

        // Frequency-dependent damping: higher modes decay much faster in wood
        for (int i = 0; i < NUM_MODES; ++i)
        {
            float modeFreq = fundamentalFreq * modeRatios[i];
            float modeDamping = 1.0f + static_cast<float>(i * i) * (0.7f - mat * 0.35f);
            float modeDecay = baseDecay / modeDamping;

            modes[i].setCoefficients(modeFreq, modeDecay, modeGains[i], currentSampleRate);
        }

        // Resonator Pipe Body (Acoustic cavity under the bar tuned to fundamental f0)
        float pipeDecay = baseDecay * 1.25f;
        pipeResonator.setCoefficients(fundamentalFreq, pipeDecay, pipeAmount * 0.75f, currentSampleRate);
    }

    float process(float exciterSample)
    {
        float sum = 0.0f;
        for (auto& mode : modes)
        {
            sum += mode.process(exciterSample);
        }

        // Add acoustic body pipe resonance
        sum += pipeResonator.process(exciterSample + sum * 0.15f);

        return sum;
    }

    bool hasEnergy() const
    {
        for (const auto& mode : modes)
        {
            if (mode.hasEnergy())
                return true;
        }
        return pipeResonator.hasEnergy();
    }

private:
    double currentSampleRate = 44100.0;
    std::array<ModalBand, NUM_MODES> modes;
    ModalBand pipeResonator;
};

} // namespace ExtasisDSP
