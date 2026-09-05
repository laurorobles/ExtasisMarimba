#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cmath>
#include <random>
#include <array>

// Simple Sine Oscillator with Phase Modulation (FM)
class FMOscillator {
public:
    void setFrequency(float freq, double sr) { phaseDelta = freq / sr; }
    void reset() { phase = 0.0f; }
    float getNextSample(float phaseMod = 0.0f) {
        float out = std::sin(2.0f * juce::MathConstants<float>::pi * (phase + phaseMod));
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
        if (decayMs < 0.1f) decayMs = 0.1f;
        decayMult = std::exp(-1.0 / ((decayMs * 0.001) * sr));
    }
    float getNext() {
        float out = current;
        current *= decayMult;
        if (current < 0.0001f) current = 0.0f;
        return out;
    }
    bool isActive() const { return current > 0.0f; }
    float getCurrent() const { return current; }
private:
    float current = 0.0f;
    float decayMult = 0.0f;
};

// Biquad Filter (used for HPF noise and BPF resonator)
class Biquad {
public:
    void setBPF(float freq, float Q, double sr) {
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float alpha = std::sin(w0) / (2.0f * Q);
        float a0 = 1.0f + alpha;
        b0 = alpha / a0; b1 = 0.0f; b2 = -alpha / a0;
        a1 = -2.0f * std::cos(w0) / a0; a2 = (1.0f - alpha) / a0;
    }
    void setHPF(float freq, float Q, double sr) {
        float w0 = 2.0f * juce::MathConstants<float>::pi * freq / sr;
        float alpha = std::sin(w0) / (2.0f * Q);
        float a0 = 1.0f + alpha;
        b0 = (1.0f + std::cos(w0)) / 2.0f / a0;
        b1 = -(1.0f + std::cos(w0)) / a0;
        b2 = (1.0f + std::cos(w0)) / 2.0f / a0;
        a1 = -2.0f * std::cos(w0) / a0;
        a2 = (1.0f - alpha) / a0;
    }
    float process(float x) {
        float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        x2 = x1; x1 = x; y2 = y1; y1 = y;
        return y;
    }
    void reset() { x1 = x2 = y1 = y2 = 0.0f; }
private:
    float b0=0, b1=0, b2=0, a1=0, a2=0;
    float x1=0, x2=0, y1=0, y2=0;
};

class DelayLine {
public:
    DelayLine(int maxSamples = 48000) { buffer.resize(maxSamples, 0.0f); }
    void write(float sample) { buffer[writePos] = sample; writePos = (writePos + 1) % buffer.size(); }
    float read(float delaySamples) {
        float readPos = static_cast<float>(writePos) - delaySamples;
        while (readPos < 0.0f) readPos += buffer.size();
        int i1 = static_cast<int>(readPos);
        int i2 = (i1 + 1) % buffer.size();
        float frac = readPos - i1;
        return buffer[i1] * (1.0f - frac) + buffer[i2] * frac;
    }
private:
    std::vector<float> buffer;
    int writePos = 0;
};

