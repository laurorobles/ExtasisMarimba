#pragma once
#include <cmath>
#include <algorithm>

namespace ExtasisDSP
{

class PercussiveEnvelope
{
public:
    enum State
    {
        Idle = 0,
        Attack,
        Hold,
        Decay,
        Sustain,
        Release
    };

    PercussiveEnvelope() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        state = Idle;
        currentLevel = 0.0f;
        targetLevel = 0.0f;
        rate = 0.0f;
    }

    void setParameters(float attackMs, float decayMs, float sustainLevel, float releaseMs, bool snapCurve = true)
    {
        attTime = std::max(0.1f, attackMs);
        decTime = std::max(1.0f, decayMs);
        susLevel = std::max(0.0f, std::min(1.0f, sustainLevel));
        relTime = std::max(1.0f, releaseMs);
        isSnap = snapCurve;
    }

    void trigger(float velocity = 1.0f)
    {
        vel = std::max(0.01f, std::min(1.0f, velocity));
        state = Attack;
        targetLevel = 1.0f;

        float attackSamples = std::max(1.0f, (attTime * 0.001f) * static_cast<float>(currentSampleRate));
        rate = 1.0f / attackSamples;
    }

    void noteOff()
    {
        if (state != Idle)
        {
            state = Release;
            targetLevel = 0.0f;
            float releaseSamples = std::max(1.0f, (relTime * 0.001f) * static_cast<float>(currentSampleRate));
            // Exponential release coefficient
            relCoeff = std::exp(-4.6f / releaseSamples);
        }
    }

    inline float getNextSample()
    {
        switch (state)
        {
            case Attack:
            {
                currentLevel += rate;
                if (currentLevel >= 1.0f)
                {
                    currentLevel = 1.0f;
                    state = Decay;
                    float decaySamples = std::max(1.0f, (decTime * 0.001f) * static_cast<float>(currentSampleRate));
                    decCoeff = std::exp(-4.6f / decaySamples);
                }
                break;
            }
            case Decay:
            {
                currentLevel = susLevel + (currentLevel - susLevel) * decCoeff;
                if (std::abs(currentLevel - susLevel) < 0.0005f)
                {
                    currentLevel = susLevel;
                    state = (susLevel > 0.001f) ? Sustain : Idle;
                }
                break;
            }
            case Sustain:
            {
                currentLevel = susLevel;
                break;
            }
            case Release:
            {
                currentLevel *= relCoeff;
                if (currentLevel < 0.0001f)
                {
                    currentLevel = 0.0f;
                    state = Idle;
                }
                break;
            }
            case Idle:
            default:
                currentLevel = 0.0f;
                break;
        }

        // Apply snappy curve if enabled (pow 1.5 - 2.0 for percussive impact)
        float out = isSnap ? (currentLevel * currentLevel) : currentLevel;
        return out * vel;
    }

    bool isActive() const { return state != Idle; }
    State getState() const { return state; }

private:
    double currentSampleRate = 44100.0;
    State state = Idle;
    float currentLevel = 0.0f;
    float targetLevel = 0.0f;
    float rate = 0.0f;
    float decCoeff = 0.999f;
    float relCoeff = 0.999f;

    float attTime = 0.5f;
    float decTime = 400.0f;
    float susLevel = 0.0f;
    float relTime = 200.0f;
    bool isSnap = true;
    float vel = 1.0f;
};

} // namespace ExtasisDSP
