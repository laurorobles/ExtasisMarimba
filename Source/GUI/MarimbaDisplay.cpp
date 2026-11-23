#include "MarimbaDisplay.h"
#include "MarimbaLookAndFeel.h"
#include <cmath>

namespace ExtasisGUI
{

MarimbaDisplay::MarimbaDisplay()
{
    pullBuffer.resize(pullBufferSize, 0.0f);
    scopeBuffer.resize(scopeBufferSize, 0.0f);
    startTimerHz(30);
}

MarimbaDisplay::~MarimbaDisplay()
{
    stopTimer();
}

void MarimbaDisplay::setPatchName(const juce::String& name)
{
    currentPatchName = name;
    repaint();
}

void MarimbaDisplay::setParameterReadout(const juce::String& paramName, const juce::String& valueText)
{
    currentParamName = paramName;
    currentValueText = valueText;
    readoutTimeoutCounter = 90; // 3 seconds at 30 Hz
    repaint();
}

void MarimbaDisplay::setModalParams(float hardness, float overtones, float tube, float buzz)
{
    paramHardness = hardness;
    paramOvertones = overtones;
    paramTube = tube;
    paramBuzz = buzz;
}

void MarimbaDisplay::triggerStrikeAnimation(float /*hardness*/, float /*decay*/)
{
    animDecay = 1.0f;
}

void MarimbaDisplay::timerCallback()
{
    animPhase += 0.09f;
    if (animPhase > juce::MathConstants<float>::twoPi)
        animPhase -= juce::MathConstants<float>::twoPi;

    if (animDecay > 0.001f)
        animDecay *= 0.88f;

    // Pull real-time telemetry lock-free from AudioProcessor
    if (onPullOutputRms)
    {
        float targetRms = juce::jlimit(0.0f, 1.0f, onPullOutputRms() * 3.2f);
        if (targetRms > smoothedRms)
            smoothedRms = smoothedRms * 0.35f + targetRms * 0.65f;
        else
            smoothedRms = smoothedRms * 0.82f + targetRms * 0.18f;
    }

    if (onPullActiveVoices)
        activeVoices = onPullActiveVoices();

    if (onPullScopeSamples)
    {
        onPullScopeSamples(pullBuffer.data(), pullBufferSize);

        // Zero-crossing trigger for stable waveform display
        int triggerIdx = 0;
        const int searchLimit = pullBufferSize - scopeBufferSize;
        for (int i = 1; i < searchLimit; ++i)
        {
            if (pullBuffer[static_cast<size_t>(i - 1)] <= 0.0f && pullBuffer[static_cast<size_t>(i)] > 0.0f)
            {
                triggerIdx = i;
                break;
            }
        }

        for (int i = 0; i < scopeBufferSize; ++i)
        {
            scopeBuffer[static_cast<size_t>(i)] = pullBuffer[static_cast<size_t>(triggerIdx + i)];
        }
    }

    if (readoutTimeoutCounter > 0)
    {
        readoutTimeoutCounter--;
        if (readoutTimeoutCounter == 0)
        {
            currentParamName = "PHYSICAL-FM ENGINE";
            currentValueText = "1:4:10 TUNED BAR // 16 VOICES";
        }
    }

    repaint();
}

void MarimbaDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. Dark Bezel Frame
    g.setColour(juce::Colour(0xff090b0d));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(0xff2d323a));
    g.drawRoundedRectangle(bounds.reduced(1.0f), 5.0f, 1.5f);

    // 2. LCD Screen Window
    auto lcdArea = bounds.reduced(6.5f);
    juce::Colour lcdDark(0xff0c1411);
    juce::Colour lcdGlow(0xff122419);
    juce::ColourGradient lcdGrad(lcdDark, lcdArea.getX(), lcdArea.getY(),
                                 lcdGlow, lcdArea.getRight(), lcdArea.getBottom(), false);
    g.setGradientFill(lcdGrad);
    g.fillRoundedRectangle(lcdArea, 4.0f);

    // LCD subtle scanlines
    g.setColour(juce::Colour(0x0938e8d8));
    for (float y = lcdArea.getY(); y < lcdArea.getBottom(); y += 3.0f)
    {
        g.drawHorizontalLine(static_cast<int>(y), lcdArea.getX(), lcdArea.getRight());
    }

    // 3. Top Header: Patch Name (Left) + Live Voice Counter (Right)
    auto headerArea = lcdArea.removeFromTop(21.0f).reduced(6.0f, 2.0f);
    auto voiceArea = headerArea.removeFromRight(84.0f);

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.5f, juce::Font::bold));
    g.setColour(MarimbaLookAndFeel::getBrightAmber());
    g.drawFittedText("PATCH: " + currentPatchName.toUpperCase(), headerArea.toNearestInt(), juce::Justification::left, 1);

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 9.5f, juce::Font::bold));
    g.setColour(activeVoices > 0 ? MarimbaLookAndFeel::getLcdCyan() : juce::Colour(0xff4a6a66));
    g.drawFittedText("VOICES " + juce::String(activeVoices) + "/16", voiceArea.toNearestInt(), juce::Justification::right, 1);

    // Divider line
    g.setColour(juce::Colour(0x2838e8d8));
    g.drawHorizontalLine(static_cast<int>(lcdArea.getY()), lcdArea.getX() + 6.0f, lcdArea.getRight() - 6.0f);

    // Bottom Footer: Live Readout
    auto footerArea = lcdArea.removeFromBottom(19.0f).reduced(6.0f, 1.0f);
    g.setColour(juce::Colour(0x2038e8d8));
    g.drawHorizontalLine(static_cast<int>(footerArea.getY()), lcdArea.getX() + 6.0f, lcdArea.getRight() - 6.0f);

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.0f, juce::Font::bold));
    g.setColour(MarimbaLookAndFeel::getLcdCyan());
    g.drawFittedText(currentParamName + "  |  " + currentValueText, footerArea.toNearestInt(), juce::Justification::left, 1);

    // 4. Middle Area: Left (Oscilloscope) & Right (Live Modal/Tube/Buzz Meters)
    auto middleArea = lcdArea.reduced(6.0f, 3.0f);
    auto modalArea = middleArea.removeFromRight(middleArea.getWidth() * 0.42f);
    auto scopeArea = middleArea;
    modalArea.translate(4.0f, 0.0f);

    // --- RIGHT: 5 Live Physical Energy Bars ---
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff557788));
    g.drawText("1:4:10 MODES & CAVITY", modalArea.removeFromTop(11.0f), juce::Justification::centred, false);

    float barWidth = 16.0f;
    float spacing = 10.0f;
    float totalWidthNeeded = (barWidth * 5.0f) + (spacing * 4.0f);
    float startX = modalArea.getX() + (modalArea.getWidth() - totalWidthNeeded) * 0.5f;

    float energy = juce::jlimit(0.0f, 1.0f, std::max(smoothedRms, animDecay * 0.55f));
    float barWeights[5] = {
        0.92f,                                                  // f0 fundamental
        0.25f + paramHardness * 0.55f * (1.0f - paramOvertones * 0.4f), // 4f0 mode
        0.15f + paramOvertones * 0.75f * (0.4f + paramHardness * 0.6f), // 10f0 mode
        0.15f + paramTube * 0.80f,                              // Resonator Tube
        0.10f + paramBuzz * 0.85f                               // Cachimba Buzz
    };
    const char* labels[5] = { "f0", "4f", "10f", "TUBE", "BUZZ" };

    float barBaseY = modalArea.getBottom() - 12.0f;
    float maxBarH = std::max(10.0f, modalArea.getHeight() - 14.0f);

    for (int i = 0; i < 5; ++i)
    {
        float bx = startX + static_cast<float>(i) * (barWidth + spacing);
        float idleShimmer = 0.03f * std::sin(animPhase + static_cast<float>(i) * 1.1f);
        float level = juce::jlimit(0.06f, 1.0f, barWeights[i] * (0.20f + energy * 0.80f) + idleShimmer);
        float currentH = maxBarH * level;
        float by = barBaseY - currentH;

        // Background slot
        g.setColour(juce::Colour(0x18203028));
        g.fillRoundedRectangle(bx, barBaseY - maxBarH, barWidth, maxBarH, 2.0f);

        // Active Bar Fill Gradient
        juce::Colour barCol = (i == 3) ? MarimbaLookAndFeel::getWoodWarmth()
                            : (i == 4) ? MarimbaLookAndFeel::getAmberGold()
                            : MarimbaLookAndFeel::getLcdCyan();
        g.setGradientFill(juce::ColourGradient(barCol.brighter(0.35f), bx, by,
                                               barCol.darker(0.60f), bx, barBaseY, false));
        g.fillRoundedRectangle(bx, by, barWidth, currentH, 2.0f);

        // Top glow cap
        g.setColour(barCol.brighter(0.85f));
        g.fillRect(bx + 1.0f, by, barWidth - 2.0f, 2.0f);

        // Label
        g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 7.8f, juce::Font::bold));
        g.setColour(juce::Colour(0xff88a0b0));
        g.drawText(labels[i], static_cast<int>(bx - 5.0f), static_cast<int>(barBaseY + 2.0f),
                   static_cast<int>(barWidth + 10.0f), 10, juce::Justification::centred, false);
    }

    // Divider Line
    g.setColour(juce::Colour(0x2238e8d8));
    g.drawVerticalLine(static_cast<int>(scopeArea.getRight() + 3.0f), middleArea.getY(), middleArea.getBottom());

    // --- LEFT: Real-time Oscilloscope ---
    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff557788));
    g.drawText("OUTPUT OSCILLOSCOPE", scopeArea.removeFromTop(11.0f), juce::Justification::left, false);

    // Scope Frame
    g.setColour(juce::Colour(0x1238e8d8));
    g.fillRect(scopeArea);
    g.setColour(juce::Colour(0x2438e8d8));
    g.drawRect(scopeArea, 1.0f);

    // Subtle gridlines
    float midY = scopeArea.getCentreY();
    g.setColour(juce::Colour(0x1838e8d8));
    g.drawHorizontalLine(static_cast<int>(midY), scopeArea.getX(), scopeArea.getRight());
    g.drawVerticalLine(static_cast<int>(scopeArea.getCentreX()), scopeArea.getY(), scopeArea.getBottom());

    // Draw Vector Waveform Path
    juce::Path wavePath;
    float xStep = scopeArea.getWidth() / static_cast<float>(scopeBufferSize - 1);
    float yHalf = scopeArea.getHeight() * 0.44f;

    for (int i = 0; i < scopeBufferSize; ++i)
    {
        float x = scopeArea.getX() + static_cast<float>(i) * xStep;
        float sample = juce::jlimit(-1.0f, 1.0f, scopeBuffer[static_cast<size_t>(i)] * 1.35f);
        float y = midY - sample * yHalf;

        if (i == 0)
            wavePath.startNewSubPath(x, y);
        else
            wavePath.lineTo(x, y);
    }

    // Phosphor glow
    g.setColour(MarimbaLookAndFeel::getAmberGold().withAlpha(0.35f));
    g.strokePath(wavePath, juce::PathStrokeType(3.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Sharp bright trace
    g.setColour(MarimbaLookAndFeel::getBrightAmber());
    g.strokePath(wavePath, juce::PathStrokeType(1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

} // namespace ExtasisGUI
