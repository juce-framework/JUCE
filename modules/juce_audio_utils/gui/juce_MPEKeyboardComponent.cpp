/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

struct MPEKeyboardComponent::MPENoteComponent final : public Component
{
    MPENoteComponent (MPEKeyboardComponent& o, uint16 sID, uint8 initial, float noteOnVel, float press)
       : owner (o),
         radiusScale (owner.getKeyWidth() / 1.5f),
         noteOnVelocity (noteOnVel),
         pressure (press),
         sourceID (sID),
         initialNote (initial)
    {
    }

    float getStrikeRadius() const      { return 5.0f + getNoteOnVelocity() * radiusScale * 2.0f; }
    float getPressureRadius() const    { return 5.0f + getPressure() * radiusScale * 2.0f; }

    float getNoteOnVelocity() const    { return noteOnVelocity; }
    float getPressure() const          { return pressure; }

    Point<float> getCentrePos() const  { return getBounds().toFloat().getCentre(); }

    void paint (Graphics& g) override
    {
        auto strikeSize = getStrikeRadius() * 2.0f;
        auto pressSize = getPressureRadius() * 2.0f;
        auto bounds = getLocalBounds().toFloat();

        g.setColour (owner.findColour (noteCircleFillColourId));
        g.fillEllipse (bounds.withSizeKeepingCentre (strikeSize, strikeSize));

        g.setColour (owner.findColour (noteCircleOutlineColourId));
        g.drawEllipse (bounds.withSizeKeepingCentre (pressSize, pressSize), 1.0f);
    }

    //==============================================================================
    MPEKeyboardComponent& owner;

    float radiusScale = 0.0f, noteOnVelocity = 0.0f, pressure = 0.5f;
    uint16 sourceID = 0;
    uint8 initialNote = 0;
    bool isLatched = true;
};

//==============================================================================
MPEKeyboardComponent::MPEKeyboardComponent (MPEInstrument& instr, Orientation orientationToUse)
    : KeyboardComponentBase (orientationToUse),
      instrument (instr)
{
    updateZoneLayout();
    colourChanged();
    setKeyWidth (25.0f);

    instrument.addListener (this);
}

MPEKeyboardComponent::~MPEKeyboardComponent()
{
    instrument.removeListener (this);
}

//==============================================================================
void MPEKeyboardComponent::drawKeyboardBackground (Graphics& g, Rectangle<float> area)
{
    g.setColour (findColour (whiteNoteColourId));
    g.fillRect (area);
}

void MPEKeyboardComponent::drawWhiteKey (int midiNoteNumber, Graphics& g, Rectangle<float> area)
{
    if (midiNoteNumber % 12 == 0)
    {
        auto fontHeight = jmin (12.0f, getKeyWidth() * 0.9f);
        auto text = MidiMessage::getMidiNoteName (midiNoteNumber, true, true, getOctaveForMiddleC());

        g.setColour (findColour (textLabelColourId));
        g.setFont (withDefaultMetrics (FontOptions { fontHeight }).withHorizontalScale (0.8f));

        switch (getOrientation())
        {
            case horizontalKeyboard:
                g.drawText (text, area.withTrimmedLeft (1.0f).withTrimmedBottom (2.0f),
                            Justification::centredBottom, false);
                break;
            case verticalKeyboardFacingLeft:
                g.drawText (text, area.reduced (2.0f), Justification::centredLeft, false);
                break;
            case verticalKeyboardFacingRight:
                g.drawText (text, area.reduced (2.0f), Justification::centredRight, false);
                break;
            default:
                break;
        }
    }
}

void MPEKeyboardComponent::drawBlackKey (int /*midiNoteNumber*/, Graphics& g, Rectangle<float> area)
{
    g.setColour (findColour (whiteNoteColourId));
    g.fillRect (area);

    g.setColour (findColour (blackNoteColourId));

    if (isHorizontal())
    {
        g.fillRoundedRectangle (area.toFloat().reduced ((area.getWidth() / 2.0f) - (getBlackNoteWidth() / 12.0f),
                                                        area.getHeight() / 4.0f), 1.0f);
    }
    else
    {
        g.fillRoundedRectangle (area.toFloat().reduced (area.getWidth() / 4.0f,
                                                        (area.getHeight() / 2.0f) - (getBlackNoteWidth() / 12.0f)), 1.0f);
    }
}

