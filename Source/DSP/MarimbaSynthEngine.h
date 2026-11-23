#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <cstdint>
#include <array>
#include <algorithm>
#include <atomic>

// Lock-free fast PRNG for real-time audio thread (no mutexes, no allocations)
class FastRng {
public:
    explicit FastRng(uint32_t seed = 0x12345678u) : state(seed ? seed : 0x12345678u) {}
    void seed(uint32_t s) { state = s ? s : 0x12345678u; }
    inline uint32_t nextU32() {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }
    // Uniform float in [-1.0, 1.0]
    inline float nextBipolar() {
        return (static_cast<float>(nextU32() & 0x00FFFFFFu) / 8388608.0f) - 1.0f;
    }
private:
    uint32_t state;
};

// Simple Sine Oscillator with Phase Modulation (FM)
class FMOscillator {
public:
    void setFrequency(float freq, double sr) {
        phaseDelta = static_cast<float>(freq / sr);
    }
    void reset() { phase = 0.0f; }
    inline float getNextSample(float phaseMod = 0.0f) {
        float out = std::sin(juce::MathConstants<float>::twoPi * (phase + phaseMod));
        phase += phaseDelta;
        if (phase >= 1.0f) phase -= 1.0f;
        return out;
    }
private:
    float phase = 0.0f;
    float phaseDelta = 0.0f;
};

// Exponential Envelope Generator
class ExpEnv {
public:
    void trigger(float peak, float decayMs, double sr) {
        current = peak;
        if (decayMs < 0.2f) decayMs = 0.2f;
        decayMult = static_cast<float>(std::exp(-1.0 / ((decayMs * 0.001) * sr)));
    }
    inline float getNext() {
        float out = current;
        current *= decayMult;
        if (current < 0.0001f) current = 0.0f;
        return out;
    }
    void reset() { current = 0.0f; }
    bool isActive() const { return current > 0.0f; }
    float getCurrent() const { return current; }
private:
    float current = 0.0f;
    float decayMult = 0.0f;
};

// Biquad Filter (used for HPF click noise and BPF resonator tube)
class Biquad {
public:
    void setBPF(float freq, float Q, double sr) {
        float safeFreq = juce::jlimit(20.0f, static_cast<float>(sr * 0.45), freq);
        float w0 = juce::MathConstants<float>::twoPi * safeFreq / static_cast<float>(sr);
        float alpha = std::sin(w0) / (2.0f * std::max(0.1f, Q));
        float a0 = 1.0f + alpha;
        b0 = alpha / a0; b1 = 0.0f; b2 = -alpha / a0;
        a1 = -2.0f * std::cos(w0) / a0; a2 = (1.0f - alpha) / a0;
    }
    void setHPF(float freq, float Q, double sr) {
        float safeFreq = juce::jlimit(20.0f, static_cast<float>(sr * 0.45), freq);
        float w0 = juce::MathConstants<float>::twoPi * safeFreq / static_cast<float>(sr);
        float cosW0 = std::cos(w0);
        float alpha = std::sin(w0) / (2.0f * std::max(0.1f, Q));
        float a0 = 1.0f + alpha;
        b0 = (1.0f + cosW0) * 0.5f / a0;
        b1 = -(1.0f + cosW0) / a0;
        b2 = (1.0f + cosW0) * 0.5f / a0;
        a1 = -2.0f * cosW0 / a0;
        a2 = (1.0f - alpha) / a0;
    }
    inline float process(float x) {
        float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        return y;
    }
    void reset() { x1 = x2 = y1 = y2 = 0.0f; }
private:
    float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float x1 = 0, x2 = 0, y1 = 0, y2 = 0;
};

