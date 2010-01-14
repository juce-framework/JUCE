/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MidiKeyboardComponent.h"


//==============================================================================
class MidiKeyboardUpDownButton  : public Button
{
public:
    MidiKeyboardUpDownButton (MidiKeyboardComponent* const owner_,
                              const int delta_)
        : Button (String::empty),
          owner (owner_),
          delta (delta_)
    {
        setOpaque (true);
    }

    ~MidiKeyboardUpDownButton()
    {
    }

    void clicked()
    {
        int note = owner->getLowestVisibleKey();

        if (delta < 0)
            note = (note - 1) / 12;
        else
            note = note / 12 + 1;

        owner->setLowestVisibleKey (note * 12);
    }

    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown)
    {
        owner->drawUpDownButton (g, getWidth(), getHeight(),
                                 isMouseOverButton, isButtonDown,
                                 delta > 0);
    }

private:
    MidiKeyboardComponent* const owner;
    const int delta;

    MidiKeyboardUpDownButton (const MidiKeyboardUpDownButton&);
    const MidiKeyboardUpDownButton& operator= (const MidiKeyboardUpDownButton&);
};

//==============================================================================
MidiKeyboardComponent::MidiKeyboardComponent (MidiKeyboardState& state_,
                                              const Orientation orientation_)
    : state (state_),
      xOffset (0),
      blackNoteLength (1),
      keyWidth (16.0f),
      orientation (orientation_),
      midiChannel (1),
      midiInChannelMask (0xffff),
      velocity (1.0f),
      noteUnderMouse (-1),
      mouseDownNote (-1),
      rangeStart (0),
      rangeEnd (127),
      firstKey (12 * 4),
      canScroll (true),
      mouseDragging (false),
      useMousePositionForVelocity (true),
      keyMappingOctave (6),
      octaveNumForMiddleC (3)
{
    addChildComponent (scrollDown = new MidiKeyboardUpDownButton (this, -1));
    addChildComponent (scrollUp   = new MidiKeyboardUpDownButton (this, 1));

    // initialise with a default set of querty key-mappings..
    const char* const keymap = "awsedftgyhujkolp;";

    for (int i = String (keymap).length(); --i >= 0;)
        setKeyPressForNote (KeyPress (keymap[i], 0, 0), i);

    setOpaque (true);
    setWantsKeyboardFocus (true);

    state.addListener (this);
}

MidiKeyboardComponent::~MidiKeyboardComponent()
{
    state.removeListener (this);
    jassert (mouseDownNote < 0 && keysPressed.countNumberOfSetBits() == 0); // leaving stuck notes!

    deleteAllChildren();
}

//==============================================================================
void MidiKeyboardComponent::setKeyWidth (const float widthInPixels)
{
    keyWidth = widthInPixels;
    resized();
}

void MidiKeyboardComponent::setOrientation (const Orientation newOrientation)
{
    if (orientation != newOrientation)
    {
        orientation = newOrientation;
        resized();
    }
}

void MidiKeyboardComponent::setAvailableRange (const int lowestNote,
                                               const int highestNote)
{
    jassert (lowestNote >= 0 && lowestNote <= 127);
    jassert (highestNote >= 0 && highestNote <= 127);
    jassert (lowestNote <= highestNote);

    if (rangeStart != lowestNote || rangeEnd != highestNote)
    {
        rangeStart = jlimit (0, 127, lowestNote);
        rangeEnd = jlimit (0, 127, highestNote);
        firstKey = jlimit (rangeStart, rangeEnd, firstKey);
        resized();
    }
}

void MidiKeyboardComponent::setLowestVisibleKey (int noteNumber)
{
    noteNumber = jlimit (rangeStart, rangeEnd, noteNumber);

    if (noteNumber != firstKey)
    {
        firstKey = noteNumber;
        sendChangeMessage (this);
        resized();
    }
}

void MidiKeyboardComponent::setScrollButtonsVisible (const bool canScroll_)
{
    if (canScroll != canScroll_)
    {
        canScroll = canScroll_;
        resized();
    }
}

void MidiKeyboardComponent::colourChanged()
{
    repaint();
}

//==============================================================================
void MidiKeyboardComponent::setMidiChannel (const int midiChannelNumber)
{
    jassert (midiChannelNumber > 0 && midiChannelNumber <= 16);

    if (midiChannel != midiChannelNumber)
    {
        resetAnyKeysInUse();
        midiChannel = jlimit (1, 16, midiChannelNumber);
    }
}

