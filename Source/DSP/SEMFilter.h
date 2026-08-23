#pragma once
#include <cmath>
#include <algorithm>

namespace ExtasisDSP
{

class SEMFilter
{
public:
    enum FilterMode
    {
        LowPass = 0,
        BandPass = 1,
        HighPass = 2
    };

    SEMFilter() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        s1 = 0.0f;
        s2 = 0.0f;
    }

    void setParameters(float cutoffHz, float resonance, FilterMode mode = LowPass)
    {
        currentCutoff = std::max(20.0f, std::min(cutoffHz, static_cast<float>(currentSampleRate * 0.45f)));
        currentResonance = std::max(0.0f, std::min(0.98f, resonance));
        currentMode = mode;

        // Trapezoidal SVF integrator coefficients (Andrew Simper / Cytomic style)
        float g = std::tan(3.14159265f * currentCutoff / static_cast<float>(currentSampleRate));
        // damping R = 1.0 - resonance * 0.95
        float r = 2.0f * (1.0f - currentResonance * 0.92f);

        // Precalculated SVF terms
        g_val = g;
        r_val = r;
        h_val = 1.0f / (1.0f + r * g + g * g);
    }

    inline float process(float input)
    {
        // Non-linear soft saturation on the input for analog warmth
        float satInput = std::tanh(input * 0.8f);

        float hp = (satInput - r_val * s1 - g_val * s1 - s2) * h_val;
        float bp = g_val * hp + s1;
        s1 = g_val * hp + bp;
        float lp = g_val * bp + s2;
        s2 = g_val * bp + lp;

        switch (currentMode)
        {
            case LowPass:  return lp;
            case BandPass: return bp;
            case HighPass: return hp;
            default:       return lp;
        }
    }

private:
    double currentSampleRate = 44100.0;
    float s1 = 0.0f;
    float s2 = 0.0f;
    float currentCutoff = 8000.0f;
    float currentResonance = 0.1f;
    FilterMode currentMode = LowPass;

    float g_val = 0.1f;
    float r_val = 1.0f;
    float h_val = 1.0f;
};

} // namespace ExtasisDSP
