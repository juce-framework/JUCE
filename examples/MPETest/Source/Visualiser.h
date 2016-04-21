/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


#ifndef VISUALISER_H_INCLUDED
#define VISUALISER_H_INCLUDED


class NoteComponent : public Component
{
public:
    NoteComponent (const MPENote& n, Colour colourToUse)
        : note (n), colour (colourToUse)
    {
    }

    //==============================================================================
    void update (const MPENote& newNote, Point<float> newCentre)
    {
        note = newNote;
        centre = newCentre;

        setBounds (getSquareAroundCentre (jmax (getNoteOnRadius(), getNoteOffRadius(), getPressureRadius()))
                     .getUnion (getTextRectangle())
                     .getSmallestIntegerContainer()
                     .expanded (3));

        repaint();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        if (note.keyState == MPENote::keyDown || note.keyState == MPENote::keyDownAndSustained)
            drawPressedNoteCircle (g, colour);
        else if (note.keyState == MPENote::sustained)
            drawSustainedNoteCircle (g, colour);
        else
            return;

        drawNoteLabel (g, colour);
    }

    //==============================================================================
    MPENote note;
    Colour colour;
    Point<float> centre;

private:
    //==============================================================================
    void drawPressedNoteCircle (Graphics& g, Colour zoneColour)
    {
        g.setColour (zoneColour.withAlpha (0.3f));
        g.fillEllipse (translateToLocalBounds (getSquareAroundCentre (getNoteOnRadius())));
        g.setColour (zoneColour);
        g.drawEllipse (translateToLocalBounds (getSquareAroundCentre (getPressureRadius())), 2.0f);
    }

    //==============================================================================
    void drawSustainedNoteCircle (Graphics& g, Colour zoneColour)
    {
        g.setColour (zoneColour);
        Path circle, dashedCircle;
        circle.addEllipse (translateToLocalBounds (getSquareAroundCentre (getNoteOffRadius())));
        const float dashLengths[] = { 3.0f, 3.0f };
        PathStrokeType (2.0, PathStrokeType::mitered).createDashedStroke (dashedCircle, circle, dashLengths, 2);
        g.fillPath (dashedCircle);
    }

    //==============================================================================
    void drawNoteLabel (Graphics& g, Colour zoneColour)
    {
        Rectangle<int> textBounds = translateToLocalBounds (getTextRectangle()).getSmallestIntegerContainer();
        g.drawText ("+", textBounds, Justification::centred);
        g.drawText (MidiMessage::getMidiNoteName (note.initialNote, true, true, 3), textBounds, Justification::centredBottom);
        g.setFont (Font (22.0f, Font::bold));
        g.drawText (String (note.midiChannel), textBounds, Justification::centredTop);
    }

    //==============================================================================
    Rectangle<float> getSquareAroundCentre (float radius) const noexcept
    {
        return Rectangle<float> (radius * 2.0f, radius * 2.0f).withCentre (centre);
    }

    Rectangle<float> translateToLocalBounds (Rectangle<float> r) const noexcept
    {
        return r - getPosition().toFloat();
    }

    Rectangle<float> getTextRectangle() const noexcept
    {
        return Rectangle<float> (30.0f, 50.0f).withCentre (centre);
    }

    float getNoteOnRadius() const noexcept     { return note.noteOnVelocity.asUnsignedFloat() * maxNoteRadius; }
    float getNoteOffRadius() const noexcept    { return note.noteOffVelocity.asUnsignedFloat() * maxNoteRadius; }
    float getPressureRadius() const noexcept   { return note.pressure.asUnsignedFloat() * maxNoteRadius; }

    const float maxNoteRadius = 100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteComponent)
};

//==============================================================================
class Visualiser : public Component,
                   public MPEInstrument::Listener,
                   private AsyncUpdater
{
public:
    //==============================================================================
    Visualiser (const ZoneColourPicker& zoneColourPicker)
        : colourPicker (zoneColourPicker)
    {}

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        float noteDistance = float (getWidth()) / 128;
        for (int i = 0; i < 128; ++i)
        {
            float x = noteDistance * i;
            int noteHeight = int (MidiMessage::isMidiNoteBlack (i) ? 0.7 * getHeight() : getHeight());
            g.setColour (MidiMessage::isMidiNoteBlack (i) ? Colours::white : Colours::grey);
            g.drawLine (x, 0.0f, x, (float) noteHeight);

            if (i > 0 && i % 12 == 0)
            {
                g.setColour (Colours::grey);
                int octaveNumber = (i / 12) - 2;
                g.drawText ("C" + String (octaveNumber), (int) x - 15, getHeight() - 30, 30, 30, Justification::centredBottom);
            }
        }
    }

    //==============================================================================
    void noteAdded (MPENote newNote) override
    {
        const ScopedLock sl (lock);
        activeNotes.add (newNote);
        triggerAsyncUpdate();
    }

    void notePressureChanged (MPENote note) override { noteChanged (note); }
    void notePitchbendChanged (MPENote note) override { noteChanged (note); }
    void noteTimbreChanged (MPENote note) override { noteChanged (note); }
    void noteKeyStateChanged (MPENote note) override { noteChanged (note); }

    void noteChanged (MPENote changedNote)
    {
        const ScopedLock sl (lock);

        for (auto& note : activeNotes)
            if (note.noteID == changedNote.noteID)
                note = changedNote;

        triggerAsyncUpdate();
    }

    void noteReleased (MPENote finishedNote) override
    {
        const ScopedLock sl (lock);

        for (int i = activeNotes.size(); --i >= 0;)
            if (activeNotes.getReference(i).noteID == finishedNote.noteID)
                activeNotes.remove (i);

        triggerAsyncUpdate();
    }


private:
    //==============================================================================
    MPENote* findActiveNote (int noteID) const noexcept
    {
        for (auto& note : activeNotes)
            if (note.noteID == noteID)
                return &note;

        return nullptr;
    }

    NoteComponent* findNoteComponent (int noteID) const noexcept
    {
        for (auto& noteComp : noteComponents)
            if (noteComp->note.noteID == noteID)
                return noteComp;

        return nullptr;
    }

    //==============================================================================
    void handleAsyncUpdate() override
    {
        const ScopedLock sl (lock);

        for (int i = noteComponents.size(); --i >= 0;)
            if (findActiveNote (noteComponents.getUnchecked(i)->note.noteID) == nullptr)
                noteComponents.remove (i);

        for (auto& note : activeNotes)
            if (findNoteComponent (note.noteID) == nullptr)
                addAndMakeVisible (noteComponents.add (new NoteComponent (note, colourPicker.getColourForMidiChannel(note.midiChannel))));

        for (auto& noteComp : noteComponents)
            if (auto* noteInfo = findActiveNote (noteComp->note.noteID))
                noteComp->update (*noteInfo, getCentrePositionForNote (*noteInfo));
    }

    //==============================================================================
    Point<float> getCentrePositionForNote (MPENote note) const
    {
        float n = float (note.initialNote) + float (note.totalPitchbendInSemitones);
        float x = getWidth() * n / 128;
        float y = getHeight() * (1 - note.timbre.asUnsignedFloat());

        return Point<float> (x, y);
    }

    //==============================================================================
    OwnedArray<NoteComponent> noteComponents;
    CriticalSection lock;
    Array<MPENote> activeNotes;
    const ZoneColourPicker& colourPicker;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualiser)
};


#endif  // VISUALISER_H_INCLUDED
