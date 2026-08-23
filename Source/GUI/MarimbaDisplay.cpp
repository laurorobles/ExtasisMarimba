#include "MarimbaDisplay.h"
#include "MarimbaLookAndFeel.h"
#include <cmath>

namespace ExtasisGUI
{

MarimbaDisplay::MarimbaDisplay()
{
    startTimerHz(30);
}

MarimbaDisplay::~MarimbaDisplay()
{
    stopTimer();
}

void MarimbaDisplay::setPresetName(const juce::String& name)
{
    currentPreset = name;
    repaint();
}

void MarimbaDisplay::setParameterReadout(const juce::String& paramName, const juce::String& paramValue)
{
    activeParam = paramName;
    activeValue = paramValue;
    repaint();
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
        repaint();
    }
}

void MarimbaDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. LCD Bezel & Background
    g.setColour(MarimbaLookAndFeel::getLcdBackground());
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(MarimbaLookAndFeel::getPanelBorder().brighter(0.15f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    // Subtle LCD Scanlines / Grid
    g.setColour(juce::Colour(0x0a38e8d8));
    for (float y = 4.0f; y < bounds.getHeight(); y += 4.0f)
    {
        g.drawHorizontalLine(static_cast<int>(y), 4.0f, bounds.getWidth() - 4.0f);
    }

    // 2. Header: Preset Name & Tag
    auto headerArea = bounds.removeFromTop(26.0f).reduced(10.0f, 4.0f);
    
    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.setColour(MarimbaLookAndFeel::getAmberGold());
    g.drawText(currentPreset, headerArea, juce::Justification::topLeft, false);

    g.setFont(juce::Font(10.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff7a8b9e));
    g.drawText("MODAL PHYSICAL MODELING // 16-VOICE", headerArea, juce::Justification::topRight, false);

    // Bottom Footer Area
    auto footerArea = bounds.removeFromBottom(20.0f).reduced(10.0f, 2.0f);
    g.setColour(juce::Colour(0x1538e8d8));
    g.drawHorizontalLine(static_cast<int>(footerArea.getY()), 8.0f, bounds.getWidth() - 8.0f);
    
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff88a0b8));
    g.drawText("STATUS: " + activeParam + "  " + activeValue, footerArea, juce::Justification::centredLeft, false);

    // 3. Middle Area: Divided into Left (Modal Resonators) & Right (Mallet Transient Wave)
    auto middleArea = bounds.reduced(10.0f, 2.0f);
    auto leftModalArea = middleArea.removeFromLeft(middleArea.getWidth() * 0.52f);
    auto rightWaveArea = middleArea;

    // --- LEFT: 5 Modal Resonant Bars ---
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff556677));
    g.drawText("HARMONIC MODAL RESONATORS", leftModalArea.removeFromTop(12.0f), juce::Justification::topLeft, false);

    float barWidth = 22.0f;
    float totalBarSpace = leftModalArea.getWidth() - 10.0f;
    float spacing = (totalBarSpace - (barWidth * 5.0f)) / 4.0f;
    
    float heights[5] = { 0.88f, 0.65f, 0.45f, 0.30f, 0.55f };
    const char* labels[5] = { "f0", "4f0", "9.2f", "16f", "PIPE" };

    float barBaseY = leftModalArea.getBottom() - 14.0f;
    float maxBarH = leftModalArea.getHeight() - 18.0f;

    for (int i = 0; i < 5; ++i)
    {
        float bx = leftModalArea.getX() + 4.0f + static_cast<float>(i) * (barWidth + spacing);
        float currentH = maxBarH * heights[i] * (0.35f + animDecay * 0.65f + std::sin(animPhase + i * 1.2f) * 0.04f);
        float by = barBaseY - currentH;

        juce::Rectangle<float> barRect(bx, by, barWidth, currentH);

        // Background inactive track slot
        g.setColour(juce::Colour(0x1a2d313a));
        g.fillRoundedRectangle(bx, barBaseY - maxBarH, barWidth, maxBarH, 2.0f);

        // Active Bar Fill Gradient
        juce::Colour barCol = (i == 4) ? MarimbaLookAndFeel::getWoodWarmth() : MarimbaLookAndFeel::getLcdCyan();
        g.setGradientFill(juce::ColourGradient(barCol.brighter(0.4f), bx, by,
                                               barCol.darker(0.6f), bx, barBaseY, false));
        g.fillRoundedRectangle(barRect, 2.0f);

        // Top glow cap
        g.setColour(barCol.brighter(0.8f));
        g.fillRect(bx + 2.0f, by, barWidth - 4.0f, 2.0f);

        // Label
        g.setFont(juce::Font(9.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xff8899aa));
        g.drawText(labels[i], bx - 4.0f, barBaseY + 2.0f, barWidth + 8.0f, 11.0f, juce::Justification::centred, false);
    }

    // Divider Line
    g.setColour(juce::Colour(0x2238e8d8));
    g.drawVerticalLine(static_cast<int>(rightWaveArea.getX() - 4.0f), middleArea.getY(), middleArea.getBottom());

    // --- RIGHT: Mallet Transient Oscilloscope ---
    g.setFont(juce::Font(9.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xff556677));
    g.drawText("MALLET STRIKE TRANSIENT", rightWaveArea.removeFromTop(12.0f), juce::Justification::topLeft, false);

    // Oscilloscope Grid Frame
    g.setColour(juce::Colour(0x1038e8d8));
    g.fillRoundedRectangle(rightWaveArea.reduced(2.0f), 4.0f);
    g.setColour(juce::Colour(0x2038e8d8));
    g.drawRoundedRectangle(rightWaveArea.reduced(2.0f), 4.0f, 1.0f);

    // Center line
    float cy = rightWaveArea.getCentreY();
    g.setColour(juce::Colour(0x18ffffff));
    g.drawHorizontalLine(static_cast<int>(cy), rightWaveArea.getX() + 4.0f, rightWaveArea.getRight() - 4.0f);

    // Draw Transient Waveform
    juce::Path wavePath;
    float startX = rightWaveArea.getX() + 6.0f;
    float waveWidth = rightWaveArea.getWidth() - 12.0f;
    float maxAmp = (rightWaveArea.getHeight() * 0.42f);

    wavePath.startNewSubPath(startX, cy);
    for (float x = 0; x < waveWidth; x += 1.5f)
    {
        float normX = x / waveWidth;
        float decayEnv = std::exp(-normX * (3.0f + (1.0f - animHardness) * 4.5f));
        float osc = std::sin(normX * 28.0f + animPhase * 2.5f);
        float wy = cy + osc * decayEnv * maxAmp * (0.3f + animDecay * 0.7f);
        wavePath.lineTo(startX + x, wy);
    }
    
    // Wave Glow & Line
    g.setColour(MarimbaLookAndFeel::getBrightAmber().withAlpha(0.25f));
    g.strokePath(wavePath, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    
    g.setColour(MarimbaLookAndFeel::getBrightAmber());
    g.strokePath(wavePath, juce::PathStrokeType(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

} // namespace ExtasisGUI