class DelayLine {
public:
    explicit DelayLine(int maxSamples = 16384) {
        buffer.assign(static_cast<size_t>(std::max(64, maxSamples)), 0.0f);
    }
    void prepare(int maxSamples) {
        buffer.assign(static_cast<size_t>(std::max(64, maxSamples)), 0.0f);
        writePos = 0;
    }
    void reset() {
        std::fill(buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }
    inline void write(float sample) {
        buffer[static_cast<size_t>(writePos)] = sample;
        writePos++;
        if (writePos >= static_cast<int>(buffer.size())) writePos = 0;
    }
    inline float read(float delaySamples) const {
        const float sizeF = static_cast<float>(buffer.size());
        const float clamped = juce::jlimit(1.0f, sizeF - 2.0f, delaySamples);
        float readPos = static_cast<float>(writePos) - clamped;
        if (readPos < 0.0f) readPos += sizeF;
        int i1 = static_cast<int>(readPos);
        int i2 = i1 + 1;
        if (i2 >= static_cast<int>(buffer.size())) i2 = 0;
        float frac = readPos - static_cast<float>(i1);
        return buffer[static_cast<size_t>(i1)] * (1.0f - frac)
             + buffer[static_cast<size_t>(i2)] * frac;
    }
private:
    std::vector<float> buffer;
    int writePos = 0;
};

class MarimbaVoice {
public:
    void prepare(double sr, uint32_t voiceSeed) {
        sampleRate = sr;
        strikeRng.seed(voiceSeed);
        // 30ms max for resonator acoustic distance at any sample rate up to 192kHz
        tubeDelay.prepare(static_cast<int>(sr * 0.035) + 16);
        reset();
    }

    void reset() {
        isNoteActive = false;
        sustainedByPedal = false;
        noteNumber = -1;
        carrier.reset(); modA.reset(); modB.reset();
        ampEnv.reset(); modAEnv.reset(); modBEnv.reset();
        noiseEnv.reset(); pitchEnv.reset();
        noiseFilt.reset(); tubeFilt.reset();
        tubeDelay.reset();
    }

