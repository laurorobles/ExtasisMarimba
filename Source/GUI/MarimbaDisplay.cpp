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
    animPhase += 0.05f;
    if (animPhase > 6.283185f)
        animPhase -= 6.283185f;

    if (animDecay > 0.001f)
    {
        animDecay *= 0.92f;
        repaint();
    }
}

void MarimbaDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // 1. LCD Bezel & Background
    g.setColour(MarimbaLookAndFeel::getLcdBackground());
    g.fillRoundedRectangle(bounds, 6.0f);

    g.setColour(MarimbaLookAndFeel::getPanelBorder().brighter(0.1f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.5f);

    // Subtle LCD Scanlines / Grid
    g.setColour(juce::Colour(0x0838e8d8));
    for (float y = 4.0f; y < bounds.getHeight(); y += 4.0f)
    {
        g.drawHorizontalLine(static_cast<int>(y), 4.0f, bounds.getWidth() - 4.0f);
    }

    // 2. Header & Preset Name
    auto headerArea = bounds.removeFromTop(28.0f).reduced(8.0f, 4.0f);
    g.setFont(juce::Font(10.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff8899aa));
    g.drawText("MODAL PHYSICAL MODELING // MARIMBA", headerArea, juce::Justification::topRight, false);

    g.setFont(juce::Font(14.0f, juce::Font::bold));
    g.setColour(MarimbaLookAndFeel::getAmberGold());
    g.drawText(currentPreset, headerArea, juce::Justification::topLeft, false);

    // 3. Middle Area: Modal Spectrum & Harmonic Resonator Visualizer
    auto visualArea = bounds.removeFromTop(bounds.getHeight() - 24.0f).reduced(12.0f, 2.0f);
    
    // Draw 4 modal resonant bars representing f0, 4f0, 9.2f0, 16f0
    float barWidth = 14.0f;
    float spacing = (visualArea.getWidth() - (barWidth * 5.0f)) / 6.0f;
    
    float heights[5] = { 0.85f, 0.60f, 0.40f, 0.25f, 0.50f };
    const char* labels[5] = { "f0", "4f0", "9f0", "16f0", "PIPE" };

    for (int i = 0; i < 5; ++i)
    {
        float bx = visualArea.getX() + spacing + static_cast<float>(i) * (barWidth + spacing);
        float bh = visualArea.getHeight() * heights[i] * (0.4f + animDecay * 0.6f + std::sin(animPhase + i) * 0.05f);
        float by = visualArea.getBottom() - bh;

        juce::Rectangle<float> barRect(bx, by, barWidth, bh);

        // Bar Fill Gradient
        juce::Colour barCol = (i == 4) ? MarimbaLookAndFeel::getWoodWarmth() : MarimbaLookAndFeel::getLcdCyan();
        g.setGradientFill(juce::ColourGradient(barCol.brighter(0.3f), bx, by,
                                               barCol.darker(0.5f), bx, by + bh, false));
        g.fillRoundedRectangle(barRect, 2.0f);

        // Label
        g.setFont(juce::Font(9.0f, juce::Font::plain));
        g.setColour(juce::Colour(0xff8899aa));
        g.drawText(labels[i], bx - 4.0f, visualArea.getBottom() + 1.0f, barWidth + 8.0f, 10.0f, juce::Justification::centred, false);
    }

    // Draw Strike Transient Waveform on the right side
    auto waveArea = visualArea.removeFromRight(visualArea.getWidth() * 0.35f);
    juce::Path wavePath;
    wavePath.startNewSubPath(waveArea.getX(), waveArea.getCentreY());
    for (float x = 0; x < waveArea.getWidth(); x += 2.0f)
    {
        float normX = x / waveArea.getWidth();
        float decayEnv = std::exp(-normX * (3.0f + (1.0f - animHardness) * 4.0f));
        float osc = std::sin(normX * 25.0f + animPhase * 2.0f);
        float wy = waveArea.getCentreY() + osc * decayEnv * (waveArea.getHeight() * 0.45f) * (0.3f + animDecay * 0.7f);
        wavePath.lineTo(waveArea.getX() + x, wy);
    }
    g.setColour(MarimbaLookAndFeel::getBrightAmber().withAlpha(0.7f));
    g.strokePath(wavePath, juce::PathStrokeType(1.5f));

    // 4. Bottom Footer: Active Parameter Readout
    auto footerArea = bounds.reduced(8.0f, 2.0f);
    g.setFont(juce::Font(11.0f, juce::Font::plain));
    g.setColour(juce::Colour(0xff667788));
    g.drawText("STATUS: " + activeParam + "  " + activeValue, footerArea, juce::Justification::bottomLeft, false);
}

} // namespace ExtasisGUI