void MidiKeyboardComponent::setMidiChannelsToDisplay (const int midiChannelMask)
{
    midiInChannelMask = midiChannelMask;
    triggerAsyncUpdate();
}

void MidiKeyboardComponent::setVelocity (const float velocity_, const bool useMousePositionForVelocity_)
{
    velocity = jlimit (0.0f, 1.0f, velocity_);
    useMousePositionForVelocity = useMousePositionForVelocity_;
}

//==============================================================================
void MidiKeyboardComponent::getKeyPosition (int midiNoteNumber, const float keyWidth, int& x, int& w) const
{
    jassert (midiNoteNumber >= 0 && midiNoteNumber < 128);

    static const float blackNoteWidth = 0.7f;

    static const float notePos[] = { 0.0f, 1 - blackNoteWidth * 0.6f,
                                     1.0f, 2 - blackNoteWidth * 0.4f,
                                     2.0f, 3.0f, 4 - blackNoteWidth * 0.7f,
                                     4.0f, 5 - blackNoteWidth * 0.5f,
                                     5.0f, 6 - blackNoteWidth * 0.3f,
                                     6.0f };

    static const float widths[] = { 1.0f, blackNoteWidth,
                                    1.0f, blackNoteWidth,
                                    1.0f, 1.0f, blackNoteWidth,
                                    1.0f, blackNoteWidth,
                                    1.0f, blackNoteWidth,
                                    1.0f };

    const int octave = midiNoteNumber / 12;
    const int note = midiNoteNumber % 12;

    x = roundToInt (octave * 7.0f * keyWidth + notePos [note] * keyWidth);
    w = roundToInt (widths [note] * keyWidth);
}

void MidiKeyboardComponent::getKeyPos (int midiNoteNumber, int& x, int& w) const
{
    getKeyPosition (midiNoteNumber, keyWidth, x, w);

    int rx, rw;
    getKeyPosition (rangeStart, keyWidth, rx, rw);

    x -= xOffset + rx;
}

int MidiKeyboardComponent::getKeyStartPosition (const int midiNoteNumber) const
{
    int x, y;
    getKeyPos (midiNoteNumber, x, y);
    return x;
}

static const uint8 whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
static const uint8 blackNotes[] = { 1, 3, 6, 8, 10 };

int MidiKeyboardComponent::xyToNote (int x, int y, float& mousePositionVelocity)
{
    if (! reallyContains (x, y, false))
        return -1;

    if (orientation != horizontalKeyboard)
    {
        swapVariables (x, y);

        if (orientation == verticalKeyboardFacingLeft)
            y = getWidth() - y;
        else
            x = getHeight() - x;
    }

    return remappedXYToNote (x + xOffset, y, mousePositionVelocity);
}

