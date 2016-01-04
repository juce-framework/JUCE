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

//==============================================================================
class NoteComponent : public Component
{
public:
    NoteComponent (const ExpressiveMidiNote& n)  : note (n)
    {
    }

    //==========================================================================
    void update (const ExpressiveMidiNote& newNote, Point<float> newCentre)
    {
        note = newNote;
        centre = newCentre;

        setBounds (getSquareAroundCentre (jmax (getNoteOnRadius(), getPressureRadius()))
                     .getUnion (getTextRectangle())
                     .getSmallestIntegerContainer()
                     .expanded (3));

        repaint();
    }

    //==========================================================================
    void paint (Graphics& g) override
    {
        Colour colour (Colours::red); // TODO
        g.setColour (colour.withAlpha (0.3f));
        g.fillEllipse (translateToLocalBounds (getSquareAroundCentre (getNoteOnRadius())));

        g.setColour (colour); // TODO
        g.drawEllipse (translateToLocalBounds (getSquareAroundCentre (getPressureRadius())), 2.0f);

        Rectangle<int> textBounds = translateToLocalBounds (getTextRectangle()).getSmallestIntegerContainer();
        g.drawText ("+", textBounds, Justification::centred);
        g.drawText (MidiMessage::getMidiNoteName (note.initialNote, true, true, 3), textBounds, Justification::centredBottom);
        g.setFont (Font (22.0f, Font::bold));
        g.drawText (String (note.midiChannel), textBounds, Justification::centredTop);
    }

    //==========================================================================
    ExpressiveMidiNote note;
    Point<float> centre;

private:
    //==========================================================================
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

    float getNoteOnRadius() const       { return note.noteOnVelocity.asUnsignedFloat() * maxNoteRadius; }
    float getPressureRadius() const     { return note.pressure.asUnsignedFloat() * maxNoteRadius; }

    const float maxNoteRadius = 100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoteComponent)
};

//==============================================================================
class Visualiser : public Component,
                   public ExpressiveMidiInstrument::Listener,
                   private AsyncUpdater
{
public:
    //==========================================================================
    Visualiser() {}

    //==========================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        float noteDistance = float (getWidth()) / 128;
        for (int i = 0; i < 128; ++i)
        {
            float x = noteDistance * i;
            int noteHeight = MidiMessage::isMidiNoteBlack (i) ? 0.7 * getHeight() : getHeight();
            g.setColour (MidiMessage::isMidiNoteBlack (i) ? Colours::white : Colours::grey);
            g.drawLine (x, 0.0f, x, noteHeight);

            if (i > 0 && i % 12 == 0)
            {
                g.setColour (Colours::grey);
                int octaveNumber = (i / 12) - 2;
                g.drawText ("C" + String (octaveNumber), x - 15, getHeight() - 30, 30, 30, Justification::centredBottom); // TIMUR TODO: beautify this!
            }
        }
    }

    //==========================================================================
    void noteAdded (ExpressiveMidiNote newNote) override
    {
        const ScopedLock sl (lock);
        activeNotes.add (newNote);
        triggerAsyncUpdate();
    }

    void noteChanged (ExpressiveMidiNote changedNote) override
    {
        const ScopedLock sl (lock);

        for (auto& note : activeNotes)
            if (note.noteID == changedNote.noteID)
                note = changedNote;

        triggerAsyncUpdate();
    }

    void noteReleased (ExpressiveMidiNote finishedNote) override
    {
        const ScopedLock sl (lock);

        for (int i = activeNotes.size(); --i >= 0;)
            if (activeNotes.getReference(i).noteID == finishedNote.noteID)
                activeNotes.remove (i);

        triggerAsyncUpdate();
    }


private:
    //==========================================================================
    ExpressiveMidiNote* findActiveNote (int noteID) const noexcept
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

    //==========================================================================
    void handleAsyncUpdate() override
    {
        const ScopedLock sl (lock);

        for (int i = noteComponents.size(); --i >= 0;)
            if (findActiveNote (noteComponents.getUnchecked(i)->note.noteID) == nullptr)
                noteComponents.remove (i);

        for (auto& note : activeNotes)
            if (findNoteComponent (note.noteID) == nullptr)
                addAndMakeVisible (noteComponents.add (new NoteComponent (note)));

        for (auto& noteComp : noteComponents)
            if (auto* noteInfo = findActiveNote (noteComp->note.noteID))
                noteComp->update (*noteInfo, getCentrePositionForNote (*noteInfo));
    }

    //==========================================================================
    Point<float> getCentrePositionForNote (ExpressiveMidiNote note) const
    {
        float pitchbendRange = 24.0f; // TIMUR TODO: get actual range !!!

        float n = float (note.initialNote) + note.pitchbend.asPitchbendInSemitones (pitchbendRange);
        float x = getWidth() * n / 128;
        float y = getHeight() * (1 - note.timbre.asUnsignedFloat());

        return Point<float> (x, y);
    }

    //==========================================================================
    OwnedArray<NoteComponent> noteComponents;
    CriticalSection lock;
    Array<ExpressiveMidiNote> activeNotes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualiser)
};


#endif  // VISUALISER_H_INCLUDED
