#include "MarimbaTriggerButton.h"
#include "MarimbaLookAndFeel.h"
#include "BinaryData.h"

namespace ExtasisGUI
{

MarimbaTriggerButton::MarimbaTriggerButton()
{
    setRepaintsOnMouseActivity(true);
    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    if (BinaryData::logo_pngSize > 0)
    {
        logoImage = juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize);
    }
}

juce::String MarimbaTriggerButton::getNoteNameString() const
{
    int currentNote = getCurrentMidiNote();
    int octave = (currentNote / 12) - 1;
    static const char* const noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    int idx = currentNote % 12;
    if (idx < 0) idx += 12;
    juce::String name = noteNames[idx];
    name += juce::String(octave);
    return name;
}

void MarimbaTriggerButton::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    // 1. Recessed Rack Panel Background with Gradient
    juce::Colour bgDark = isTriggerActive ? juce::Colour(0xff221708) : (isMouseOver() ? juce::Colour(0xff1e1b17) : juce::Colour(0xff121417));
    juce::Colour bgLight = isTriggerActive ? juce::Colour(0xff33220c) : (isMouseOver() ? juce::Colour(0xff2a2622) : juce::Colour(0xff1a1d22));

    juce::ColourGradient bgGrad(bgDark, bounds.getX(), bounds.getY(),
                                bgLight, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(bgGrad);
    g.fillRoundedRectangle(bounds, 6.0f);

    // Border Glow
    juce::Colour borderColor = isTriggerActive ? MarimbaLookAndFeel::getBrightAmber() 
                             : (isMouseOver() ? MarimbaLookAndFeel::getAmberGold() : juce::Colour(0xff2b313c));
    g.setColour(borderColor);
    g.drawRoundedRectangle(bounds, 6.0f, isTriggerActive ? 1.8f : 1.0f);

    // 2. Active / Hover Radial Glow behind Logo
    auto logoArea = bounds.reduced(8.0f);

    if (isTriggerActive)
    {
        g.setColour(MarimbaLookAndFeel::getAmberGold().withAlpha(0.25f));
        g.fillEllipse(logoArea.getCentreX() - 48.0f, logoArea.getCentreY() - 48.0f, 96.0f, 96.0f);
        g.setColour(MarimbaLookAndFeel::getBrightAmber().withAlpha(0.40f));
        g.fillEllipse(logoArea.getCentreX() - 28.0f, logoArea.getCentreY() - 28.0f, 56.0f, 56.0f);
    }
    else if (isMouseOver())
    {
        g.setColour(MarimbaLookAndFeel::getAmberGold().withAlpha(0.15f));
        g.fillEllipse(logoArea.getCentreX() - 40.0f, logoArea.getCentreY() - 40.0f, 80.0f, 80.0f);
    }

    // 3. Draw the High-Res Logo Image
    if (logoImage.isValid())
    {
        auto drawArea = logoArea;
        if (isTriggerActive)
            drawArea = drawArea.translated(0.0f, 1.0f); // subtle 3D tactile button press

        float opacity = isTriggerActive ? 1.0f : (isMouseOver() ? 0.96f : 0.85f);
        g.setOpacity(opacity);
        g.drawImage(logoImage, drawArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        g.setOpacity(1.0f);
    }

    // 4. Status LED Dot in bottom right corner
    float dotX = bounds.getRight() - 14.0f;
    float dotY = bounds.getBottom() - 14.0f;
    juce::Colour ledColor = isTriggerActive ? juce::Colour(0xffff2a2a) 
                          : (isMouseOver() ? juce::Colour(0xff992222) : juce::Colour(0xff441111));
    
    if (isTriggerActive)
    {
        g.setColour(juce::Colour(0x60ff2a2a));
        g.fillEllipse(dotX - 2.0f, dotY - 2.0f, 10.0f, 10.0f);
    }
    g.setColour(ledColor);
    g.fillEllipse(dotX, dotY, 6.0f, 6.0f);
}

void MarimbaTriggerButton::mouseDown(const juce::MouseEvent& e)
{
    isTriggerActive = true;
    dragStartY = (float)e.getScreenY();
    dragStartOffset = semitoneOffset;

    if (onNoteOn)
        onNoteOn(getCurrentMidiNote(), 1.0f);

    if (onStatusChange)
    {
        juce::String offsetStr = (semitoneOffset >= 0 ? "+" : "") + juce::String(semitoneOffset);
        onStatusChange("EXTASIS TRIGGER ACTIVE", "NOTE: " + getNoteNameString() + " (" + offsetStr + " st) // DRAG UP/DOWN TO PITCH");
    }

    repaint();
}

void MarimbaTriggerButton::mouseDrag(const juce::MouseEvent& e)
{
    if (!isTriggerActive)
        return;

    float deltaY = dragStartY - (float)e.getScreenY();
    int newOffset = juce::jlimit(-24, 24, dragStartOffset + (int)(deltaY / 7.0f));

    if (newOffset != semitoneOffset)
    {
        int oldNote = getCurrentMidiNote();
        semitoneOffset = newOffset;
        int newNote = getCurrentMidiNote();

        if (onNoteOff)
            onNoteOff(oldNote);
        if (onNoteOn)
            onNoteOn(newNote, 1.0f);

        if (onStatusChange)
        {
            juce::String offsetStr = (semitoneOffset >= 0 ? "+" : "") + juce::String(semitoneOffset);
            onStatusChange("TRIGGER PITCH SHIFT", "NOTE: " + getNoteNameString() + " (" + offsetStr + " st) [MIDI " + juce::String(newNote) + "]");
        }

        repaint();
    }
}

void MarimbaTriggerButton::mouseUp(const juce::MouseEvent&)
{
    if (isTriggerActive)
    {
        isTriggerActive = false;
        if (onNoteOff)
            onNoteOff(getCurrentMidiNote());

        if (onStatusChange)
        {
            juce::String offsetStr = (semitoneOffset >= 0 ? "+" : "") + juce::String(semitoneOffset);
            onStatusChange("TRIGGER RELEASED", "NOTE: " + getNoteNameString() + " (" + offsetStr + " st) READY");
        }

        repaint();
    }
}

void MarimbaTriggerButton::mouseEnter(const juce::MouseEvent&)
{
    if (onStatusChange)
    {
        juce::String offsetStr = (semitoneOffset >= 0 ? "+" : "") + juce::String(semitoneOffset);
        onStatusChange("EXTASIS TRIGGER [" + getNoteNameString() + " / " + offsetStr + " st]", "CLICK & HOLD TO AUDITION // DRAG UP/DOWN TO PITCH (+/-24 st)");
    }
}

void MarimbaTriggerButton::mouseExit(const juce::MouseEvent&)
{
    if (!isTriggerActive && onStatusChange)
    {
        onStatusChange("MODAL PHYSICAL SYNTH", "MARIMBA ENGINE READY");
    }
}

} // namespace ExtasisGUI