int MidiKeyboardComponent::remappedXYToNote (int x, int y, float& mousePositionVelocity) const
{
    if (y < blackNoteLength)
    {
        for (int octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
        {
            for (int i = 0; i < 5; ++i)
            {
                const int note = octaveStart + blackNotes [i];

                if (note >= rangeStart && note <= rangeEnd)
                {
                    int kx, kw;
                    getKeyPos (note, kx, kw);
                    kx += xOffset;

                    if (x >= kx && x < kx + kw)
                    {
                        mousePositionVelocity = y / (float) blackNoteLength;
                        return note;
                    }
                }
            }
        }
    }

    for (int octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
    {
        for (int i = 0; i < 7; ++i)
        {
            const int note = octaveStart + whiteNotes [i];

            if (note >= rangeStart && note <= rangeEnd)
            {
                int kx, kw;
                getKeyPos (note, kx, kw);
                kx += xOffset;

                if (x >= kx && x < kx + kw)
                {
                    mousePositionVelocity = y / (float) getHeight();
                    return note;
                }
            }
        }
    }

    mousePositionVelocity = 0;
    return -1;
}

//==============================================================================
void MidiKeyboardComponent::repaintNote (const int noteNum)
{
    if (noteNum >= rangeStart && noteNum <= rangeEnd)
    {
        int x, w;
        getKeyPos (noteNum, x, w);

        if (orientation == horizontalKeyboard)
            repaint (x, 0, w, getHeight());
        else if (orientation == verticalKeyboardFacingLeft)
            repaint (0, x, getWidth(), w);
        else if (orientation == verticalKeyboardFacingRight)
            repaint (0, getHeight() - x - w, getWidth(), w);
    }
}

void MidiKeyboardComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white.overlaidWith (findColour (whiteNoteColourId)));

    const Colour lineColour (findColour (keySeparatorLineColourId));
    const Colour textColour (findColour (textLabelColourId));

    int x, w, octave;

    for (octave = 0; octave < 128; octave += 12)
    {
        for (int white = 0; white < 7; ++white)
        {
            const int noteNum = octave + whiteNotes [white];

            if (noteNum >= rangeStart && noteNum <= rangeEnd)
            {
                getKeyPos (noteNum, x, w);

                if (orientation == horizontalKeyboard)
                    drawWhiteNote (noteNum, g, x, 0, w, getHeight(),
                                   state.isNoteOnForChannels (midiInChannelMask, noteNum),
                                   noteUnderMouse == noteNum,
                                   lineColour, textColour);
                else if (orientation == verticalKeyboardFacingLeft)
                    drawWhiteNote (noteNum, g, 0, x, getWidth(), w,
                                   state.isNoteOnForChannels (midiInChannelMask, noteNum),
                                   noteUnderMouse == noteNum,
                                   lineColour, textColour);
                else if (orientation == verticalKeyboardFacingRight)
                    drawWhiteNote (noteNum, g, 0, getHeight() - x - w, getWidth(), w,
                                   state.isNoteOnForChannels (midiInChannelMask, noteNum),
                                   noteUnderMouse == noteNum,
                                   lineColour, textColour);
            }
        }
    }

    float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;

    if (orientation == verticalKeyboardFacingLeft)
    {
        x1 = getWidth() - 1.0f;
        x2 = getWidth() - 5.0f;
    }
    else if (orientation == verticalKeyboardFacingRight)
        x2 = 5.0f;
    else
        y2 = 5.0f;

    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.3f), x1, y1,
                                       Colours::transparentBlack, x2, y2, false));

    getKeyPos (rangeEnd, x, w);
    x += w;

    if (orientation == verticalKeyboardFacingLeft)
        g.fillRect (getWidth() - 5, 0, 5, x);
    else if (orientation == verticalKeyboardFacingRight)
        g.fillRect (0, 0, 5, x);
    else
        g.fillRect (0, 0, x, 5);

    g.setColour (lineColour);

    if (orientation == verticalKeyboardFacingLeft)
        g.fillRect (0, 0, 1, x);
    else if (orientation == verticalKeyboardFacingRight)
        g.fillRect (getWidth() - 1, 0, 1, x);
    else
        g.fillRect (0, getHeight() - 1, x, 1);

    const Colour blackNoteColour (findColour (blackNoteColourId));

    for (octave = 0; octave < 128; octave += 12)
    {
        for (int black = 0; black < 5; ++black)
        {
            const int noteNum = octave + blackNotes [black];

            if (noteNum >= rangeStart && noteNum <= rangeEnd)
            {
                getKeyPos (noteNum, x, w);

                if (orientation == horizontalKeyboard)
                    drawBlackNote (noteNum, g, x, 0, w, blackNoteLength,
                                   state.isNoteOnForChannels (midiInChannelMask, noteNum),
                                   noteUnderMouse == noteNum,
                                   blackNoteColour);
                else if (orientation == verticalKeyboardFacingLeft)
                    drawBlackNote (noteNum, g, getWidth() - blackNoteLength, x, blackNoteLength, w,
                                   state.isNoteOnForChannels (midiInChannelMask, noteNum),
                                   noteUnderMouse == noteNum,
                                   blackNoteColour);
                else if (orientation == verticalKeyboardFacingRight)
                    drawBlackNote (noteNum, g, 0, getHeight() - x - w, blackNoteLength, w,
                                   state.isNoteOnForChannels (midiInChannelMask, noteNum),
                                   noteUnderMouse == noteNum,
                                   blackNoteColour);
            }
        }
    }
}