class MarimbaVoice {
public:
    void noteOn(int note, float vel, double sr,
                float p_hardness, float p_overtones, float p_material, float p_click,
                float p_tube, float p_decay, float p_buzz, float p_buzzVel,
                float p_organic, float p_spread) {
        
        noteNumber = note;
        sampleRate = sr;
        isNoteActive = true;
        
        // --- 44. Imperfect Tuning & 26. Note-dependent Envelope ---
        std::mt19937 gen(note * 12345);
        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        
        float detuneCents = dist(gen) * 8.0f * p_organic; // +/- 8 cents max
        float freq = 440.0f * std::pow(2.0f, (note - 69.0f + (detuneCents / 100.0f)) / 12.0f);
        fundamental = freq;
        
        // Acoustic Stereo Panning (Key-tracked: Low = Left, High = Right)
        float acousticPan = (note - 66.0f) / 30.0f; // C4 is center
        acousticPan = std::max(-1.0f, std::min(1.0f, acousticPan));
        pan = acousticPan * p_spread; // Spread scales the width
        
        // Randomizations based on organic
        float randomDecayShift = 1.0f + (dist(gen) * 0.2f * p_organic); 

        // --- 46. Dynamic Strike Variation (Per-Hit Randomness) ---
        // While the above 'gen' gives consistent imperfection PER KEY,
        // this gives variation PER STRIKE (hitting different spots on the bar)
        float strikeRand1 = (juce::Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f;
        float strikeRand2 = (juce::Random::getSystemRandom().nextFloat() * 2.0f) - 1.0f;

        // --- 20. FM Core Setup (Tuned Bar 1:4:10) ---
        carrier.reset(); modA.reset(); modB.reset();
        
        // Pitch Transient (+10 cents dropping to 0, slightly randomized per strike)
        float pitchHit = (10.0f + (strikeRand1 * 5.0f * p_organic)) / 1200.0f;
        pitchEnv.trigger(pitchHit, 20.0f, sr); 
        
        // Tuned Ratios (Point 4 & 55): 4.0 and 10.0
        // 'Material' parameter slightly de-tunes them to make the wood less perfect
        float ratioA = 4.0f + (p_material - 0.5f) * 0.5f; 
        float ratioB = 10.0f + (p_material - 0.5f) * 1.5f;
        modA.setFrequency(freq * ratioA, sr);
        modB.setFrequency(freq * ratioB, sr);
        
        // --- Spectral ADSRs & Velocity Mapping (Point 29 & 12) ---
        // Velocity heavily influences how much energy goes into upper modes, not just volume
        // We also apply up to +/- 15% random variation based on ORGANIC DRIFT for realism
        float dynamicFM = (vel * vel) * (1.0f + (strikeRand2 * 0.15f * p_organic));
        
        // 1. Modulator A (4x Mode): Medium decay
        float envADecay = 80.0f + (1.0f - p_hardness) * 100.0f; // 80ms to 180ms
        float indexA = (0.5f + p_hardness * 3.0f) * dynamicFM * (1.0f - p_overtones);
        modAEnv.trigger(indexA, envADecay, sr);
        
        // 2. Modulator B (10x Mode): Very Fast decay
        float envBDecay = 20.0f + (1.0f - p_hardness) * 30.0f; // 20ms to 50ms
        float indexB = (0.5f + p_hardness * 4.0f) * dynamicFM * p_overtones;
        modBEnv.trigger(indexB, envBDecay, sr);
        
        // 3. Amplitude Envelope (Macroescala)
        float macroDecay = 100.0f + p_decay * 400.0f; // 100ms to 500ms
        // Stronger Keytracking: High notes decay extremely fast, low notes ring out
        float keyTrack = std::pow(0.5f, (note - 60.0f) / 15.0f); 
        keyTrack = std::max(0.12f, std::min(2.5f, keyTrack));
        ampEnv.trigger(vel, macroDecay * keyTrack * randomDecayShift, sr);
        
        // --- 39. Attack Noise (TAK/TCK) ---
        noiseFilt.reset();
        noiseFilt.setHPF(2500.0f, 0.707f, sr); // HPF at 2.5kHz
        // The click also varies per strike
        float dynamicClick = p_click * (1.0f + (strikeRand1 * 0.2f * p_organic));
        noiseEnv.trigger(dynamicFM * dynamicClick * 1.5f, 5.0f, sr); // 5ms click envelope
        
        // --- 10 & 41. Resonator & Delay (El Tubo) ---
        tubeFilt.reset();
        // Slightly detuned resonator (Point 45)
        float tubeFreq = freq * (1.0f + dist(gen) * 0.01f * p_organic);
        tubeFilt.setBPF(tubeFreq, 15.0f, sr); 
        tubeAmount = p_tube * 1.5f;
        
        // --- 13. Charleo Buzz ---
        charleoAmount = p_buzz;
        charleoThreshold = 1.0f - p_buzzVel; 
    }

    void noteOff() {
        ampEnv.trigger(ampEnv.getCurrent(), 50.0f, sampleRate);
        modAEnv.trigger(modAEnv.getCurrent(), 10.0f, sampleRate);
        modBEnv.trigger(modBEnv.getCurrent(), 10.0f, sampleRate);
    }

    void process(float& outL, float& outR) {
        if (!isNoteActive) return;
        
        // 1. Pitch Transient
        float pitchMult = std::pow(2.0f, pitchEnv.getNext());
        carrier.setFrequency(fundamental * pitchMult, sampleRate);
        
        // 2. FM Modulation (Mod A and Mod B)
        float fm = 0.0f;
        if (modAEnv.isActive()) fm += modA.getNextSample() * modAEnv.getNext();
        if (modBEnv.isActive()) fm += modB.getNextSample() * modBEnv.getNext();
        
        // 3. Carrier (The Wood Body)
        float body = carrier.getNextSample(fm) * ampEnv.getNext();
        
        // 4. Attack Noise (The Hit)
        float click = 0.0f;
        if (noiseEnv.isActive()) {
            float white = ((rand() % 1000) / 500.0f) - 1.0f;
            click = noiseFilt.process(white) * noiseEnv.getNext();
        }
        
        // Combined Source
        float source = body + click;
        
        // 5. The Resonator Tube (Delay + Filter)
        // Delay 20ms to simulate acoustic distance to tube (Point 41)
        tubeDelay.write(source);
        float delayedSource = tubeDelay.read(0.02f * sampleRate);
        float tube = tubeFilt.process(delayedSource) * tubeAmount;
        
        // 6. Charleo Buzz (Membrane distortion)
        float buzz = 0.0f;
        if (charleoAmount > 0.0f) {
            float velNorm = ampEnv.getCurrent();
            if (velNorm > charleoThreshold) {
                float white = ((rand() % 1000) / 500.0f) - 1.0f;
                buzz = white * tube * charleoAmount * 2.0f;
            }
        }
        
        float finalOut = source + tube + buzz;
        // Point 53: Soft Saturation (1-3dB drive)
        finalOut = std::tanh(finalOut * 1.25f);
        
        outL += finalOut * (1.0f - pan);
        outR += finalOut * (1.0f + pan);
        
        if (!ampEnv.isActive() && !noiseEnv.isActive()) {
            isNoteActive = false;
        }
    }
    
    bool isVoiceActive() const { return isNoteActive; }
    int noteNumber = -1;

private:
    bool isNoteActive = false;
    double sampleRate = 48000.0;
    float fundamental = 440.0f;
    float pan = 0.0f;
    
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
    void prepare(double sr, int samplesPerBlock) {
        sampleRate = sr;
    }

    void setParameters(float hardness, float overtones, float material, float click,
                       float tube, float decay, float buzz, float buzzVel,
                       float organic, float ambience, float spread, float volume) {
        p_hardness = hardness; p_overtones = overtones; p_material = material; p_click = click;
        p_tube = tube; p_decay = decay; p_buzz = buzz; p_buzzVel = buzzVel;
        p_organic = organic; p_ambience = ambience; p_spread = spread; p_volume = volume;
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
        for (auto& v : voices) if (v.isVoiceActive() && v.noteNumber == midiNote) v.noteOff();
    }
    void reset() { allNotesOff(); }
    void allNotesOff() {
        for (auto& v : voices) if (v.isVoiceActive()) v.noteOff();
    }

    void renderAudio(juce::AudioBuffer<float>& buffer, int numSamples) {
        auto* leftOut = buffer.getWritePointer(0);
        auto* rightOut = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;
        
        for (int s = 0; s < numSamples; ++s) {
            float mixedL = 0.0f, mixedR = 0.0f;
            for (auto& voice : voices) {
                if (voice.isVoiceActive()) {
                    voice.process(mixedL, mixedR);
                }
            }
            
            // Point 50: The Floor Reflection / Space (Ambience)
            // A simple diffuse delay network to simulate the sound projecting downwards and reflecting
            floorDelay.write(mixedL + mixedR);
            float floor1 = floorDelay.read(0.015f * sampleRate); // 15ms bounce
            float floor2 = floorDelay.read(0.035f * sampleRate); // 35ms bounce
            float room = (floor1 * 0.4f + floor2 * 0.2f) * p_ambience;
            
            float finalL = (mixedL + room) * p_volume * 0.5f;
            float finalR = (mixedR + room) * p_volume * 0.5f;
            
            leftOut[s] += finalL;
            if (rightOut != nullptr) rightOut[s] += finalR;
        }
    }

private:
    int findFreeVoice() {
        for (int i=0; i<16; ++i) if (!voices[i].isVoiceActive()) return i;
        return 0; // steal 0
    }
    MarimbaVoice voices[16];
    DelayLine floorDelay;
    double sampleRate = 48000.0;
    
    // 12 Parameters
    float p_hardness=0, p_overtones=0, p_material=0, p_click=0;
    float p_tube=0, p_decay=0, p_buzz=0, p_buzzVel=0;
    float p_organic=0, p_ambience=0, p_spread=0, p_volume=0;
};
