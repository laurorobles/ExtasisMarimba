#pragma once
#include "MalletExciter.h"
#include "ModalBarResonator.h"
#include "BuzzMembrane.h"
#include "SEMFilter.h"
#include "PercussiveEnvelope.h"
#include "StereoBodyProcessor.h"
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

namespace ExtasisDSP
{

struct MarimbaVoice
{
    MalletExciter exciter;
    ModalBarResonator resonator;
    BuzzMembrane buzzMembrane;
    SEMFilter filter;
    PercussiveEnvelope ampEnv;
    PercussiveEnvelope strikeEnv;

    int noteNumber = -1;
    float currentFreq = 440.0f;
    float targetFreq = 440.0f;
    float velocity = 0.0f;
    float keyPan = 0.0f;
    bool isNoteActive = false;
    uint32_t age = 0;

    void prepare(double sampleRate)
    {
        exciter.prepare(sampleRate);
        resonator.prepare(sampleRate);
        buzzMembrane.prepare(sampleRate);
        filter.prepare(sampleRate);
        ampEnv.prepare(sampleRate);
        strikeEnv.prepare(sampleRate);
        reset();
    }

    void reset()
    {
        exciter.reset();
        resonator.reset();
        buzzMembrane.reset();
        filter.reset();
        ampEnv.reset();
        strikeEnv.reset();
        noteNumber = -1;
        isNoteActive = false;
        age = 0;
    }

    void noteOn(int midiNote, float vel, float glideTimeMs, double sampleRate,
                float attackMs, float decayMs, float sustainLevel, float releaseMs, bool isSnap,
                float pitchDriftCents)
    {
        noteNumber = midiNote;
        velocity = vel;
        isNoteActive = true;
        age = 0;

        // Stereo panning across keyboard (C1 to C7)
        keyPan = (static_cast<float>(midiNote) - 60.0f) / 36.0f;
        keyPan = std::max(-1.0f, std::min(1.0f, keyPan));

        // Fundamental frequency with organic micro-pitch variation
        float noteWithDrift = static_cast<float>(midiNote) + (pitchDriftCents * 0.01f);
        targetFreq = 440.0f * std::pow(2.0f, (noteWithDrift - 69.0f) / 12.0f);
        
        if (currentFreq <= 0.0f || glideTimeMs < 1.0f)
        {
            currentFreq = targetFreq;
        }

        // Configure envelopes
        ampEnv.setParameters(attackMs, decayMs, sustainLevel, releaseMs, isSnap);
        strikeEnv.setParameters(0.5f, 35.0f, 0.0f, 20.0f, true);

        ampEnv.trigger(vel);
        strikeEnv.trigger(vel);
    }

    void noteOff()
    {
        ampEnv.noteOff();
        strikeEnv.noteOff();
        buzzMembrane.noteOff();
    }

    bool isVoiceActive() const
    {
        return isNoteActive && (ampEnv.isActive() || exciter.isPlaying() || resonator.hasEnergy() || buzzMembrane.isPlaying());
    }
};

class MarimbaSynthEngine
{
public:
    static constexpr int NUM_VOICES = 16;

    MarimbaSynthEngine() = default;

    void prepare(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;
        for (auto& voice : voices)
        {
            voice.prepare(sampleRate);
        }
        bodyProcessor.prepare(sampleRate);
        reset();
    }

    void reset()
    {
        for (auto& voice : voices)
        {
            voice.reset();
        }
        bodyProcessor.reset();
        voiceCounter = 0;
    }

    void noteOn(int midiNote, float velocity)
    {
        if (velocity <= 0.001f)
        {
            noteOff(midiNote);
            return;
        }

        int voiceIndex = findFreeVoice();
        auto& voice = voices[voiceIndex];

        // 1. Organic "Marimba de Pueblo" Deterministic Variations per Key
        uint32_t seed = static_cast<uint32_t>(midiNote * 2654435761u);
        float randPitch = ((float)(seed % 1000) / 500.0f - 1.0f) * 7.0f * organicDrift; // +/- 7 cents
        float randDecay = 1.0f + ((float)((seed >> 4) % 1000) / 500.0f - 1.0f) * 0.18f * organicDrift; // +/- 18% decay
        float randBuzz = 1.0f + ((float)((seed >> 8) % 1000) / 500.0f - 1.0f) * 0.25f * organicDrift; // +/- 25% buzz

        // 2. Trigger voice
        voice.noteOn(midiNote, velocity, glideTime, currentSampleRate,
                     attackMs, decayMs * randDecay, sustainLevel, releaseMs, isSnap, randPitch);

        // 3. Mallet Strike Exciter (Hardness + Noise + Click)
        float effectiveHardness = malletHardness + velocity * 0.25f * velToHardness;
        effectiveHardness = std::max(0.0f, std::min(1.0f, effectiveHardness));
        voice.exciter.trigger(velocity, effectiveHardness, clickAmount, noiseAmount);

        // 4. Modal Resonator & Tube Cavity
        voice.resonator.update(voice.currentFreq, resonatorDecay * randDecay, material, overtoneMix, tubeAmount, midiNote);

        // 5. La Cachimba (Buzz Membrane)
        voice.buzzMembrane.trigger(voice.currentFreq, velocity, buzzAmount * randBuzz, buzzVelSens);
    }

    void noteOff(int midiNote)
    {
        for (auto& voice : voices)
        {
            if (voice.noteNumber == midiNote && voice.isNoteActive)
            {
                voice.noteOff();
            }
        }
    }

    void allNotesOff()
    {
        for (auto& voice : voices)
        {
            voice.reset();
        }
    }