void MidiKeyboardComponent::drawWhiteNote (int midiNoteNumber,
                                           Graphics& g, int x, int y, int w, int h,
                                           bool isDown, bool isOver,
                                           const Colour& lineColour,
                                           const Colour& textColour)
{
    Colour c (Colours::transparentWhite);

    if (isDown)
        c = findColour (keyDownOverlayColourId);

    if (isOver)
        c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));

    g.setColour (c);
    g.fillRect (x, y, w, h);

    const String text (getWhiteNoteText (midiNoteNumber));

    if (! text.isEmpty())
    {
        g.setColour (textColour);

        Font f (jmin (12.0f, keyWidth * 0.9f));
        f.setHorizontalScale (0.8f);
        g.setFont (f);
        Justification justification (Justification::centredBottom);

        if (orientation == verticalKeyboardFacingLeft)
            justification = Justification::centredLeft;
        else if (orientation == verticalKeyboardFacingRight)
            justification = Justification::centredRight;

        g.drawFittedText (text, x + 2, y + 2, w - 4, h - 4, justification, 1);
    }

    g.setColour (lineColour);

    if (orientation == horizontalKeyboard)
        g.fillRect (x, y, 1, h);
    else if (orientation == verticalKeyboardFacingLeft)
        g.fillRect (x, y, w, 1);
    else if (orientation == verticalKeyboardFacingRight)
        g.fillRect (x, y + h - 1, w, 1);

    if (midiNoteNumber == rangeEnd)
    {
        if (orientation == horizontalKeyboard)
            g.fillRect (x + w, y, 1, h);
        else if (orientation == verticalKeyboardFacingLeft)
            g.fillRect (x, y + h, w, 1);
        else if (orientation == verticalKeyboardFacingRight)
            g.fillRect (x, y - 1, w, 1);
    }
}

void MidiKeyboardComponent::drawBlackNote (int /*midiNoteNumber*/,
                                           Graphics& g, int x, int y, int w, int h,
                                           bool isDown, bool isOver,
                                           const Colour& noteFillColour)
{
    Colour c (noteFillColour);

    if (isDown)
        c = c.overlaidWith (findColour (keyDownOverlayColourId));

    if (isOver)
        c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));

    g.setColour (c);
    g.fillRect (x, y, w, h);

    if (isDown)
    {
        g.setColour (noteFillColour);
        g.drawRect (x, y, w, h);
    }
    else
    {
        const int xIndent = jmax (1, jmin (w, h) / 8);

        g.setColour (c.brighter());

        if (orientation == horizontalKeyboard)
            g.fillRect (x + xIndent, y, w - xIndent * 2, 7 * h / 8);
        else if (orientation == verticalKeyboardFacingLeft)
            g.fillRect (x + w / 8, y + xIndent, w - w / 8, h - xIndent * 2);
        else if (orientation == verticalKeyboardFacingRight)
            g.fillRect (x, y + xIndent, 7 * w / 8, h - xIndent * 2);
    }
}

void MidiKeyboardComponent::setOctaveForMiddleC (const int octaveNumForMiddleC_) throw()
{
    octaveNumForMiddleC = octaveNumForMiddleC_;
    repaint();
}

const String MidiKeyboardComponent::getWhiteNoteText (const int midiNoteNumber)
{
    if (keyWidth > 14.0f && midiNoteNumber % 12 == 0)
        return MidiMessage::getMidiNoteName (midiNoteNumber, true, true, octaveNumForMiddleC);

    return String::empty;
}

void MidiKeyboardComponent::drawUpDownButton (Graphics& g, int w, int h,
                                              const bool isMouseOver,
                                              const bool isButtonDown,
                                              const bool movesOctavesUp)
{
    g.fillAll (findColour (upDownButtonBackgroundColourId));

    float angle;

    if (orientation == MidiKeyboardComponent::horizontalKeyboard)
        angle = movesOctavesUp ? 0.0f : 0.5f;
    else if (orientation == MidiKeyboardComponent::verticalKeyboardFacingLeft)
        angle = movesOctavesUp ? 0.25f : 0.75f;
    else
        angle = movesOctavesUp ? 0.75f : 0.25f;

    Path path;
    path.lineTo (0.0f, 1.0f);
    path.lineTo (1.0f, 0.5f);
    path.closeSubPath();

    path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * angle, 0.5f, 0.5f));

    g.setColour (findColour (upDownButtonArrowColourId)
                  .withAlpha (isButtonDown ? 1.0f : (isMouseOver ? 0.6f : 0.4f)));

    g.fillPath (path, path.getTransformToScaleToFit (1.0f, 1.0f,
                                                     w - 2.0f,
                                                     h - 2.0f,
                                                     true));
}

