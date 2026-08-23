#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

namespace ExtasisGUI
{

class LogoTriggerComponent : public juce::Component
{
public:
    std::function<void(int midiNote, float velocity)> onNoteOn;
    std::function<void(int midiNote)> onNoteOff;
    std::function<void(int newNote)> onNoteChanged;

    LogoTriggerComponent()
    {
        setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
    }

    void setImage(const juce::Image& img)
    {
        logoImg = img;
        repaint();
    }

    void setNote(int note)
    {
        currentNote = juce::jlimit(24, 96, note);
        repaint();
    }

    int getNote() const { return currentNote; }

    void setVelocity(float vel)
    {
        currentVelocity = juce::jlimit(0.1f, 1.0f, vel);
    }

    float getVelocity() const { return currentVelocity; }

    static juce::String getNoteName(int midiNote)
    {
        const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
        int octave = (midiNote / 12) - 1;
        int noteInOctave = midiNote % 12;
        if (noteInOctave < 0) noteInOctave += 12;
        return juce::String(noteNames[noteInOctave]) + juce::String(octave);
    }

    void mouseEnter(const juce::MouseEvent&) override
    {
        isHovered = true;
        repaint();
    }

    void mouseExit(const juce::MouseEvent&) override
    {
        isHovered = false;
        repaint();
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        initialNote = currentNote;
        isMouseDown = true;
        
        if (onNoteOn)
            onNoteOn(currentNote, currentVelocity);
            
        repaint();
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        int deltaY = e.getMouseDownPosition().y - e.getPosition().y;
        // Each 7 pixels of vertical drag changes 1 semitone
        int semitoneDelta = deltaY / 7;
        int newNote = juce::jlimit(24, 96, initialNote + semitoneDelta);

        if (newNote != currentNote)
        {
            if (onNoteOff)
                onNoteOff(currentNote);

            currentNote = newNote;

            if (onNoteOn)
                onNoteOn(currentNote, currentVelocity);

            if (onNoteChanged)
                onNoteChanged(currentNote);

            repaint();
        }
    }

    void mouseUp(const juce::MouseEvent&) override
    {
        isMouseDown = false;
        
        if (onNoteOff)
            onNoteOff(currentNote);
            
        repaint();
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        
        // Background medallion
        auto badgeArea = bounds.removeFromTop(bounds.getHeight() - 22.0f);
        auto centre = badgeArea.getCentre();
        float radius = juce::jmin(badgeArea.getWidth(), badgeArea.getHeight()) * 0.5f - 4.0f;

        juce::Colour bgBase = isMouseDown ? juce::Colour(0xff3d2e14) : (isHovered ? juce::Colour(0xff2a2214) : juce::Colour(0xff181614));
        g.setColour(bgBase);
        g.fillEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f);

        // Concentric metallic ring & glow
        juce::Colour gold = juce::Colour(0xffffa834);
        if (isMouseDown || isHovered)
        {
            g.setColour(gold.withAlpha(isMouseDown ? 0.35f : 0.18f));
            g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, 6.0f);
        }

        g.setColour(isMouseDown ? juce::Colour(0xffffc860) : (isHovered ? gold : gold.withAlpha(0.7f)));
        g.drawEllipse(centre.x - radius, centre.y - radius, radius * 2.0f, radius * 2.0f, isMouseDown ? 2.2f : 1.4f);

        // Inner dark inset
        float innerR = radius - 5.0f;
        g.setColour(juce::Colour(0xff0d0f11));
        g.fillEllipse(centre.x - innerR, centre.y - innerR, innerR * 2.0f, innerR * 2.0f);

        // Draw Logo Image
        if (logoImg.isValid())
        {
            auto logoRect = juce::Rectangle<float>(centre.x - innerR * 0.75f, centre.y - innerR * 0.75f,
                                                   innerR * 1.5f, innerR * 1.5f);
            g.drawImage(logoImg, logoRect, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize);
        }
        else
        {
            // Fallback stylized icon
            g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 13.0f, juce::Font::bold));
            g.setColour(gold);
            g.drawText("EXTASIS", badgeArea.toNearestInt(), juce::Justification::centred, false);
        }

        // Live Note Readout Pill below the logo
        auto pillArea = bounds.reduced(6.0f, 1.0f);
        g.setColour(juce::Colour(0xff121417));
        g.fillRoundedRectangle(pillArea, 4.0f);
        g.setColour(isMouseDown ? gold : juce::Colour(0xff333842));
        g.drawRoundedRectangle(pillArea, 4.0f, 1.0f);

        g.setFont(juce::Font(juce::Font::getDefaultMonospacedFontName(), 10.5f, juce::Font::bold));
        g.setColour(isMouseDown ? juce::Colour(0xffffc860) : gold);
        g.drawText("▲ " + getNoteName(currentNote) + " (" + juce::String(currentNote) + ") ▼", pillArea.toNearestInt(), juce::Justification::centred, false);
    }

private:
    juce::Image logoImg;
    int currentNote = 60; // C4
    int initialNote = 60;
    float currentVelocity = 1.0f;
    bool isMouseDown = false;
    bool isHovered = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LogoTriggerComponent)
};

} // namespace ExtasisGUI