    void noteOn(int note, float vel, double sr,
                float p_hardness, float p_overtones, float p_material, float p_click,
                float p_tube, float p_decay, float p_buzz, float p_buzzVel,
                float p_organic, float p_spread) {

        noteNumber = note;
        sampleRate = sr;
        isNoteActive = true;
        sustainedByPedal = false;

        // Deterministic per-key PRNG ("Marimba de Pueblo" organic imperfection)
        FastRng keyRng(static_cast<uint32_t>((note + 1) * 2654435761u));
        float keyDetuneRand = keyRng.nextBipolar();
        float keyDecayRand  = keyRng.nextBipolar();
        float keyTubeRand   = keyRng.nextBipolar();

        float detuneCents = keyDetuneRand * 8.0f * p_organic; // +/- 8 cents max
        float freq = 440.0f * std::pow(2.0f, (static_cast<float>(note) - 69.0f + (detuneCents / 100.0f)) / 12.0f);
        fundamental = freq;

        // Equal-Power Acoustic Stereo Panning (Key-tracked: Low = Left, High = Right)
        float acousticPan = juce::jlimit(-1.0f, 1.0f, (static_cast<float>(note) - 64.0f) / 28.0f);
        float pan = acousticPan * p_spread; // [-1, +1]
        float panAngle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        panGainL = std::cos(panAngle);
        panGainR = std::sin(panAngle);

        float randomDecayShift = 1.0f + (keyDecayRand * 0.2f * p_organic);

        // Dynamic Strike Variation (Per-Hit Randomness: hitting slightly different spots on the bar)
        float strikeRand1 = strikeRng.nextBipolar();
        float strikeRand2 = strikeRng.nextBipolar();

        // FM Core Setup (Hand-carved 1:4:10 Tuned Bar)
        carrier.reset(); modA.reset(); modB.reset();

        // Pitch Transient (+10 cents dropping to 0, subtly randomized per strike)
        float pitchHit = (10.0f + (strikeRand1 * 5.0f * p_organic)) / 1200.0f;
        pitchEnv.trigger(pitchHit, 20.0f, sr);

        // Tuned Ratios: 4.0x and 10.0x; 'Material' morphs wood carving imperfection
        float ratioA = 4.0f + (p_material - 0.5f) * 0.5f;
        float ratioB = 10.0f + (p_material - 0.5f) * 1.5f;
        float freqA = freq * ratioA;
        float freqB = freq * ratioB;
        float nyquistGuard = static_cast<float>(sr * 0.46);

        modA.setFrequency(std::min(freqA, nyquistGuard), sr);
        modB.setFrequency(std::min(freqB, nyquistGuard), sr);

        // Graceful anti-aliasing rolloff for high treble upper modes
        float nyqAttenA = (freqA < nyquistGuard) ? 1.0f : 0.0f;
        float nyqAttenB = (freqB < nyquistGuard) ? 1.0f : 0.0f;

        // Spectral ADSRs & Velocity Mapping
        float dynamicFM = (vel * vel) * (1.0f + (strikeRand2 * 0.15f * p_organic));

        // 1. Modulator A (4x Mode): Medium decay
        float envADecay = 80.0f + (1.0f - p_hardness) * 100.0f;
        float indexA = (0.5f + p_hardness * 3.0f) * dynamicFM * (1.0f - p_overtones * 0.7f) * nyqAttenA;
        modAEnv.trigger(indexA, envADecay, sr);

        // 2. Modulator B (10x Mode): Very fast decay
        float envBDecay = 20.0f + (1.0f - p_hardness) * 30.0f;
        float indexB = (0.5f + p_hardness * 4.0f) * dynamicFM * p_overtones * nyqAttenB;
        modBEnv.trigger(indexB, envBDecay, sr);

        // 3. Amplitude Envelope (Macroescala) with physical key-tracking
        float macroDecay = 100.0f + p_decay * 450.0f; // 100ms to 550ms
        float keyTrack = std::pow(0.5f, (static_cast<float>(note) - 60.0f) / 15.0f);
        keyTrack = juce::jlimit(0.12f, 2.6f, keyTrack);
        ampEnv.trigger(vel, macroDecay * keyTrack * randomDecayShift, sr);

        // Attack Noise (TAK/TCK rubber/wood mallet transient)
        noiseFilt.reset();
        noiseFilt.setHPF(2500.0f, 0.707f, sr);
        float dynamicClick = p_click * (1.0f + (strikeRand1 * 0.2f * p_organic));
        noiseEnv.trigger(dynamicFM * dynamicClick * 1.5f, 5.0f, sr);

        // Resonator Tube Cavity (Delay + Narrow BPF)
        tubeFilt.reset();
        tubeDelay.reset();
        float tubeFreq = freq * (1.0f + keyTubeRand * 0.01f * p_organic);
        tubeFilt.setBPF(tubeFreq, 15.0f, sr);
        tubeAmount = p_tube * 1.5f;

        // La Cachimba (Membrane Buzz)
        charleoAmount = p_buzz;
        charleoThreshold = 1.0f - p_buzzVel;
    }

    void noteOff(bool pedalDown) {
        if (pedalDown) {
            sustainedByPedal = true;
            return;
        }
        sustainedByPedal = false;
        ampEnv.trigger(ampEnv.getCurrent(), 65.0f, sampleRate);
        modAEnv.trigger(modAEnv.getCurrent(), 12.0f, sampleRate);
        modBEnv.trigger(modBEnv.getCurrent(), 10.0f, sampleRate);
    }

    void releasePedal() {
        if (sustainedByPedal && isNoteActive) {
            noteOff(false);
        }
    }