void MidiKeyboardComponent::resized()
{
    int w = getWidth();
    int h = getHeight();

    if (w > 0 && h > 0)
    {
        if (orientation != horizontalKeyboard)
            swapVariables (w, h);

        blackNoteLength = roundToInt (h * 0.7f);

        int kx2, kw2;
        getKeyPos (rangeEnd, kx2, kw2);

        kx2 += kw2;

        if (firstKey != rangeStart)
        {
            int kx1, kw1;
            getKeyPos (rangeStart, kx1, kw1);

            if (kx2 - kx1 <= w)
            {
                firstKey = rangeStart;
                sendChangeMessage (this);
                repaint();
            }
        }

        const bool showScrollButtons = canScroll && (firstKey > rangeStart || kx2 > w + xOffset * 2);

        scrollDown->setVisible (showScrollButtons);
        scrollUp->setVisible (showScrollButtons);

        xOffset = 0;

        if (showScrollButtons)
        {
            const int scrollButtonW = jmin (12, w / 2);

            if (orientation == horizontalKeyboard)
            {
                scrollDown->setBounds (0, 0, scrollButtonW, getHeight());
                scrollUp->setBounds (getWidth() - scrollButtonW, 0, scrollButtonW, getHeight());
            }
            else if (orientation == verticalKeyboardFacingLeft)
            {
                scrollDown->setBounds (0, 0, getWidth(), scrollButtonW);
                scrollUp->setBounds (0, getHeight() - scrollButtonW, getWidth(), scrollButtonW);
            }
            else if (orientation == verticalKeyboardFacingRight)
            {
                scrollDown->setBounds (0, getHeight() - scrollButtonW, getWidth(), scrollButtonW);
                scrollUp->setBounds (0, 0, getWidth(), scrollButtonW);
            }

            int endOfLastKey, kw;
            getKeyPos (rangeEnd, endOfLastKey, kw);
            endOfLastKey += kw;

            float mousePositionVelocity;
            const int spaceAvailable = w - scrollButtonW * 2;
            const int lastStartKey = remappedXYToNote (endOfLastKey - spaceAvailable, 0, mousePositionVelocity) + 1;

            if (lastStartKey >= 0 && firstKey > lastStartKey)
            {
                firstKey = jlimit (rangeStart, rangeEnd, lastStartKey);
                sendChangeMessage (this);
            }

            int newOffset = 0;
            getKeyPos (firstKey, newOffset, kw);
            xOffset = newOffset - scrollButtonW;
        }
        else
        {
            firstKey = rangeStart;
        }

        timerCallback();
        repaint();
    }
}

//==============================================================================
void MidiKeyboardComponent::handleNoteOn (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/, float /*velocity*/)
{
    triggerAsyncUpdate();
}

void MidiKeyboardComponent::handleNoteOff (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/)
{
    triggerAsyncUpdate();
}

void MidiKeyboardComponent::handleAsyncUpdate()
{
    for (int i = rangeStart; i <= rangeEnd; ++i)
    {
        if (keysCurrentlyDrawnDown[i] != state.isNoteOnForChannels (midiInChannelMask, i))
        {
            keysCurrentlyDrawnDown.setBit (i, state.isNoteOnForChannels (midiInChannelMask, i));
            repaintNote (i);
        }
    }
}

//==============================================================================
void MidiKeyboardComponent::resetAnyKeysInUse()
{
    if (keysPressed.countNumberOfSetBits() > 0 || mouseDownNote > 0)
    {
        state.allNotesOff (midiChannel);
        keysPressed.clear();
        mouseDownNote = -1;
    }
}

void MidiKeyboardComponent::updateNoteUnderMouse (int x, int y)
{
    float mousePositionVelocity = 0.0f;
    const int newNote = (mouseDragging || isMouseOver())
                            ? xyToNote (x, y, mousePositionVelocity) : -1;

    if (noteUnderMouse != newNote)
    {
        if (mouseDownNote >= 0)
        {
            state.noteOff (midiChannel, mouseDownNote);
            mouseDownNote = -1;
        }

        if (mouseDragging && newNote >= 0)
        {
            if (! useMousePositionForVelocity)
                mousePositionVelocity = 1.0f;

            state.noteOn (midiChannel, newNote, mousePositionVelocity * velocity);
            mouseDownNote = newNote;
        }

        repaintNote (noteUnderMouse);
        noteUnderMouse = newNote;
        repaintNote (noteUnderMouse);
    }
    else if (mouseDownNote >= 0 && ! mouseDragging)
    {
        state.noteOff (midiChannel, mouseDownNote);
        mouseDownNote = -1;
    }
}

