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

//==============================================================================
/**
    A component that displays an MPE-compatible keyboard, whose notes can be clicked on.

    This component will mimic a physical MPE-compatible keyboard, showing the current state
    of an MPEInstrument object. When the on-screen keys are clicked on, it will play these
    notes by calling the noteOn() and noteOff() methods of its MPEInstrument object. Moving
    the mouse will update the pitchbend and timbre dimensions of the MPEInstrument.

    @see MPEInstrument

    @tags{Audio}
*/
class JUCE_API  MPEKeyboardComponent  : public KeyboardComponentBase,
                                        private MPEInstrument::Listener,
                                        private Timer
{
public:
    //==============================================================================
    /** Creates an MPEKeyboardComponent.

        @param instrument   the MPEInstrument that this component represents
        @param orientation  whether the keyboard is horizontal or vertical
    */
    MPEKeyboardComponent (MPEInstrument& instrument, Orientation orientation);

    /** Destructor. */
    virtual ~MPEKeyboardComponent() override;

    //==============================================================================
    /** Sets the note-on velocity, or "strike", value that will be used when triggering new notes. */
    void setVelocity (float newVelocity)                                 { velocity = jlimit (0.0f, 1.0f, newVelocity); }

    /** Sets the pressure value that will be used for new notes. */
    void setPressure (float newPressure)                                 { pressure = jlimit (0.0f, 1.0f, newPressure); }

    /** Sets the note-off velocity, or "lift", value that will be used when notes are released. */
    void setLift (float newLift)                                         { lift = jlimit (0.0f, 1.0f, newLift); }

    /** Use this to enable the mouse source pressure to be used for the initial note-on
        velocity, or "strike", value if the mouse source supports it.
    */
    void setUseMouseSourcePressureForStrike (bool usePressureForStrike)  { useMouseSourcePressureForStrike = usePressureForStrike; }

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        whiteNoteColourId         = 0x1006000,
        blackNoteColourId         = 0x1006001,
        textLabelColourId         = 0x1006002,
        noteCircleFillColourId    = 0x1006003,
        noteCircleOutlineColourId = 0x1006004
    };

    //==============================================================================
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    void focusLost (FocusChangeType) override;
    /** @internal */
    void colourChanged() override;

private:
    //==============================================================================
    struct MPENoteComponent;

    //==============================================================================
    void drawKeyboardBackground (Graphics& g, Rectangle<float> area) override;
    void drawWhiteKey (int midiNoteNumber, Graphics& g, Rectangle<float> area) override;
    void drawBlackKey (int midiNoteNumber, Graphics& g, Rectangle<float> area) override;

    void updateNoteData (MPENote&);

    void noteAdded (MPENote) override;
    void notePressureChanged (MPENote) override;
    void notePitchbendChanged (MPENote) override;
    void noteTimbreChanged (MPENote) override;
    void noteReleased (MPENote) override;
    void zoneLayoutChanged() override;

    void timerCallback() override;

    //==============================================================================
    MPEValue mousePositionToPitchbend (int, Point<float>);
    MPEValue mousePositionToTimbre (Point<float>);

    void addNewNote (MPENote);
    void removeNote (MPENote);

    void handleNoteOns  (std::set<MPENote>&);
    void handleNoteOffs (std::set<MPENote>&);
    void updateNoteComponentBounds (const MPENote&, MPENoteComponent&);
    void updateNoteComponents();

    void updateZoneLayout();

    //==============================================================================
    MPEInstrument& instrument;
    std::unique_ptr<MPEChannelAssigner> channelAssigner;

    CriticalSection activeNotesLock;
    std::vector<std::pair<MPENote, bool>> activeNotes;
    std::vector<std::unique_ptr<MPENoteComponent>> noteComponents;
    std::map<int, uint16> sourceIDMap;

    float velocity = 0.7f, pressure = 1.0f, lift = 0.0f;
    bool useMouseSourcePressureForStrike = false;
    int perNotePitchbendRange = 48;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MPEKeyboardComponent)
};

} // namespace juce