    inline void process(float& outL, float& outR, float pitchBendMult) {
        if (!isNoteActive) return;

        // 1. Pitch Transient + Pitch Bend
        float pitchMult = std::pow(2.0f, pitchEnv.getNext()) * pitchBendMult;
        carrier.setFrequency(fundamental * pitchMult, sampleRate);

        // 2. FM Modulation (Mod A and Mod B)
        float fm = 0.0f;
        if (modAEnv.isActive()) fm += modA.getNextSample() * modAEnv.getNext();
        if (modBEnv.isActive()) fm += modB.getNextSample() * modBEnv.getNext();

        // 3. Carrier (The Wood Body)
        float amp = ampEnv.getNext();
        float body = carrier.getNextSample(fm) * amp;

        // 4. Attack Noise (The Hit)
        float click = 0.0f;
        if (noiseEnv.isActive()) {
            float white = strikeRng.nextBipolar();
            click = noiseFilt.process(white) * noiseEnv.getNext();
        }

        // Combined Source
        float source = body + click;

        // 5. The Resonator Tube (20ms Acoustic Distance Delay + Resonant BPF)
        tubeDelay.write(source);
        float delayedSource = tubeDelay.read(0.02f * static_cast<float>(sampleRate));
        float tube = tubeFilt.process(delayedSource) * tubeAmount;

        // 6. La Cachimba / Charleo Buzz (Pressure-Gated Membrane Distortion)
        float buzz = 0.0f;
        if (charleoAmount > 0.001f && amp > charleoThreshold) {
            float white = strikeRng.nextBipolar();
            buzz = white * tube * charleoAmount * 2.0f;
        }

        float finalOut = source + tube + buzz;
        // Gentle analog soft saturation glue
        finalOut = std::tanh(finalOut * 1.25f);

        outL += finalOut * panGainL;
        outR += finalOut * panGainR;

        if (!ampEnv.isActive() && !noiseEnv.isActive()) {
            isNoteActive = false;
            sustainedByPedal = false;
        }
    }

    bool isVoiceActive() const { return isNoteActive; }
    float getAmplitude() const { return ampEnv.getCurrent(); }
    int noteNumber = -1;

private:
    bool isNoteActive = false;
    bool sustainedByPedal = false;
    double sampleRate = 48000.0;
    float fundamental = 440.0f;
    float panGainL = 0.7071f;
    float panGainR = 0.7071f;

    FastRng strikeRng;
    FMOscillator carrier, modA, modB;
    ExpEnv ampEnv, modAEnv, modBEnv, noiseEnv, pitchEnv;
    Biquad noiseFilt, tubeFilt;
    DelayLine tubeDelay;

    float tubeAmount = 0.0f;
    float charleoAmount = 0.0f;
    float charleoThreshold = 0.0f;
};

class MarimbaSynthEngine {
public:
    static constexpr int MAX_VOICES = 16;

    void prepare(double sr, int /*samplesPerBlock*/) {
        sampleRate = sr;
        for (int i = 0; i < MAX_VOICES; ++i) {
            voices[i].prepare(sr, static_cast<uint32_t>(0x9E3779B9u * (i + 1)));
        }
        int roomBufSize = static_cast<int>(sr * 0.08) + 32; // up to 80ms stereo room reflections
        roomDelayL.prepare(roomBufSize);
        roomDelayR.prepare(roomBufSize);
        smoothedVolume = p_volume;
        sustainPedalDown = false;
        pitchBendMult = 1.0f;
        activeVoicesAtomic.store(0, std::memory_order_relaxed);
    }

    void setParameters(float hardness, float overtones, float material, float click,
                       float tube, float decay, float buzz, float buzzVel,
                       float organic, float ambience, float spread, float volume) {
        p_hardness = hardness; p_overtones = overtones; p_material = material; p_click = click;
        p_tube = tube; p_decay = decay; p_buzz = buzz; p_buzzVel = buzzVel;
        p_organic = organic; p_ambience = ambience; p_spread = spread; p_volume = volume;
    }

    void setSustainPedal(bool isDown) {
        sustainPedalDown = isDown;
        if (!isDown) {
            for (auto& v : voices) v.releasePedal();
        }
    }

    // pitchWheelValue in [0, 16383], center 8192 -> +/- 2 semitones
    void setPitchWheel(int pitchWheelValue) {
        float norm = (static_cast<float>(pitchWheelValue) - 8192.0f) / 8192.0f;
        pitchBendMult = std::pow(2.0f, (norm * 2.0f) / 12.0f);
    }

