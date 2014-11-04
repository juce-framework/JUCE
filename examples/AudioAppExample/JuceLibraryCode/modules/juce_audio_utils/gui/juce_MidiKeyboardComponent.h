/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_MIDIKEYBOARDCOMPONENT_H_INCLUDED
#define JUCE_MIDIKEYBOARDCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A component that displays a piano keyboard, whose notes can be clicked on.

    This component will mimic a physical midi keyboard, showing the current state of
    a MidiKeyboardState object. When the on-screen keys are clicked on, it will play these
    notes by calling the noteOn() and noteOff() methods of its MidiKeyboardState object.

    Another feature is that the computer keyboard can also be used to play notes. By
    default it maps the top two rows of a standard querty keyboard to the notes, but
    these can be remapped if needed. It will only respond to keypresses when it has
    the keyboard focus, so to disable this feature you can call setWantsKeyboardFocus (false).

    The component is also a ChangeBroadcaster, so if you want to be informed when the
    keyboard is scrolled, you can register a ChangeListener for callbacks.

    @see MidiKeyboardState
*/
class JUCE_API  MidiKeyboardComponent  : public Component,
                                         public MidiKeyboardStateListener,
                                         public ChangeBroadcaster,
                                         private Timer
{
public:
    //==============================================================================
    /** The direction of the keyboard.
        @see setOrientation
    */
    enum Orientation
    {
        horizontalKeyboard,
        verticalKeyboardFacingLeft,
        verticalKeyboardFacingRight,
    };

    /** Creates a MidiKeyboardComponent.

        @param state        the midi keyboard model that this component will represent
        @param orientation  whether the keyboard is horizonal or vertical
    */
    MidiKeyboardComponent (MidiKeyboardState& state,
                           Orientation orientation);

    /** Destructor. */
    ~MidiKeyboardComponent();

    //==============================================================================
    /** Changes the velocity used in midi note-on messages that are triggered by clicking
        on the component.

        Values are 0 to 1.0, where 1.0 is the heaviest.

        @see setMidiChannel
    */
    void setVelocity (float velocity, bool useMousePositionForVelocity);

    /** Changes the midi channel number that will be used for events triggered by clicking
        on the component.

        The channel must be between 1 and 16 (inclusive). This is the channel that will be
        passed on to the MidiKeyboardState::noteOn() method when the user clicks the component.

        Although this is the channel used for outgoing events, the component can display
        incoming events from more than one channel - see setMidiChannelsToDisplay()

        @see setVelocity
    */
    void setMidiChannel (int midiChannelNumber);

    /** Returns the midi channel that the keyboard is using for midi messages.
        @see setMidiChannel
    */
    int getMidiChannel() const noexcept                             { return midiChannel; }

    /** Sets a mask to indicate which incoming midi channels should be represented by
        key movements.

        The mask is a set of bits, where bit 0 = midi channel 1, bit 1 = midi channel 2, etc.

        If the MidiKeyboardState has a key down for any of the channels whose bits are set
        in this mask, the on-screen keys will also go down.

        By default, this mask is set to 0xffff (all channels displayed).

        @see setMidiChannel
    */
    void setMidiChannelsToDisplay (int midiChannelMask);

    /** Returns the current set of midi channels represented by the component.
        This is the value that was set with setMidiChannelsToDisplay().
    */
    int getMidiChannelsToDisplay() const noexcept                   { return midiInChannelMask; }

    //==============================================================================
    /** Changes the width used to draw the white keys. */
    void setKeyWidth (float widthInPixels);

    /** Returns the width that was set by setKeyWidth(). */
    float getKeyWidth() const noexcept                              { return keyWidth; }

    /** Changes the keyboard's current direction. */
    void setOrientation (Orientation newOrientation);

    /** Returns the keyboard's current direction. */
    Orientation getOrientation() const noexcept                     { return orientation; }

    /** Sets the range of midi notes that the keyboard will be limited to.

        By default the range is 0 to 127 (inclusive), but you can limit this if you
        only want a restricted set of the keys to be shown.

        Note that the values here are inclusive and must be between 0 and 127.
    */
    void setAvailableRange (int lowestNote,
                            int highestNote);

    /** Returns the first note in the available range.
        @see setAvailableRange
    */
    int getRangeStart() const noexcept                              { return rangeStart; }

    /** Returns the last note in the available range.
        @see setAvailableRange
    */
    int getRangeEnd() const noexcept                                { return rangeEnd; }

    /** If the keyboard extends beyond the size of the component, this will scroll
        it to show the given key at the start.

        Whenever the keyboard's position is changed, this will use the ChangeBroadcaster
        base class to send a callback to any ChangeListeners that have been registered.
    */
    void setLowestVisibleKey (int noteNumber);

    /** Returns the number of the first key shown in the component.
        @see setLowestVisibleKey
    */
    int getLowestVisibleKey() const noexcept                        { return (int) firstKey; }

    /** Returns the length of the black notes.
        This will be their vertical or horizontal length, depending on the keyboard's orientation.
    */
    int getBlackNoteLength() const noexcept                         { return blackNoteLength; }

    /** If set to true, then scroll buttons will appear at either end of the keyboard
        if there are too many notes to fit them all in the component at once.
    */
    void setScrollButtonsVisible (bool canScroll);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        whiteNoteColourId               = 0x1005000,
        blackNoteColourId               = 0x1005001,
        keySeparatorLineColourId        = 0x1005002,
        mouseOverKeyOverlayColourId     = 0x1005003,  /**< This colour will be overlaid on the normal note colour. */
        keyDownOverlayColourId          = 0x1005004,  /**< This colour will be overlaid on the normal note colour. */
        textLabelColourId               = 0x1005005,
        upDownButtonBackgroundColourId  = 0x1005006,
        upDownButtonArrowColourId       = 0x1005007,
        shadowColourId                  = 0x1005008
    };

    /** Returns the position within the component of the left-hand edge of a key.

        Depending on the keyboard's orientation, this may be a horizontal or vertical
        distance, in either direction.
    */
    int getKeyStartPosition (int midiNoteNumber) const;

    /** Returns the key at a given coordinate. */
    int getNoteAtPosition (Point<int> position);

    //==============================================================================
    /** Deletes all key-mappings.
        @see setKeyPressForNote
    */
    void clearKeyMappings();

    /** Maps a key-press to a given note.

        @param key                  the key that should trigger the note
        @param midiNoteOffsetFromC  how many semitones above C the triggered note should
                                    be. The actual midi note that gets played will be
                                    this value + (12 * the current base octave). To change
                                    the base octave, see setKeyPressBaseOctave()
    */
    void setKeyPressForNote (const KeyPress& key,
                             int midiNoteOffsetFromC);

    /** Removes any key-mappings for a given note.
        For a description of what the note number means, see setKeyPressForNote().
    */
    void removeKeyPressForNote (int midiNoteOffsetFromC);

    /** Changes the base note above which key-press-triggered notes are played.

        The set of key-mappings that trigger notes can be moved up and down to cover
        the entire scale using this method.

        The value passed in is an octave number between 0 and 10 (inclusive), and
        indicates which C is the base note to which the key-mapped notes are
        relative.
    */
    void setKeyPressBaseOctave (int newOctaveNumber);

    /** This sets the octave number which is shown as the octave number for middle C.

        This affects only the default implementation of getWhiteNoteText(), which
        passes this octave number to MidiMessage::getMidiNoteName() in order to
        get the note text. See MidiMessage::getMidiNoteName() for more info about
        the parameter.

        By default this value is set to 3.

        @see getOctaveForMiddleC
    */
    void setOctaveForMiddleC (int octaveNumForMiddleC);

    /** This returns the value set by setOctaveForMiddleC().
        @see setOctaveForMiddleC
    */
    int getOctaveForMiddleC() const noexcept            { return octaveNumForMiddleC; }

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void mouseMove (const MouseEvent&) override;
    /** @internal */
    void mouseDrag (const MouseEvent&) override;
    /** @internal */
    void mouseDown (const MouseEvent&) override;
    /** @internal */
    void mouseUp (const MouseEvent&) override;
    /** @internal */
    void mouseEnter (const MouseEvent&) override;
    /** @internal */
    void mouseExit (const MouseEvent&) override;
    /** @internal */
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    /** @internal */
    void timerCallback() override;
    /** @internal */
    bool keyStateChanged (bool isKeyDown) override;
    /** @internal */
    void focusLost (FocusChangeType) override;
    /** @internal */
    void handleNoteOn (MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    /** @internal */
    void handleNoteOff (MidiKeyboardState*, int midiChannel, int midiNoteNumber) override;
    /** @internal */
    void colourChanged() override;

protected:
    //==============================================================================
    /** Draws a white note in the given rectangle.

        isOver indicates whether the mouse is over the key, isDown indicates whether the key is
        currently pressed down.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual void drawWhiteNote (int midiNoteNumber,
                                Graphics& g,
                                int x, int y, int w, int h,
                                bool isDown, bool isOver,
                                const Colour& lineColour,
                                const Colour& textColour);

    /** Draws a black note in the given rectangle.

        isOver indicates whether the mouse is over the key, isDown indicates whether the key is
        currently pressed down.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual void drawBlackNote (int midiNoteNumber,
                                Graphics& g,
                                int x, int y, int w, int h,
                                bool isDown, bool isOver,
                                const Colour& noteFillColour);

    /** Allows text to be drawn on the white notes.
        By default this is used to label the C in each octave, but could be used for other things.
        @see setOctaveForMiddleC
    */
    virtual String getWhiteNoteText (const int midiNoteNumber);

    /** Draws the up and down buttons that change the base note. */
    virtual void drawUpDownButton (Graphics& g, int w, int h,
                                   const bool isMouseOver,
                                   const bool isButtonPressed,
                                   const bool movesOctavesUp);

    /** Callback when the mouse is clicked on a key.

        You could use this to do things like handle right-clicks on keys, etc.

        Return true if you want the click to trigger the note, or false if you
        want to handle it yourself and not have the note played.

        @see mouseDraggedToKey
    */
    virtual bool mouseDownOnKey (int midiNoteNumber, const MouseEvent& e);

    /** Callback when the mouse is dragged from one key onto another.
        @see mouseDownOnKey
    */
    virtual void mouseDraggedToKey (int midiNoteNumber, const MouseEvent& e);

    /** Callback when the mouse is released from a key.
        @see mouseDownOnKey
    */
    virtual void mouseUpOnKey (int midiNoteNumber, const MouseEvent& e);

    /** Calculates the positon of a given midi-note.

        This can be overridden to create layouts with custom key-widths.

        @param midiNoteNumber   the note to find
        @param keyWidth         the desired width in pixels of one key - see setKeyWidth()
        @param x                the x position of the left-hand edge of the key (this method
                                always works in terms of a horizontal keyboard)
        @param w                the width of the key
    */
    virtual void getKeyPosition (int midiNoteNumber, float keyWidth,
                                 int& x, int& w) const;

private:
    //==============================================================================
    friend class MidiKeyboardUpDownButton;

    MidiKeyboardState& state;
    int xOffset, blackNoteLength;
    float keyWidth;
    Orientation orientation;

    int midiChannel, midiInChannelMask;
    float velocity;

    Array<int> mouseOverNotes, mouseDownNotes;
    BigInteger keysPressed, keysCurrentlyDrawnDown;
    bool shouldCheckState;

    int rangeStart, rangeEnd;
    float firstKey;
    bool canScroll, useMousePositionForVelocity, shouldCheckMousePos;
    ScopedPointer<Button> scrollDown, scrollUp;

    Array<KeyPress> keyPresses;
    Array<int> keyPressNotes;
    int keyMappingOctave, octaveNumForMiddleC;

    static const uint8 whiteNotes[];
    static const uint8 blackNotes[];

    void getKeyPos (int midiNoteNumber, int& x, int& w) const;
    int xyToNote (Point<int>, float& mousePositionVelocity);
    int remappedXYToNote (Point<int>, float& mousePositionVelocity) const;
    void resetAnyKeysInUse();
    void updateNoteUnderMouse (Point<int>, bool isDown, int fingerNum);
    void updateNoteUnderMouse (const MouseEvent&, bool isDown);
    void repaintNote (const int midiNoteNumber);
    void setLowestVisibleKeyFloat (float noteNumber);
    Rectangle<int> getWhiteNotePos (int noteNumber) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MidiKeyboardComponent)
};


#endif   // JUCE_MIDIKEYBOARDCOMPONENT_H_INCLUDED