void MPEKeyboardComponent::colourChanged()
{
    setOpaque (findColour (whiteNoteColourId).isOpaque());
    repaint();
}

//==============================================================================
MPEValue MPEKeyboardComponent::mousePositionToPitchbend (int initialNote, Point<float> mousePos)
{
    auto constrainedMousePos = [&]
    {
        auto horizontal = isHorizontal();

        auto posToCheck = jlimit (0.0f,
                                  horizontal ? (float) getWidth() - 1.0f : (float) getHeight(),
                                  horizontal ? mousePos.x : mousePos.y);

        auto bottomKeyRange = getRectangleForKey (jmax (getRangeStart(), initialNote - perNotePitchbendRange));
        auto topKeyRange    = getRectangleForKey (jmin (getRangeEnd(),   initialNote + perNotePitchbendRange));

        auto lowerLimit = horizontal ? bottomKeyRange.getCentreX()
                                     : getOrientation() == Orientation::verticalKeyboardFacingRight ? topKeyRange.getCentreY()
                                                                                                    : bottomKeyRange.getCentreY();

        auto upperLimit = horizontal ? topKeyRange.getCentreX()
                                     : getOrientation() == Orientation::verticalKeyboardFacingRight ? bottomKeyRange.getCentreY()
                                                                                                    : topKeyRange.getCentreY();

        posToCheck = jlimit (lowerLimit, upperLimit, posToCheck);

        return horizontal ? Point<float> (posToCheck, 0.0f)
                          : Point<float> (0.0f, posToCheck);
    }();

    auto note = getNoteAndVelocityAtPosition (constrainedMousePos, true).note;

    if (note == -1)
    {
        jassertfalse;
        return {};
    }

    auto fractionalSemitoneBend = [&]
    {
        auto noteRect = getRectangleForKey (note);

        switch (getOrientation())
        {
            case horizontalKeyboard:          return (constrainedMousePos.x - noteRect.getCentreX()) / noteRect.getWidth();
            case verticalKeyboardFacingRight: return (noteRect.getCentreY() - constrainedMousePos.y) / noteRect.getHeight();
            case verticalKeyboardFacingLeft:  return (constrainedMousePos.y - noteRect.getCentreY()) / noteRect.getHeight();
        }

        jassertfalse;
        return 0.0f;
    }();

    auto totalNumSemitones = ((float) note + fractionalSemitoneBend) - (float) initialNote;

    return MPEValue::fromUnsignedFloat (jmap (totalNumSemitones, (float) -perNotePitchbendRange, (float) perNotePitchbendRange, 0.0f, 1.0f));
}

MPEValue MPEKeyboardComponent::mousePositionToTimbre (Point<float> mousePos)
{
    auto delta = [mousePos, this]
    {
        switch (getOrientation())
        {
            case horizontalKeyboard:          return mousePos.y;
            case verticalKeyboardFacingLeft:  return (float) getWidth() - mousePos.x;
            case verticalKeyboardFacingRight: return mousePos.x;
        }

        jassertfalse;
        return 0.0f;
    }();

    return MPEValue::fromUnsignedFloat (jlimit (0.0f, 1.0f, 1.0f - (delta / getWhiteNoteLength())));
}

void MPEKeyboardComponent::mouseDown (const MouseEvent& e)
{
    auto newNote = getNoteAndVelocityAtPosition (e.position).note;

    if (newNote >= 0)
    {
        auto channel = channelAssigner->findMidiChannelForNewNote (newNote);

        instrument.noteOn (channel, newNote, MPEValue::fromUnsignedFloat (velocity));
        sourceIDMap[e.source.getIndex()] = instrument.getNote (instrument.getNumPlayingNotes() - 1).noteID;

        instrument.pitchbend (channel, MPEValue::centreValue());
        instrument.timbre    (channel, mousePositionToTimbre (e.position));
        instrument.pressure  (channel, MPEValue::fromUnsignedFloat (e.isPressureValid()
                                                                    && useMouseSourcePressureForStrike ? e.pressure
                                                                                                       : pressure));
    }
}

