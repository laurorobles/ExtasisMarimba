#include "MarimbaLookAndFeel.h"

namespace ExtasisGUI
{

MarimbaLookAndFeel::MarimbaLookAndFeel()
{
    setColour(juce::Label::textColourId, juce::Colour(0xffd5dadf));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff121417));
    setColour(juce::ComboBox::textColourId, getBrightAmber());
    setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff3a414d));
    setColour(juce::ComboBox::arrowColourId, getAmberGold());
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff16191e));
    setColour(juce::PopupMenu::textColourId, getBrightAmber());
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff2d2417));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void MarimbaLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = static_cast<float>(juce::jmin(width / 2, height / 2)) - 4.0f;
    auto centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    auto centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    bool isHot = slider.isMouseOverOrDragging();

    // 1. Outer track (inactive)
    juce::Path backgroundArc;
    backgroundArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour(isHot ? juce::Colour(0xff343a46) : juce::Colour(0xff262a32));
    g.strokePath(backgroundArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // 2. Active arc value (Amber glow)
    if (sliderPosProportional > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc(centreX, centreY, radius - 2.0f, radius - 2.0f, 0.0f, rotaryStartAngle, angle, true);

        // Glow effect
        g.setColour(getAmberGold().withAlpha(isHot ? 0.38f : 0.24f));
        g.strokePath(valueArc, juce::PathStrokeType(isHot ? 7.0f : 6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        g.setColour(isHot ? getBrightAmber() : getAmberGold());
        g.strokePath(valueArc, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // 3. Knob Body (Metallic Bevel)
    auto knobRadius = radius - 7.5f;
    auto kx = centreX - knobRadius;
    auto ky = centreY - knobRadius;
    auto kw = knobRadius * 2.0f;

    // Bevel Outer Rim
    auto rimCol = isHot ? getKnobRim().brighter(0.32f) : getKnobRim().brighter(0.18f);
    g.setGradientFill(juce::ColourGradient(rimCol, centreX, ky,
                                           getKnobRim().darker(0.45f), centreX, ky + kw, false));
    g.fillEllipse(kx, ky, kw, kw);

    // Inner Face
    auto faceRadius = knobRadius - 2.0f;
    g.setGradientFill(juce::ColourGradient(getKnobFace().brighter(isHot ? 0.18f : 0.10f), centreX, centreY - faceRadius,
                                           getKnobFace().darker(0.32f), centreX, centreY + faceRadius, false));
    g.fillEllipse(centreX - faceRadius, centreY - faceRadius, faceRadius * 2.0f, faceRadius * 2.0f);

    // 4. Indicator Needle
    juce::Path p;
    auto pointerLength = faceRadius * 0.72f;
    auto pointerThickness = 2.6f;
    p.addRoundedRectangle(-pointerThickness * 0.5f, -faceRadius + 1.5f, pointerThickness, pointerLength, 1.2f);
    p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));

    g.setColour(isHot ? juce::Colours::white : getBrightAmber());
    g.fillPath(p);
}

void MarimbaLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat().reduced(1.0f);

    // Dark Rounded Background
    g.setColour(juce::Colour(0xff121417));
    g.fillRoundedRectangle(bounds, 4.0f);

    // Glowing border when focused / hovered
    juce::Colour outlineCol = box.hasKeyboardFocus(true) ? getBrightAmber()
                            : (box.isMouseOver() ? getAmberGold() : juce::Colour(0xff3a414d));
    g.setColour(outlineCol);
    g.drawRoundedRectangle(bounds, 4.0f, 1.2f);

    // Chevron Arrow (v)
    juce::Path arrow;
    float ax = static_cast<float>(buttonX) + static_cast<float>(buttonW) * 0.35f;
    float ay = static_cast<float>(buttonY) + static_cast<float>(buttonH) * 0.5f - 2.0f;
    float aw = 8.0f;
    float ah = 4.5f;

    arrow.startNewSubPath(ax, ay);
    arrow.lineTo(ax + aw * 0.5f, ay + ah);
    arrow.lineTo(ax + aw, ay);

    g.setColour(isButtonDown ? getBrightAmber() : getAmberGold());
    g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::beveled, juce::PathStrokeType::rounded));
}

void MarimbaLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label)
{
    label.setBounds(7, 1, box.getWidth() - 26, box.getHeight() - 2);
    label.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 11.0f, juce::Font::bold));
}

void MarimbaLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                              const juce::Colour& backgroundColour,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    auto baseColor = backgroundColour;

    if (shouldDrawButtonAsDown)
        baseColor = baseColor.darker(0.25f);
    else if (shouldDrawButtonAsHighlighted)
        baseColor = baseColor.brighter(0.18f);

    g.setColour(baseColor);
    g.fillRoundedRectangle(bounds, 4.5f);

    juce::Colour borderCol = shouldDrawButtonAsHighlighted ? getAmberGold() : baseColor.brighter(0.28f);
    g.setColour(borderCol);
    g.drawRoundedRectangle(bounds, 4.5f, 1.0f);
}

void MarimbaLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                        bool /*shouldDrawButtonAsHighlighted*/,
                                        bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat();
    if (shouldDrawButtonAsDown)
        bounds.translate(0.0f, 1.0f);

    juce::Colour textCol = button.findColour(juce::TextButton::textColourOffId);
    if (!button.isEnabled())
        textCol = juce::Colours::grey;

    g.setFont(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));
    g.setColour(textCol);
    g.drawFittedText(button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
}

juce::Font MarimbaLookAndFeel::getLabelFont(juce::Label&)
{
    return juce::Font(juce::FontOptions(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));
}

} // namespace ExtasisGUI
