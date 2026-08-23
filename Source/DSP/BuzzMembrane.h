#pragma once
#include <cmath>
#include <algorithm>
#include <random>

namespace ExtasisDSP
{

class BuzzMembrane
{
public:
    BuzzMembrane() : rng(std::random_device{}()), dist(-1.0f, 1.0f) {}

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        
        // Setup 1-pole filters for the 3kHz - 5.5kHz noise bandpass
        float hpFreq = 3000.0f;
        float lpFreq = 5500.0f;
        
        hpCoeff = std::exp(-2.0f * 3.14159265f * hpFreq / static_cast<float>(currentSampleRate));
        lpCoeff = std::exp(-2.0f * 3.14159265f * lpFreq / static_cast<float>(currentSampleRate));

        reset();
    }

    void reset()
    {
        phaseCarrier = 0.0f;
        phaseMod = 0.0f;
        envLevel = 0.0f;
        noiseState1 = 0.0f;
        noiseState2 = 0.0f;
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

        // FM Core for the Tonal Buzz element
        carrierFreq = fundamentalFreq * 2.0f;
        modFreq = fundamentalFreq * 7.2f;

        // Dynamic Velocity Scaling (Non-Linear Excitation)
        float dynamicVelocityScale = (1.0f - velSens) + velSens * (vel * vel * vel); // Cubed for explosive attack
        effectiveIndex = amount * (1.2f + dynamicVelocityScale * 4.5f);

        envLevel = 1.0f;
        
        // Decay in approx 60ms - 180ms (Very fast drop for the Erosion / Wide Noise charleo)
        float decayMs = 60.0f + (1.0f - amount) * 120.0f;
        decayCoeff = std::exp(-4.6f / ((decayMs * 0.001f) * static_cast<float>(currentSampleRate)));
        
        // Charleo doesn't sustain much unless amount is very high
        sustainFloor = amount * 0.08f * dynamicVelocityScale;

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

        // --- 1. Tonal Membrane Buzz (Non-Linear FM) ---
        float modInc = (2.0f * 3.14159265f * modFreq) / static_cast<float>(currentSampleRate);
        float carrierInc = (2.0f * 3.14159265f * carrierFreq) / static_cast<float>(currentSampleRate);

        phaseMod += modInc;
        if (phaseMod > 6.283185f) phaseMod -= 6.283185f;

        float modSig = std::sin(phaseMod) * effectiveIndex * envLevel;

        phaseCarrier += carrierInc + modSig * 0.25f;
        if (phaseCarrier > 6.283185f) phaseCarrier -= 6.283185f;

        float carrierSig = std::sin(phaseCarrier + modSig);
        
        // Asymmetric Chatter (Membrane rattling against wax)
        float tonalChatter = (carrierSig > 0.0f) ? (carrierSig * 1.5f - 0.5f * carrierSig * carrierSig * carrierSig)
                                                 : (carrierSig * 0.6f);

        // --- 2. Wide Noise Charleo (Ableton Erosion Style) ---
        // Generates white noise, bandpassed at 3kHz - 5.5kHz
        float rawNoise = dist(rng);
        
        // 1-pole high-pass
        noiseState1 = hpCoeff * noiseState1 + (1.0f - hpCoeff) * rawNoise;
        float hpOut = rawNoise - noiseState1;
        
        // 1-pole low-pass
        noiseState2 = lpCoeff * noiseState2 + (1.0f - lpCoeff) * hpOut;
        float noiseCharleo = noiseState2 * 2.5f; // Gain compensation

        // The charleo noise only triggers heavily on high velocities (Threshold behavior)
        float noiseThreshold = vel * vel;
        
        // Blend Tonal Buzz and Noise Charleo
        float blendedBuzz = tonalChatter * 0.6f + noiseCharleo * 0.4f * noiseThreshold;

        // --- 3. Envelope follower logic ---
        envLevel = sustainFloor + (envLevel - sustainFloor) * decayCoeff;
        if (envLevel < 0.0005f)
        {
            envLevel = 0.0f;
            isActive = false;
        }

        // Apply Envelope and Amount to final buzz
        return blendedBuzz * envLevel * amount * 1.2f;
    }

    bool isPlaying() const { return isActive; }

private:
    double currentSampleRate = 44100.0;
    float fundamentalFreq = 440.0f;
    float carrierFreq = 880.0f;
    float modFreq = 3168.0f;
    float phaseCarrier = 0.0f;
    float phaseMod = 0.0f;
    
    // Noise Generator
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
    float hpCoeff = 0.0f;
    float lpCoeff = 0.0f;
    float noiseState1 = 0.0f;
    float noiseState2 = 0.0f;

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
