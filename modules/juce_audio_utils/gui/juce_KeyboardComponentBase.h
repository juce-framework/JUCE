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
    A base class for drawing a custom MIDI keyboard component.

    Implement the drawKeyboardBackground(), drawWhiteKey(), and drawBlackKey() methods
    to draw your content and this class will handle the underlying keyboard logic.

    The component is a ChangeBroadcaster, so if you want to be informed when the
    keyboard is scrolled, you can register a ChangeListener for callbacks.

    @tags{Audio}
*/
class JUCE_API  KeyboardComponentBase  : public Component,
                                         public ChangeBroadcaster
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

    //==============================================================================
    /** Constructor.

        @param orientation  whether the keyboard is horizontal or vertical
    */
    explicit KeyboardComponentBase (Orientation orientation);

    /** Destructor. */
    ~KeyboardComponentBase() override = default;

    //==============================================================================
    /** Changes the width used to draw the white keys. */
    void setKeyWidth (float widthInPixels);

    /** Returns the width that was set by setKeyWidth(). */
    float getKeyWidth() const noexcept                              { return keyWidth; }

    /** Changes the width used to draw the buttons that scroll the keyboard up/down in octaves. */
    void setScrollButtonWidth (int widthInPixels);

    /** Returns the width that was set by setScrollButtonWidth(). */
    int getScrollButtonWidth() const noexcept                       { return scrollButtonWidth; }

    /** Changes the keyboard's current direction. */
    void setOrientation (Orientation newOrientation);

    /** Returns the keyboard's current direction. */
    Orientation getOrientation() const noexcept                     { return orientation; }

    /** Returns true if the keyboard's orientation is horizontal. */
    bool isHorizontal() const noexcept                              { return orientation == horizontalKeyboard; }

    /** Sets the range of midi notes that the keyboard will be limited to.

        By default the range is 0 to 127 (inclusive), but you can limit this if you
        only want a restricted set of the keys to be shown.

        Note that the values here are inclusive and must be between 0 and 127.
    */
    void setAvailableRange (int lowestNote, int highestNote);

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

    /** Returns the absolute length of the white notes.

        This will be their vertical or horizontal length, depending on the keyboard's orientation.
    */
    float getWhiteNoteLength() const noexcept;

    /** Sets the length of the black notes as a proportion of the white note length. */
    void setBlackNoteLengthProportion (float ratio) noexcept;

    /** Returns the length of the black notes as a proportion of the white note length. */
    float getBlackNoteLengthProportion() const noexcept             { return blackNoteLengthRatio; }

    /** Returns the absolute length of the black notes.

        This will be their vertical or horizontal length, depending on the keyboard's orientation.
    */
    float getBlackNoteLength() const noexcept;

    /** Sets the width of the black notes as a proportion of the white note width. */
    void setBlackNoteWidthProportion (float ratio) noexcept;

    /** Returns the width of the black notes as a proportion of the white note width. */
    float getBlackNoteWidthProportion() const noexcept             { return blackNoteWidthRatio; }

    /** Returns the absolute width of the black notes.

        This will be their vertical or horizontal width, depending on the keyboard's orientation.
    */
    float getBlackNoteWidth() const noexcept                       { return keyWidth * blackNoteWidthRatio; }

    /** If set to true, then scroll buttons will appear at either end of the keyboard
        if there are too many notes to fit them all in the component at once.
    */
    void setScrollButtonsVisible (bool canScroll);

    //==============================================================================
    /** Colour IDs to use to change the colour of the octave scroll buttons.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        upDownButtonBackgroundColourId  = 0x1004000,
        upDownButtonArrowColourId       = 0x1004001
    };

    /** Returns the position within the component of the left-hand edge of a key.

        Depending on the keyboard's orientation, this may be a horizontal or vertical
        distance, in either direction.
    */
    float getKeyStartPosition (int midiNoteNumber) const;

    /** Returns the total width needed to fit all the keys in the available range. */
    float getTotalKeyboardWidth() const noexcept;

    /** This structure is returned by the getNoteAndVelocityAtPosition() method.
    */
    struct JUCE_API  NoteAndVelocity
    {
        int note;
        float velocity;
    };

    /** Returns the note number and velocity for a given position within the component.

        If includeChildComponents is true then this will return a key obscured by any child
        components.
    */
    NoteAndVelocity getNoteAndVelocityAtPosition (Point<float> position, bool includeChildComponents = false);

   #ifndef DOXYGEN
    /** Returns the key at a given coordinate, or -1 if the position does not intersect a key. */
    [[deprecated ("This method has been deprecated in favour of getNoteAndVelocityAtPosition.")]]
    int getNoteAtPosition (Point<float> p)  { return getNoteAndVelocityAtPosition (p).note; }
   #endif

    /** Returns the rectangle for a given key. */
    Rectangle<float> getRectangleForKey (int midiNoteNumber) const;

    //==============================================================================
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
    /** Use this method to draw the background of the keyboard that will be drawn under
        the white and black notes. This can also be used to draw any shadow or outline effects.
    */
    virtual void drawKeyboardBackground (Graphics& g, Rectangle<float> area) = 0;

    /** Use this method to draw a white key of the keyboard in a given rectangle.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual void drawWhiteKey (int midiNoteNumber, Graphics& g, Rectangle<float> area) = 0;

    /** Use this method to draw a black key of the keyboard in a given rectangle.

        When doing this, be sure to note the keyboard's orientation.
    */
    virtual void drawBlackKey (int midiNoteNumber, Graphics& g, Rectangle<float> area) = 0;

    /** This can be overridden to draw the up and down buttons that scroll the keyboard
        up/down in octaves.
    */
    virtual void drawUpDownButton (Graphics& g, int w, int h, bool isMouseOver, bool isButtonPressed, bool movesOctavesUp);

    /** Calculates the position of a given midi-note.

        This can be overridden to create layouts with custom key-widths.

        @param midiNoteNumber   the note to find
        @param keyWidth         the desired width in pixels of one key - see setKeyWidth()
        @returns                the start and length of the key along the axis of the keyboard
    */
    virtual Range<float> getKeyPosition (int midiNoteNumber, float keyWidth) const;

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;

private:
    //==============================================================================
    struct UpDownButton;

    Range<float> getKeyPos (int midiNoteNumber) const;
    NoteAndVelocity remappedXYToNote (Point<float>) const;
    void setLowestVisibleKeyFloat (float noteNumber);

    //==============================================================================
    Orientation orientation;

    float blackNoteLengthRatio = 0.7f, blackNoteWidthRatio = 0.7f;
    float xOffset = 0.0f;
    float keyWidth = 16.0f;
    float firstKey = 12 * 4.0f;

    int scrollButtonWidth = 12;
    int rangeStart = 0, rangeEnd = 127;
    int octaveNumForMiddleC = 3;

    bool canScroll = true;
    std::unique_ptr<Button> scrollDown, scrollUp;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardComponentBase)
};

} // namespace juce
