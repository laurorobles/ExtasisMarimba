#pragma once
#include "MalletExciter.h"
#include "ModalBarResonator.h"
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
    SEMFilter filter;
    PercussiveEnvelope ampEnv;
    PercussiveEnvelope strikeEnv; // Cycling/auxiliary strike modulation envelope

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
        filter.prepare(sampleRate);
        ampEnv.prepare(sampleRate);
        strikeEnv.prepare(sampleRate);
        reset();
    }

    void reset()
    {
        exciter.reset();
        resonator.reset();
        filter.reset();
        ampEnv.reset();
        strikeEnv.reset();
        noteNumber = -1;
        isNoteActive = false;
        age = 0;
    }

    void noteOn(int midiNote, float vel, float glideTimeMs, double sampleRate)
    {
        noteNumber = midiNote;
        velocity = vel;
        isNoteActive = true;
        age = 0;

        // Calculate stereo panning based on keyboard position (MIDI 24 C1 to MIDI 96 C7)
        keyPan = (static_cast<float>(midiNote) - 60.0f) / 36.0f;
        keyPan = std::max(-1.0f, std::min(1.0f, keyPan));

        targetFreq = 440.0f * std::pow(2.0f, (static_cast<float>(midiNote) - 69.0f) / 12.0f);
        if (currentFreq <= 0.0f || glideTimeMs < 1.0f)
        {
            currentFreq = targetFreq;
        }

        // Trigger envelopes
        ampEnv.trigger(vel);
        strikeEnv.trigger(vel);
    }

    void noteOff()
    {
        ampEnv.noteOff();
        strikeEnv.noteOff();
    }

    bool isVoiceActive() const
    {
        return isNoteActive && (ampEnv.isActive() || exciter.isPlaying());
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
        int voiceIndex = findFreeVoice();
        auto& voice = voices[voiceIndex];

        voice.noteOn(midiNote, velocity, glideTime, currentSampleRate);

        // Exciter trigger with velocity sensitivity
        float effectiveHardness = malletHardness + velocity * 0.25f * velToHardness;
        effectiveHardness = std::max(0.0f, std::min(1.0f, effectiveHardness));

        voice.exciter.trigger(velocity, effectiveHardness, clickAmount);

        // Configure envelopes
        voice.ampEnv.setParameters(attackMs, decayMs, sustainLevel, releaseMs, isSnap);
        // Fast auxiliary strike envelope (approx 15-40ms)
        voice.strikeEnv.setParameters(0.5f, 25.0f + effectiveHardness * 20.0f, 0.0f, 20.0f, true);

        // Update resonator
        voice.resonator.update(voice.currentFreq, resonatorDecay, material, overtoneMix, pipeBody);
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

    void setParameters(float hardness, float click, float decay, float mat, float overtones, float pipe,
                       float cutoffHz, float reso, float filterEnvAmt,
                       float att, float dec, float sus, float rel, bool snap,
                       float spread, float drv, float ambience, float vol)
    {
        malletHardness = hardness;
        clickAmount = click;
        resonatorDecay = decay;
        material = mat;
        overtoneMix = overtones;
        pipeBody = pipe;

        filterCutoff = cutoffHz;
        filterResonance = reso;
        filterEnvAmount = filterEnvAmt;

        attackMs = att;
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
                    voice.resonator.update(voice.currentFreq, resonatorDecay, material, overtoneMix, pipeBody);
                }

                // Generate mallet strike & modal resonance
                float exciterSig = voice.exciter.getNextSample();
                float modalSig = voice.resonator.process(exciterSig);

                // Envelopes
                float ampEnvSig = voice.ampEnv.getNextSample();
                float strikeEnvSig = voice.strikeEnv.getNextSample();

                // Dynamic Cutoff with envelope and velocity modulation
                float dynamicCutoff = filterCutoff + filterEnvAmount * strikeEnvSig * 8000.0f + voice.velocity * 2000.0f;
                dynamicCutoff = std::max(20.0f, std::min(dynamicCutoff, static_cast<float>(currentSampleRate * 0.45f)));

                voice.filter.setParameters(dynamicCutoff, filterResonance, SEMFilter::LowPass);
                float filtered = voice.filter.process(modalSig);

                float voiceSample = filtered * ampEnvSig;

                // Stereo Panning per voice
                float panAngle = (voice.keyPan * stereoSpread + 1.0f) * 0.25f * 3.14159265f;
                float vL = voiceSample * std::cos(panAngle);
                float vR = voiceSample * std::sin(panAngle);

                mixedL += vL;
                mixedR += vR;

                if (!voice.ampEnv.isActive() && !voice.exciter.isPlaying())
                {
                    voice.isNoteActive = false;
                }
            }

            // Master Body Processor (Saturation, Ambience, Spread)
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
        // 1. Search for an inactive voice
        for (int i = 0; i < NUM_VOICES; ++i)
        {
            if (!voices[i].isVoiceActive())
            {
                return i;
            }
        }

        // 2. Voice stealing: find the oldest or quietest voice
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
    float clickAmount = 0.35f;
    float velToHardness = 0.5f;
    float resonatorDecay = 0.55f;
    float material = 0.15f; // 0.0 = Wood, 1.0 = Glass
    float overtoneMix = 0.60f;
    float pipeBody = 0.50f;

    float filterCutoff = 10000.0f;
    float filterResonance = 0.15f;
    float filterEnvAmount = 0.40f;

    float attackMs = 0.5f;
    float decayMs = 550.0f;
    float sustainLevel = 0.0f;
    float releaseMs = 280.0f;
    bool isSnap = true;
    float glideTime = 0.0f;

    float stereoSpread = 0.65f;
    float drive = 0.12f;
    float bodyAmbience = 0.25f;
    float masterVolume = 0.85f;
};

} // namespace ExtasisDSP
