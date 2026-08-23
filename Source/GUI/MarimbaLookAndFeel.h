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

    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox& box) override;

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override;

    juce::Font getLabelFont(juce::Label&) override;

    // Custom Color Palette
    static juce::Colour getBackgroundDark()    { return juce::Colour(0xff121417); }
    static juce::Colour getPanelBackground()    { return juce::Colour(0xff1a1d22); }
    static juce::Colour getPanelBorder()        { return juce::Colour(0xff2d333d); }
    static juce::Colour getAmberGold()          { return juce::Colour(0xffffa834); }
    static juce::Colour getBrightAmber()        { return juce::Colour(0xffffca68); }
    static juce::Colour getWoodWarmth()         { return juce::Colour(0xffb86b36); }
    static juce::Colour getLcdBackground()      { return juce::Colour(0xff0d1512); }
    static juce::Colour getLcdCyan()            { return juce::Colour(0xff38e8d8); }
    static juce::Colour getKnobFace()           { return juce::Colour(0xff232730); }
    static juce::Colour getKnobRim()            { return juce::Colour(0xff3a414d); }
};

} // namespace ExtasisGUI
