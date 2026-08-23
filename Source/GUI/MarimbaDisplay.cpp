#include "MarimbaDisplay.h"
#include "MarimbaLookAndFeel.h"
#include <cmath>

namespace ExtasisGUI
{

MarimbaDisplay::MarimbaDisplay()
{
    scopeBuffer.resize(scopeBufferSize, 0.0f);
    incomingFifo.resize(1024, 0.0f);
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
    readoutTimeoutCounter = 90; // 3 seconds
    repaint();
}

void MarimbaDisplay::pushAudioSamples(const float* samples, int numSamples)
{
    if (samples == nullptr || numSamples <= 0)
        return;

    const juce::ScopedLock sl(fifoLock);
    for (int i = 0; i < numSamples; ++i)
    {
        incomingFifo.push_back(samples[i]);
    }
    while (incomingFifo.size() > 2048)
    {
        incomingFifo.erase(incomingFifo.begin(), incomingFifo.begin() + 512);
    }
}

void MarimbaDisplay::triggerStrikeAnimation(float hardness, float decay)
{
    animDecay = 1.0f;
    animHardness = hardness;
}

void MarimbaDisplay::timerCallback()
{
    animPhase += 0.08f;
    if (animPhase > 6.283185f)
        animPhase -= 6.283185f;

    if (animDecay > 0.001f)
    {
        animDecay *= 0.90f;
    }

    // Process audio oscilloscope with zero crossing trigger
    {
        const juce::ScopedLock sl(fifoLock);
        if (incomingFifo.size() >= (size_t)scopeBufferSize)
        {
            size_t triggerIdx = 0;
            for (size_t i = 1; i < incomingFifo.size() - scopeBufferSize; ++i)
            {
                if (incomingFifo[i - 1] <= 0.0f && incomingFifo[i] > 0.0f)
                {
                    triggerIdx = i;
                    break;
                }
            }

            for (int i = 0; i < scopeBufferSize; ++i)
            {
                if (triggerIdx + i < incomingFifo.size())
                    scopeBuffer[i] = incomingFifo[triggerIdx + i];
            }

            incomingFifo.erase(incomingFifo.begin(), incomingFifo.begin() + juce::jmin((size_t)scopeBufferSize, incomingFifo.size()));
        }
    }

    if (readoutTimeoutCounter > 0)
    {
        readoutTimeoutCounter--;
        if (readoutTimeoutCounter == 0)
        {
            currentParamName = "MODAL PHYSICAL SYNTH";
            currentValueText = "16-VOICE POLYPHONY";
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
    auto lcdArea = bounds.reduced(7.0f);
    juce::Colour lcdDark(0xff0d1512);
    juce::Colour lcdGlow(0xff12261a);
    juce::ColourGradient lcdGrad(lcdDark, lcdArea.getX(), lcdArea.getY(),
                                lcdGlow, lcdArea.getRight(), lcdArea.getBottom(), false);
    g.setGradientFill(lcdGrad);
    g.fillRoundedRectangle(lcdArea, 4.0f);

    // LCD subtle scanlines
    g.setColour(juce::Colour(0x0a38e8d8));
    for (float y = lcdArea.getY(); y < lcdArea.getBottom(); y += 3.0f)
    {
        g.drawHorizontalLine((int)y, lcdArea.getX(), lcdArea.getRight());
    }

    // 3. Top Header: Patch Name
    auto headerArea = lcdArea.removeFromTop(22.0f).reduced(6.0f, 2.0f);
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 12.5f, juce::Font::bold));
    g.setColour(MarimbaLookAndFeel::getBrightAmber());
    g.drawFittedText("PATCH: " + currentPatchName.toUpperCase(), headerArea.toNearestInt(), juce::Justification::left, 1);

    // Top right text removed to avoid collision

    // Divider line
    g.setColour(juce::Colour(0x2838e8d8));
    g.drawHorizontalLine((int)lcdArea.getY(), lcdArea.getX() + 6.0f, lcdArea.getRight() - 6.0f);

    // Bottom Footer: Live Readout
    auto footerArea = lcdArea.removeFromBottom(20.0f).reduced(6.0f, 2.0f);
    g.setColour(juce::Colour(0x2038e8d8));
    g.drawHorizontalLine((int)footerArea.getY(), lcdArea.getX() + 6.0f, lcdArea.getRight() - 6.0f);
    
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));
    g.setColour(MarimbaLookAndFeel::getLcdCyan());
    g.drawFittedText(currentParamName + "  |  " + currentValueText, footerArea.toNearestInt(), juce::Justification::left, 1);

    // 4. Middle Area: Left (Oscilloscope) & Right (Modal Bars)
    auto middleArea = lcdArea.reduced(6.0f, 2.0f);
    auto rightModalArea = middleArea.removeFromRight(middleArea.getWidth() * 0.40f);
    auto rightScopeArea = middleArea; // It's actually the left area now, but we keep the variable name for the code below
    auto leftModalArea = rightModalArea;
    leftModalArea.translate(4.0f, 0.0f);

    // --- LEFT: 5 Modal Resonator Bars ---
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff557788));
    g.drawText("MODAL RESONATORS", leftModalArea.removeFromTop(11.0f), juce::Justification::centred, false);

    float barWidth = 14.0f;
    float spacing = 12.0f;
    float totalWidthNeeded = (barWidth * 5.0f) + (spacing * 4.0f);
    float startX = leftModalArea.getX() + (leftModalArea.getWidth() - totalWidthNeeded) * 0.5f;
    
    float heights[5] = { 0.88f, 0.65f, 0.45f, 0.30f, 0.55f };
    const char* labels[5] = { "f0", "4f0", "9.2f", "16f", "PIPE" };

    float barBaseY = leftModalArea.getBottom() - 13.0f;
    float maxBarH = leftModalArea.getHeight() - 16.0f;

    for (int i = 0; i < 5; ++i)
    {
        float bx = startX + static_cast<float>(i) * (barWidth + spacing);
        float currentH = maxBarH * heights[i] * (0.35f + animDecay * 0.65f + std::sin(animPhase + i * 1.2f) * 0.04f);
        float by = barBaseY - currentH;

        // Background slot
        g.setColour(juce::Colour(0x18203028));
        g.fillRoundedRectangle(bx, barBaseY - maxBarH, barWidth, maxBarH, 2.0f);

        // Active Bar Fill Gradient
        juce::Colour barCol = (i == 4) ? MarimbaLookAndFeel::getWoodWarmth() : MarimbaLookAndFeel::getLcdCyan();
        g.setGradientFill(juce::ColourGradient(barCol.brighter(0.4f), bx, by,
                                               barCol.darker(0.6f), bx, barBaseY, false));
        g.fillRoundedRectangle(bx, by, barWidth, currentH, 2.0f);

        // Top glow cap
        g.setColour(barCol.brighter(0.8f));
        g.fillRect(bx + 1.0f, by, barWidth - 2.0f, 2.0f);

        // Label
        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff88a0b0));
        g.drawText(labels[i], bx - 4.0f, barBaseY + 2.0f, barWidth + 8.0f, 10.0f, juce::Justification::centred, false);
    }

    // Divider Line
    g.setColour(juce::Colour(0x2238e8d8));
    g.drawVerticalLine((int)(rightScopeArea.getRight() + 4.0f), middleArea.getY(), middleArea.getBottom());

    // --- RIGHT: Real-time Oscilloscope (Scope Buffer) ---
    g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 8.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xff557788));
    g.drawText("OUTPUT OSCILLOSCOPE", rightScopeArea.removeFromTop(11.0f), juce::Justification::left, false);

    // Scope Frame
    g.setColour(juce::Colour(0x1238e8d8));
    g.fillRect(rightScopeArea);
    g.setColour(juce::Colour(0x2438e8d8));
    g.drawRect(rightScopeArea, 1.0f);

    // Center baseline
    float midY = rightScopeArea.getCentreY();
    g.setColour(juce::Colour(0x2038e8d8));
    g.drawHorizontalLine((int)midY, rightScopeArea.getX(), rightScopeArea.getRight());

    // Draw Vector Waveform Path
    juce::Path wavePath;
    float xStep = rightScopeArea.getWidth() / (float)scopeBufferSize;
    float yHalf = rightScopeArea.getHeight() * 0.44f;

    for (int i = 0; i < scopeBufferSize; ++i)
    {
        float x = rightScopeArea.getX() + i * xStep;
        float sample = juce::jlimit(-1.0f, 1.0f, scopeBuffer[i]);
        float y = midY - sample * yHalf;

        if (i == 0)
            wavePath.startNewSubPath(x, y);
        else
            wavePath.lineTo(x, y);
    }

    // Phosphor glow (thick background trace)
    g.setColour(MarimbaLookAndFeel::getAmberGold().withAlpha(0.35f));
    g.strokePath(wavePath, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Sharp bright trace
    g.setColour(MarimbaLookAndFeel::getBrightAmber());
    g.strokePath(wavePath, juce::PathStrokeType(1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

} // namespace ExtasisGUI
