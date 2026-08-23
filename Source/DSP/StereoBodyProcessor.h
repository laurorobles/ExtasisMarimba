#pragma once
#include <cmath>
#include <vector>
#include <algorithm>

namespace ExtasisDSP
{

class StereoBodyProcessor
{
public:
    StereoBodyProcessor() = default;

    void prepare(double sampleRate)
    {
        currentSampleRate = sampleRate;
        int maxDelay = static_cast<int>(sampleRate * 0.05); // 50ms max for body reflections
        delayL.assign(maxDelay, 0.0f);
        delayR.assign(maxDelay, 0.0f);
        writeIndex = 0;
    }

    void reset()
    {
        std::fill(delayL.begin(), delayL.end(), 0.0f);
        std::fill(delayR.begin(), delayR.end(), 0.0f);
        writeIndex = 0;
    }

    void process(float& left, float& right, float stereoSpread, float drive, float bodyAmbience, float notePan = 0.0f)
    {
        // 1. Soft saturation / analog tube/tape drive
        float driveGain = 1.0f + drive * 2.5f;
        left = std::tanh(left * driveGain) / (1.0f + drive * 0.3f);
        right = std::tanh(right * driveGain) / (1.0f + drive * 0.3f);

        // 2. Note panning across the marimba keyboard (spread)
        float panL = std::cos((notePan * stereoSpread + 1.0f) * 0.25f * 3.14159265f);
        float panR = std::sin((notePan * stereoSpread + 1.0f) * 0.25f * 3.14159265f);

        float mid = (left + right) * 0.5f;
        float side = (left - right) * 0.5f * (1.0f + stereoSpread);

        left = (mid + side) * panL * 1.414f;
        right = (mid - side) * panR * 1.414f;

        // 3. Acoustic body reflection / early resonator ambience
        if (bodyAmbience > 0.001f && !delayL.empty())
        {
            int d1 = static_cast<int>(currentSampleRate * 0.013); // 13ms
            int d2 = static_cast<int>(currentSampleRate * 0.019); // 19ms
            int size = static_cast<int>(delayL.size());

            int rIdx1 = (writeIndex - d1 + size) % size;
            int rIdx2 = (writeIndex - d2 + size) % size;

            float reflL = delayL[rIdx1] * 0.4f - delayR[rIdx2] * 0.25f;
            float reflR = delayR[rIdx2] * 0.4f - delayL[rIdx1] * 0.25f;

            delayL[writeIndex] = left + reflL * 0.3f;
            delayR[writeIndex] = right + reflR * 0.3f;

            writeIndex = (writeIndex + 1) % size;

            left += reflL * bodyAmbience * 0.6f;
            right += reflR * bodyAmbience * 0.6f;
        }
    }

private:
    double currentSampleRate = 44100.0;
    std::vector<float> delayL;
    std::vector<float> delayR;
    int writeIndex = 0;
};

} // namespace ExtasisDSP