void MPEKeyboardComponent::mouseDrag (const MouseEvent& e)
{
    auto noteID = sourceIDMap[e.source.getIndex()];
    auto note = instrument.getNoteWithID (noteID);

    if (! note.isValid())
        return;

    auto noteComponent = std::find_if (noteComponents.begin(),
                                       noteComponents.end(),
                                       [noteID] (auto& comp) { return comp->sourceID == noteID; });

    if (noteComponent == noteComponents.end())
        return;

    if ((*noteComponent)->isLatched && std::abs (isHorizontal() ? e.getDistanceFromDragStartX()
                                                                : e.getDistanceFromDragStartY()) > roundToInt (getKeyWidth() / 4.0f))
    {
        (*noteComponent)->isLatched = false;
    }

    auto channel = channelAssigner->findMidiChannelForExistingNote (note.initialNote);

    if (! (*noteComponent)->isLatched)
        instrument.pitchbend (channel, mousePositionToPitchbend (note.initialNote, e.position));

    instrument.timbre (channel, mousePositionToTimbre (e.position));
    instrument.pressure (channel, MPEValue::fromUnsignedFloat (e.isPressureValid()
                                                               && useMouseSourcePressureForStrike ? e.pressure
                                                                                                  : pressure));
}

void MPEKeyboardComponent::mouseUp (const MouseEvent& e)
{
    auto note = instrument.getNoteWithID (sourceIDMap[e.source.getIndex()]);

    if (! note.isValid())
        return;

    instrument.noteOff (channelAssigner->findMidiChannelForExistingNote (note.initialNote),
                        note.initialNote, MPEValue::fromUnsignedFloat (lift));
    channelAssigner->noteOff (note.initialNote);
    sourceIDMap.erase (e.source.getIndex());
}

void MPEKeyboardComponent::focusLost (FocusChangeType)
{
    for (auto& comp : noteComponents)
    {
        auto note = instrument.getNoteWithID (comp->sourceID);

        if (note.isValid())
            instrument.noteOff (channelAssigner->findMidiChannelForExistingNote (note.initialNote),
                                note.initialNote, MPEValue::fromUnsignedFloat (lift));
    }
}

//==============================================================================
void MPEKeyboardComponent::updateZoneLayout()
{
    {
        const ScopedLock noteLock (activeNotesLock);
        activeNotes.clear();
    }

    noteComponents.clear();

    if (instrument.isLegacyModeEnabled())
    {
        channelAssigner = std::make_unique<MPEChannelAssigner> (instrument.getLegacyModeChannelRange());
        perNotePitchbendRange = instrument.getLegacyModePitchbendRange();
    }
    else
    {
        auto layout = instrument.getZoneLayout();

        if (layout.isActive())
        {
            auto zone = layout.getLowerZone().isActive() ? layout.getLowerZone()
                                                         : layout.getUpperZone();

            channelAssigner = std::make_unique<MPEChannelAssigner> (zone);
            perNotePitchbendRange = zone.perNotePitchbendRange;
        }
        else
        {
            channelAssigner.reset();
        }
    }
}

void MPEKeyboardComponent::addNewNote (MPENote note)
{
    noteComponents.push_back (std::make_unique<MPENoteComponent> (*this, note.noteID, note.initialNote,
                                                                  note.noteOnVelocity.asUnsignedFloat(),
                                                                  note.pressure.asUnsignedFloat()));
    auto& comp = noteComponents.back();

    addAndMakeVisible (*comp);
    comp->toBack();
}

void MPEKeyboardComponent::handleNoteOns (std::set<MPENote>& notesToUpdate)
{
    for (auto& note : notesToUpdate)
    {
        if (! std::any_of (noteComponents.begin(),
                           noteComponents.end(),
                           [note] (auto& comp) { return comp->sourceID == note.noteID; }))
        {
            addNewNote (note);
        }
    }
}

void MPEKeyboardComponent::handleNoteOffs (std::set<MPENote>& notesToUpdate)
{
    auto removePredicate = [&notesToUpdate] (std::unique_ptr<MPENoteComponent>& comp)
    {
        return std::none_of (notesToUpdate.begin(),
                             notesToUpdate.end(),
                             [&comp] (auto& note) { return comp->sourceID == note.noteID; });
    };

    noteComponents.erase (std::remove_if (std::begin (noteComponents),
                                          std::end (noteComponents),
                                          removePredicate),
                          std::end (noteComponents));

    if (noteComponents.empty())
        stopTimer();
}