    void setParameters(float hardness, float noise, float click, float att,
                       float decay, float mat, float overtones, float organic,
                       float tube, float buzz, float buzzVel, float cutoffHz, float reso, float filterEnvAmt,
                       float dec, float sus, float rel, bool snap,
                       float spread, float drv, float ambience, float vol)
    {
        malletHardness = hardness;
        noiseAmount = noise;
        clickAmount = click;
        attackMs = att;

        resonatorDecay = decay;
        material = mat;
        overtoneMix = overtones;
        organicDrift = organic;

        tubeAmount = tube;
        buzzAmount = buzz;
        buzzVelSens = buzzVel;

        filterCutoff = cutoffHz;
        filterResonance = reso;
        filterEnvAmount = filterEnvAmt;

        decayMs = dec;
        sustainLevel = sus;
        releaseMs = rel;
        isSnap = snap;

        stereoSpread = spread;
        drive = drv;
        bodyAmbience = ambience;
        masterVolume = vol;
    }

    void renderAudio(juce::AudioBuffer<float>& buffer, int numSamples)
    {
        auto* leftOut = buffer.getWritePointer(0);
        auto* rightOut = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int s = 0; s < numSamples; ++s)
        {
            float mixedL = 0.0f;
            float mixedR = 0.0f;

            for (auto& voice : voices)
            {
                if (!voice.isVoiceActive())
                    continue;

                voice.age++;

                // Pitch glide smoothing
                if (glideTime > 1.0f && std::abs(voice.currentFreq - voice.targetFreq) > 0.1f)
                {
                    float glideCoeff = std::exp(-1.0f / (glideTime * 0.001f * static_cast<float>(currentSampleRate)));
                    voice.currentFreq = voice.targetFreq + (voice.currentFreq - voice.targetFreq) * glideCoeff;
                    voice.resonator.update(voice.currentFreq, resonatorDecay, material, overtoneMix, tubeAmount, voice.noteNumber);
                }

                // 1. Strike Exciter (Impulse + Noise + Click)
                float exciterSig = voice.exciter.getNextSample();

                // 2. Modal Bar Resonator + Acoustic Tube Cavity
                float modalSig = voice.resonator.process(exciterSig);

                // 3. La Cachimba (Buzz Membrane)
                float buzzSig = voice.buzzMembrane.process();

                // Sum modal bar sound with buzz membrane
                float rawVoiceAudio = modalSig + buzzSig;

                // 4. Envelopes
                float ampEnvSig = voice.ampEnv.getNextSample();
                float strikeEnvSig = voice.strikeEnv.getNextSample();

                // 5. SEM Filter with dynamic strike envelope
                float dynamicCutoff = filterCutoff + filterEnvAmount * strikeEnvSig * 7000.0f + voice.velocity * 1800.0f;
                dynamicCutoff = std::max(20.0f, std::min(dynamicCutoff, static_cast<float>(currentSampleRate * 0.45f)));

                voice.filter.setParameters(dynamicCutoff, filterResonance, SEMFilter::LowPass);
                float filtered = voice.filter.process(rawVoiceAudio);

                float voiceSample = filtered * ampEnvSig;

                // 6. Stereo Keyboard Panning
                float panAngle = (voice.keyPan * stereoSpread + 1.0f) * 0.25f * 3.14159265f;
                float vL = voiceSample * std::cos(panAngle);
                float vR = voiceSample * std::sin(panAngle);

                mixedL += vL;
                mixedR += vR;

                if (!voice.ampEnv.isActive() && !voice.exciter.isPlaying() && !voice.resonator.hasEnergy() && !voice.buzzMembrane.isPlaying())
                {
                    voice.isNoteActive = false;
                }
            }

            // Master Body Processor (Warm Drive, Body Ambience, Stereo Spread)
            bodyProcessor.process(mixedL, mixedR, stereoSpread, drive, bodyAmbience, 0.0f);

            mixedL *= masterVolume;
            mixedR *= masterVolume;

            leftOut[s] += mixedL;
            if (rightOut != nullptr)
            {
                rightOut[s] += mixedR;
            }
        }
    }

private:
    int findFreeVoice()
    {
        // 1. Search for inactive voice
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            if (!voices[i].isVoiceActive())
            {
                return i;
            }
        }

        // 2. Voice stealing
        uint32_t maxAge = 0;
        int oldestIndex = 0;
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            if (voices[i].age > maxAge)
            {
                maxAge = voices[i].age;
                oldestIndex = i;
            }
        }
        return oldestIndex;
    }

    double currentSampleRate = 44100.0;
    std::array<MarimbaVoice, NUM_VOICES> voices;
    StereoBodyProcessor bodyProcessor;
    uint32_t voiceCounter = 0;

    // Parameters
    float malletHardness = 0.65f;
    float noiseAmount = 0.35f;
    float clickAmount = 0.30f;
    float velToHardness = 0.5f;
    float attackMs = 0.5f;

    float resonatorDecay = 0.55f;
    float material = 0.10f; // Hormiguillo wood
    float overtoneMix = 0.60f;
    float organicDrift = 0.40f; // Marimba de Pueblo

    float tubeAmount = 0.65f; // DOOO~WONNNG tube cavity
    float buzzAmount = 0.45f; // La Cachimba buzz
    float buzzVelSens = 0.85f; // Dynamic buzz on hard velocity

    float filterCutoff = 11000.0f;
    float filterResonance = 0.15f;
    float filterEnvAmount = 0.40f;

    float decayMs = 900.0f;
    float sustainLevel = 0.0f;
    float releaseMs = 380.0f;
    bool isSnap = true;
    float glideTime = 0.0f;

    float stereoSpread = 0.65f;
    float drive = 0.15f;
    float bodyAmbience = 0.25f;
    float masterVolume = 0.85f;
};

} // namespace ExtasisDSP