    void noteOn(int midiNote, float vel) {
        if (vel <= 0.001f) { noteOff(midiNote); return; }
        int vIdx = findFreeVoice();
        voices[vIdx].noteOn(midiNote, vel, sampleRate,
                            p_hardness, p_overtones, p_material, p_click,
                            p_tube, p_decay, p_buzz, p_buzzVel,
                            p_organic, p_spread);
    }

    void noteOff(int midiNote) {
        for (auto& v : voices) {
            if (v.isVoiceActive() && v.noteNumber == midiNote)
                v.noteOff(sustainPedalDown);
        }
    }

    void reset() {
        for (auto& v : voices) v.reset();
        roomDelayL.reset();
        roomDelayR.reset();
        sustainPedalDown = false;
        pitchBendMult = 1.0f;
        activeVoicesAtomic.store(0, std::memory_order_relaxed);
    }

    void allNotesOff() {
        sustainPedalDown = false;
        for (auto& v : voices) {
            if (v.isVoiceActive()) v.noteOff(false);
        }
    }

    int getActiveVoiceCount() const {
        return activeVoicesAtomic.load(std::memory_order_relaxed);
    }

    void renderAudio(juce::AudioBuffer<float>& buffer, int numSamples) {
        auto* leftOut = buffer.getWritePointer(0);
        auto* rightOut = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
        const float srF = static_cast<float>(sampleRate);
        const float volSmoothCoeff = 0.005f;

        for (int s = 0; s < numSamples; ++s) {
            smoothedVolume += (p_volume - smoothedVolume) * volSmoothCoeff;

            float mixedL = 0.0f, mixedR = 0.0f;
            for (auto& voice : voices) {
                if (voice.isVoiceActive()) {
                    voice.process(mixedL, mixedR, pitchBendMult);
                }
            }

            // Decorrelated Stereo Floor / Courtyard Reflections (Ambience)
            float tapL1 = roomDelayL.read(0.015f * srF);
            float tapL2 = roomDelayL.read(0.033f * srF);
            float tapR1 = roomDelayR.read(0.019f * srF);
            float tapR2 = roomDelayR.read(0.041f * srF);

            float roomL = (tapL1 * 0.42f + tapR2 * 0.24f) * p_ambience;
            float roomR = (tapR1 * 0.42f + tapL2 * 0.24f) * p_ambience;

            roomDelayL.write(mixedL + roomR * 0.22f);
            roomDelayR.write(mixedR + roomL * 0.22f);

            float finalL = (mixedL + roomL) * smoothedVolume * 0.65f;
            float finalR = (mixedR + roomR) * smoothedVolume * 0.65f;

            leftOut[s] += finalL;
            if (rightOut != nullptr) rightOut[s] += finalR;
        }

        int activeCount = 0;
        for (const auto& v : voices) {
            if (v.isVoiceActive()) activeCount++;
        }
        activeVoicesAtomic.store(activeCount, std::memory_order_relaxed);
    }

private:
    int findFreeVoice() {
        int quietestIdx = 0;
        float lowestAmp = 1e9f;
        for (int i = 0; i < MAX_VOICES; ++i) {
            if (!voices[i].isVoiceActive())
                return i;
            float amp = voices[i].getAmplitude();
            if (amp < lowestAmp) {
                lowestAmp = amp;
                quietestIdx = i;
            }
        }
        return quietestIdx;
    }

    MarimbaVoice voices[MAX_VOICES];
    DelayLine roomDelayL, roomDelayR;
    double sampleRate = 48000.0;
    bool sustainPedalDown = false;
    float pitchBendMult = 1.0f;
    float smoothedVolume = 0.85f;
    std::atomic<int> activeVoicesAtomic { 0 };

    // 12 Parameters
    float p_hardness = 0.65f, p_overtones = 0.50f, p_material = 0.50f, p_click = 0.30f;
    float p_tube = 0.70f, p_decay = 0.50f, p_buzz = 0.50f, p_buzzVel = 0.80f;
    float p_organic = 0.40f, p_ambience = 0.30f, p_spread = 0.50f, p_volume = 0.85f;
};