void MidiKeyboardComponent::mouseMove (const MouseEvent& e)
{
    updateNoteUnderMouse (e.x, e.y);
    stopTimer();
}

void MidiKeyboardComponent::mouseDrag (const MouseEvent& e)
{
    float mousePositionVelocity;
    const int newNote = xyToNote (e.x, e.y, mousePositionVelocity);

    if (newNote >= 0)
        mouseDraggedToKey (newNote, e);

    updateNoteUnderMouse (e.x, e.y);
}

bool MidiKeyboardComponent::mouseDownOnKey (int /*midiNoteNumber*/, const MouseEvent&)
{
    return true;
}

void MidiKeyboardComponent::mouseDraggedToKey (int /*midiNoteNumber*/, const MouseEvent&)
{
}

void MidiKeyboardComponent::mouseDown (const MouseEvent& e)
{
    float mousePositionVelocity;
    const int newNote = xyToNote (e.x, e.y, mousePositionVelocity);
    mouseDragging = false;

    if (newNote >= 0 && mouseDownOnKey (newNote, e))
    {
        repaintNote (noteUnderMouse);
        noteUnderMouse = -1;
        mouseDragging = true;

        updateNoteUnderMouse (e.x, e.y);
        startTimer (500);
    }
}

void MidiKeyboardComponent::mouseUp (const MouseEvent& e)
{
    mouseDragging = false;
    updateNoteUnderMouse (e.x, e.y);

    stopTimer();
}

void MidiKeyboardComponent::mouseEnter (const MouseEvent& e)
{
    updateNoteUnderMouse (e.x, e.y);
}

void MidiKeyboardComponent::mouseExit (const MouseEvent& e)
{
    updateNoteUnderMouse (e.x, e.y);
}

void MidiKeyboardComponent::mouseWheelMove (const MouseEvent&, float ix, float iy)
{
    setLowestVisibleKey (getLowestVisibleKey() + roundToInt ((ix != 0 ? ix : iy) * 5.0f));
}

void MidiKeyboardComponent::timerCallback()
{
    int mx, my;
    getMouseXYRelative (mx, my);

    updateNoteUnderMouse (mx, my);
}

//==============================================================================
void MidiKeyboardComponent::clearKeyMappings()
{
    resetAnyKeysInUse();
    keyPressNotes.clear();
    keyPresses.clear();
}

void MidiKeyboardComponent::setKeyPressForNote (const KeyPress& key,
                                                const int midiNoteOffsetFromC)
{
    removeKeyPressForNote (midiNoteOffsetFromC);

    keyPressNotes.add (midiNoteOffsetFromC);
    keyPresses.add (key);
}

void MidiKeyboardComponent::removeKeyPressForNote (const int midiNoteOffsetFromC)
{
    for (int i = keyPressNotes.size(); --i >= 0;)
    {
        if (keyPressNotes.getUnchecked (i) == midiNoteOffsetFromC)
        {
            keyPressNotes.remove (i);
            keyPresses.remove (i);
        }
    }
}

void MidiKeyboardComponent::setKeyPressBaseOctave (const int newOctaveNumber)
{
    jassert (newOctaveNumber >= 0 && newOctaveNumber <= 10);

    keyMappingOctave = newOctaveNumber;
}

bool MidiKeyboardComponent::keyStateChanged (const bool /*isKeyDown*/)
{
    bool keyPressUsed = false;

    for (int i = keyPresses.size(); --i >= 0;)
    {
        const int note = 12 * keyMappingOctave + keyPressNotes.getUnchecked (i);

        if (keyPresses.getReference(i).isCurrentlyDown())
        {
            if (! keysPressed [note])
            {
                keysPressed.setBit (note);
                state.noteOn (midiChannel, note, velocity);
                keyPressUsed = true;
            }
        }
        else
        {
            if (keysPressed [note])
            {
                keysPressed.clearBit (note);
                state.noteOff (midiChannel, note);
                keyPressUsed = true;
            }
        }
    }

    return keyPressUsed;
}

void MidiKeyboardComponent::focusLost (FocusChangeType)
{
    resetAnyKeysInUse();
}


END_JUCE_NAMESPACE
