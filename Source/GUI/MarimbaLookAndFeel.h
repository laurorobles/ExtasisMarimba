#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace ExtasisGUI
{

class MarimbaLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MarimbaLookAndFeel();
    ~MarimbaLookAndFeel() override = default;

    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont(juce::Label&) override;

    // Custom Color Palette
    static juce::Colour getBackgroundDark()    { return juce::Colour(0xff141518); }
    static juce::Colour getPanelBackground()    { return juce::Colour(0xff1d1f24); }
    static juce::Colour getPanelBorder()        { return juce::Colour(0xff2d313a); }
    static juce::Colour getAmberGold()          { return juce::Colour(0xffffa834); }
    static juce::Colour getBrightAmber()        { return juce::Colour(0xffffca68); }
    static juce::Colour getWoodWarmth()         { return juce::Colour(0xffb86b36); }
    static juce::Colour getLcdBackground()      { return juce::Colour(0xff0d131a); }
    static juce::Colour getLcdCyan()            { return juce::Colour(0xff38e8d8); }
    static juce::Colour getKnobFace()           { return juce::Colour(0xff23262d); }
    static juce::Colour getKnobRim()            { return juce::Colour(0xff393e48); }
};

} // namespace ExtasisGUI
