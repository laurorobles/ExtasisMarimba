#include "MarimbaLookAndFeel.h"

namespace ExtasisGUI
{

MarimbaLookAndFeel::MarimbaLookAndFeel()
{
    setColour(juce::Label::textColourId, juce::Colour(0xffd5dadf));
    setColour(juce::ComboBox::backgroundColourId, getPanelBackground());
    setColour(juce::ComboBox::textColourId, getAmberGold());
    setColour(juce::ComboBox::outlineColourId, getPanelBorder());
}

void MarimbaLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                         float sliderPosProportional, float rotaryStartAngle,
                                         float rotaryEndAngle, juce::Slider& /*slider*/)
{
    auto radius = (float) juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float) x + (float) width * 0.5f;
    auto centreY = (float) y + (float) height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // 1. Outer track (inactive)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(juce::Colour(0xff2a2e36));
    g.strokePath(backgroundArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Active arc value (Amber glow)
    if (sliderPosProportional > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, angle, true);
        
        // Glow effect
        g.setColour(getAmberGold().withAlpha(0.25f));
        g.strokePath(valueArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(getAmberGold());
        g.strokePath(valueArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Knob Body (Metallic Bevel)
    auto knobRadius = radius - 8.0f;
    auto kx = centreX - knobRadius;
    auto ky = centreY - knobRadius;
    auto kw = knobRadius * 2.0f;

    // Bevel Outer Rim
    g.setGradientFill(juce::ColourGradient(getKnobRim().brighter(0.2f), centreX, ky,
                                           getKnobRim().darker(0.4f), centreX, ky + kw, false));
    g.fillEllipse(kx, ky, kw, kw);

    // Inner Face
    auto faceRadius = knobRadius - 2.0f;
    g.setGradientFill(juce::ColourGradient(getKnobFace().brighter(0.15f), centreX, centreY - faceRadius,
                                           getKnobFace().darker(0.3f), centreX, centreY + faceRadius, false));
    g.fillEllipse(centreX - faceRadius, centreY - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f);

    // 4. Indicator Needle / Pointer
    juce::Path p;
    auto pointerLength = faceRadius * 0.75f;
    auto pointerThickness = 3.0f;
    p.addRoundedRectangle(-pointerThickness * 0.5f, -faceRadius + 2.0f, pointerThickness, pointerLength, 1.5f);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(getBrightAmber());
    g.fillPath(p);
}

void MarimbaLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                             const juce::Colour& backgroundColour,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    bool isTrigger = button.getButtonText().contains("TRIGGER");

    if (isTrigger)
    {
        juce::Colour bg = shouldDrawButtonAsDown ? getAmberGold().darker(0.2f) : (shouldDrawButtonAsHighlighted ? juce::Colour(0xff382e1c) : juce::Colour(0xff221b10));
        g.setColour(bg);
        g.fillRoundedRectangle(bounds, 5.0f);

        // Amber Glowing Border
        g.setColour(shouldDrawButtonAsDown ? getBrightAmber() : (shouldDrawButtonAsHighlighted ? getBrightAmber().withAlpha(0.9f) : getAmberGold().withAlpha(0.6f)));
        g.drawRoundedRectangle(bounds, 5.0f, shouldDrawButtonAsDown ? 2.0f : 1.2f);

        // Top Highlight Bevel
        g.setColour(juce::Colour(0x30ffffff));
        g.drawHorizontalLine(static_cast<int>(bounds.getY() + 2.0f), bounds.getX() + 4.0f, bounds.getRight() - 4.0f);
        return;
    }

    auto baseColor = backgroundColour.isOpaque() ? backgroundColour : getPanelBackground();

    if (shouldDrawButtonAsDown)
        baseColor = baseColor.darker(0.3f);
    else if (shouldDrawButtonAsHighlighted)
        baseColor = baseColor.brighter(0.2f);

    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds, 4.0f);

    g.setColour(shouldDrawButtonAsHighlighted ? getAmberGold().withAlpha(0.7f) : getPanelBorder());
    g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
}

void MarimbaLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                       bool /*shouldDrawButtonAsHighlighted*/,
                                       bool /*shouldDrawButtonAsDown*/)
{
    g.setFont(juce::Font(13.0f, juce::Font::bold));
    g.setColour(button.isEnabled() ? getAmberGold() : juce::Colours::grey);
    g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 1);
}

juce::Font MarimbaLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(12.5f, juce::Font::bold);
}

} // namespace ExtasisGUI