void MPEKeyboardComponent::updateNoteComponentBounds (const MPENote& note, MPENoteComponent& noteComponent)
{
    auto xPos = [&]
    {
        const auto currentNote = note.initialNote + (float) note.totalPitchbendInSemitones;
        const auto noteBend = currentNote - std::floor (currentNote);

        const auto averageKeySize = (float) getTotalKeyboardWidth() / (float) (1 + getRangeEnd() - getRangeStart());
        const auto distance = noteBend * averageKeySize;

        const auto noteBounds = getRectangleForKey ((int) currentNote);
        const auto horizontal = isHorizontal();
        return (horizontal ? noteBounds.getCentreX() : noteBounds.getCentreY()) + distance;
    }();

    auto yPos = [&]
    {
        const auto currentOrientation = getOrientation();

        const auto timbrePosition = (currentOrientation == horizontalKeyboard
                                    || currentOrientation == verticalKeyboardFacingRight ? 1.0f - note.timbre.asUnsignedFloat()
                                                                                         : note.timbre.asUnsignedFloat());

        return timbrePosition * getWhiteNoteLength();
    }();

    const auto centrePos = (isHorizontal() ? Point<float> (xPos, yPos)
                                           : Point<float> (yPos, xPos));

    const auto radius = jmax (noteComponent.getStrikeRadius(), noteComponent.getPressureRadius());

    noteComponent.setBounds (Rectangle<float> (radius * 2.0f, radius * 2.0f)
                               .withCentre (centrePos)
                               .getSmallestIntegerContainer());
}

static bool operator< (const MPENote& n1, const MPENote& n2) noexcept  { return n1.noteID < n2.noteID; }

void MPEKeyboardComponent::updateNoteComponents()
{
    std::set<MPENote> notesToUpdate;

    {
        ScopedLock noteLock (activeNotesLock);

        for (const auto& note : activeNotes)
            if (note.second)
                notesToUpdate.insert (note.first);
    };

    handleNoteOns  (notesToUpdate);
    handleNoteOffs (notesToUpdate);

    for (auto& comp : noteComponents)
    {
        auto noteForComponent = std::find_if (notesToUpdate.begin(),
                                              notesToUpdate.end(),
                                              [&comp] (auto& note) { return note.noteID == comp->sourceID; });

        if (noteForComponent != notesToUpdate.end())
        {
            comp->pressure = noteForComponent->pressure.asUnsignedFloat();
            updateNoteComponentBounds (*noteForComponent, *comp);

            comp->repaint();
        }
    }
}

void MPEKeyboardComponent::timerCallback()
{
    updateNoteComponents();
}

//==============================================================================
void MPEKeyboardComponent::noteAdded (MPENote newNote)
{
    {
        const ScopedLock noteLock (activeNotesLock);
        activeNotes.push_back ({ newNote, true });
    }

    startTimerHz (30);
}

void MPEKeyboardComponent::updateNoteData (MPENote& changedNote)
{
    const ScopedLock noteLock (activeNotesLock);

    for (auto& note : activeNotes)
    {
        if (note.first.noteID == changedNote.noteID)
        {
            note.first = changedNote;
            note.second = true;
            return;
        }
    }
}

void MPEKeyboardComponent::notePressureChanged (MPENote changedNote)
{
    updateNoteData (changedNote);
}

void MPEKeyboardComponent::notePitchbendChanged (MPENote changedNote)
{
    updateNoteData (changedNote);
}

void MPEKeyboardComponent::noteTimbreChanged (MPENote changedNote)
{
    updateNoteData (changedNote);
}

void MPEKeyboardComponent::noteReleased (MPENote finishedNote)
{
    const ScopedLock noteLock (activeNotesLock);

    activeNotes.erase (std::remove_if (std::begin (activeNotes),
                                       std::end (activeNotes),
                                       [finishedNote] (auto& note) { return note.first.noteID == finishedNote.noteID; }),
                       std::end (activeNotes));
}

void MPEKeyboardComponent::zoneLayoutChanged()
{
    MessageManager::callAsync ([ref = SafePointer<MPEKeyboardComponent> { this }]
    {
        if (ref != nullptr)
            ref->updateZoneLayout();
    });
}

} // namespace juce
