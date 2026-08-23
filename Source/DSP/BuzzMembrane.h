#pragma once
#include <cmath>
#include <algorithm>

namespace ExtasisDSP
{

class BuzzMembrane
{
public:
    BuzzMembrane() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        reset();
    }

    void reset()
    {
        phaseCarrier = 0.0f;
        phaseMod = 0.0f;
        envLevel = 0.0f;
        isActive = false;
    }

    void trigger(float freq, float velocity, float buzzAmount, float buzzVelSens)
    {
        fundamentalFreq = freq;
        vel = std::max(0.01f, std::min(1.0f, velocity));
        amount = std::max(0.0f, std::min(1.0f, buzzAmount));
        velSens = std::max(0.0f, std::min(1.0f, buzzVelSens));

        if (amount < 0.005f)
        {
            isActive = false;
            return;
        }

        // Carrier at 2x fundamental, Modulator at 7.2x fundamental for nasal buzz timbre
        carrierFreq = fundamentalFreq * 2.0f;
        modFreq = fundamentalFreq * 7.2f;

        // Dynamic FM index driven by velocity sensitivity
        float dynamicVelocityScale = (1.0f - velSens) + velSens * (vel * vel);
        effectiveIndex = amount * (1.2f + dynamicVelocityScale * 3.5f);

        envLevel = 1.0f;
        // Decay in approx 140ms - 260ms
        float decayMs = 140.0f + (1.0f - amount) * 120.0f;
        decayCoeff = std::exp(-4.6f / ((decayMs * 0.001f) * static_cast<float>(currentSampleRate)));
        sustainFloor = amount * 0.18f * dynamicVelocityScale;

        isActive = true;
    }

    void noteOff()
    {
        sustainFloor = 0.0f;
    }

    inline float process()
    {
        if (!isActive || amount < 0.005f)
            return 0.0f;

        // 1. Update FM phases
        float modInc = (2.0f * 3.14159265f * modFreq) / static_cast<float>(currentSampleRate);
        float carrierInc = (2.0f * 3.14159265f * carrierFreq) / static_cast<float>(currentSampleRate);

        phaseMod += modInc;
        if (phaseMod > 6.283185f) phaseMod -= 6.283185f;

        float modSig = std::sin(phaseMod) * effectiveIndex * envLevel;

        phaseCarrier += carrierInc + modSig * 0.15f;
        if (phaseCarrier > 6.283185f) phaseCarrier -= 6.283185f;

        float carrierSig = std::sin(phaseCarrier + modSig);

        // 2. Non-linear buzzing chatter (membrane rattling against the cachimba hole)
        float buzzChatter = (carrierSig > 0.0f) ? (carrierSig * 1.3f - 0.3f * carrierSig * carrierSig * carrierSig)
                                                : (carrierSig * 0.5f);

        // 3. Envelope decay towards sustain floor
        envLevel = sustainFloor + (envLevel - sustainFloor) * decayCoeff;
        if (envLevel < 0.0005f)
        {
            envLevel = 0.0f;
            isActive = false;
        }

        return buzzChatter * envLevel * amount * 0.85f;
    }

    bool isPlaying() const { return isActive; }

private:
    double currentSampleRate = 44100.0;
    float fundamentalFreq = 440.0f;
    float carrierFreq = 880.0f;
    float modFreq = 3168.0f;
    float phaseCarrier = 0.0f;
    float phaseMod = 0.0f;
    float envLevel = 0.0f;
    float decayCoeff = 0.999f;
    float sustainFloor = 0.0f;
    float effectiveIndex = 1.0f;
    float amount = 0.0f;
    float vel = 1.0f;
    float velSens = 0.8f;
    bool isActive = false;
};

} // namespace ExtasisDSP
