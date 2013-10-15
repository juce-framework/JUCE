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

class MidiKeyboardUpDownButton  : public Button
{
public:
    MidiKeyboardUpDownButton (MidiKeyboardComponent& comp, const int d)
        : Button (String::empty),
          owner (comp),
          delta (d)
    {
        setOpaque (true);
    }

    void clicked() override
    {
        int note = owner.getLowestVisibleKey();

        if (delta < 0)
            note = (note - 1) / 12;
        else
            note = note / 12 + 1;

        owner.setLowestVisibleKey (note * 12);
    }

    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
    {
        owner.drawUpDownButton (g, getWidth(), getHeight(),
                                isMouseOverButton, isButtonDown,
                                delta > 0);
    }

private:
    MidiKeyboardComponent& owner;
    const int delta;

    JUCE_DECLARE_NON_COPYABLE (MidiKeyboardUpDownButton)
};

//==============================================================================
MidiKeyboardComponent::MidiKeyboardComponent (MidiKeyboardState& s,
                                              const Orientation o)
    : state (s),
      xOffset (0),
      blackNoteLength (1),
      keyWidth (16.0f),
      orientation (o),
      midiChannel (1),
      midiInChannelMask (0xffff),
      velocity (1.0f),
      shouldCheckState (false),
      rangeStart (0),
      rangeEnd (127),
      firstKey (12 * 4.0f),
      canScroll (true),
      useMousePositionForVelocity (true),
      shouldCheckMousePos (false),
      keyMappingOctave (6),
      octaveNumForMiddleC (3)
{
    addChildComponent (scrollDown = new MidiKeyboardUpDownButton (*this, -1));
    addChildComponent (scrollUp   = new MidiKeyboardUpDownButton (*this, 1));

    // initialise with a default set of querty key-mappings..
    const char* const keymap = "awsedftgyhujkolp;";

    for (int i = 0; keymap[i] != 0; ++i)
        setKeyPressForNote (KeyPress (keymap[i], 0, 0), i);

    mouseOverNotes.insertMultiple (0, -1, 32);
    mouseDownNotes.insertMultiple (0, -1, 32);

    setOpaque (true);
    setWantsKeyboardFocus (true);

    state.addListener (this);

    startTimer (1000 / 20);
}

MidiKeyboardComponent::~MidiKeyboardComponent()
{
    state.removeListener (this);
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
        firstKey = jlimit ((float) rangeStart, (float) rangeEnd, firstKey);
        resized();
    }
}

void MidiKeyboardComponent::setLowestVisibleKey (int noteNumber)
{
    setLowestVisibleKeyFloat ((float) noteNumber);
}

void MidiKeyboardComponent::setLowestVisibleKeyFloat (float noteNumber)
{
    noteNumber = jlimit ((float) rangeStart, (float) rangeEnd, noteNumber);

    if (noteNumber != firstKey)
    {
        const bool hasMoved = (((int) firstKey) != (int) noteNumber);
        firstKey = noteNumber;

        if (hasMoved)
        {
            sendChangeMessage();
            resized();
        }
    }
}

void MidiKeyboardComponent::setScrollButtonsVisible (const bool newCanScroll)
{
    if (canScroll != newCanScroll)
    {
        canScroll = newCanScroll;
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
    shouldCheckState = true;
}

void MidiKeyboardComponent::setVelocity (const float v, const bool useMousePosition)
{
    velocity = jlimit (0.0f, 1.0f, v);
    useMousePositionForVelocity = useMousePosition;
}

//==============================================================================
void MidiKeyboardComponent::getKeyPosition (int midiNoteNumber, const float keyWidth_, int& x, int& w) const
{
    jassert (midiNoteNumber >= 0 && midiNoteNumber < 128);

    static const float blackNoteWidth = 0.7f;

    static const float notePos[] = { 0.0f, 1 - blackNoteWidth * 0.6f,
                                     1.0f, 2 - blackNoteWidth * 0.4f,
                                     2.0f,
                                     3.0f, 4 - blackNoteWidth * 0.7f,
                                     4.0f, 5 - blackNoteWidth * 0.5f,
                                     5.0f, 6 - blackNoteWidth * 0.3f,
                                     6.0f };

    static const float widths[] = { 1.0f, blackNoteWidth,
                                    1.0f, blackNoteWidth,
                                    1.0f,
                                    1.0f, blackNoteWidth,
                                    1.0f, blackNoteWidth,
                                    1.0f, blackNoteWidth,
                                    1.0f };

    const int octave = midiNoteNumber / 12;
    const int note   = midiNoteNumber % 12;

    x = roundToInt (octave * 7.0f * keyWidth_ + notePos [note] * keyWidth_);
    w = roundToInt (widths [note] * keyWidth_);
}

void MidiKeyboardComponent::getKeyPos (int midiNoteNumber, int& x, int& w) const
{
    getKeyPosition (midiNoteNumber, keyWidth, x, w);

    int rx, rw;
    getKeyPosition (rangeStart, keyWidth, rx, rw);

    x -= xOffset + rx;
}

Rectangle<int> MidiKeyboardComponent::getWhiteNotePos (int noteNum) const
{
    int x, w;
    getKeyPos (noteNum, x, w);
    Rectangle<int> pos;

    switch (orientation)
    {
        case horizontalKeyboard:            pos.setBounds (x, 0, w, getHeight()); break;
        case verticalKeyboardFacingLeft:    pos.setBounds (0, x, getWidth(), w); break;
        case verticalKeyboardFacingRight:   pos.setBounds (0, getHeight() - x - w, getWidth(), w); break;
        default: break;
    }

    return pos;
}

int MidiKeyboardComponent::getKeyStartPosition (const int midiNoteNumber) const
{
    int x, w;
    getKeyPos (midiNoteNumber, x, w);
    return x;
}

const uint8 MidiKeyboardComponent::whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
const uint8 MidiKeyboardComponent::blackNotes[] = { 1, 3, 6, 8, 10 };

int MidiKeyboardComponent::xyToNote (Point<int> pos, float& mousePositionVelocity)
{
    if (! reallyContains (pos, false))
        return -1;

    Point<int> p (pos);

    if (orientation != horizontalKeyboard)
    {
        p = Point<int> (p.y, p.x);

        if (orientation == verticalKeyboardFacingLeft)
            p = Point<int> (p.x, getWidth() - p.y);
        else
            p = Point<int> (getHeight() - p.x, p.y);
    }

    return remappedXYToNote (p + Point<int> (xOffset, 0), mousePositionVelocity);
}

int MidiKeyboardComponent::remappedXYToNote (Point<int> pos, float& mousePositionVelocity) const
{
    if (pos.getY() < blackNoteLength)
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

                    if (pos.x >= kx && pos.x < kx + kw)
                    {
                        mousePositionVelocity = pos.y / (float) blackNoteLength;
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

                if (pos.x >= kx && pos.x < kx + kw)
                {
                    const int whiteNoteLength = (orientation == horizontalKeyboard) ? getHeight() : getWidth();
                    mousePositionVelocity = pos.y / (float) whiteNoteLength;
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
        repaint (getWhiteNotePos (noteNum));
}

void MidiKeyboardComponent::paint (Graphics& g)
{
    g.fillAll (Colours::white.overlaidWith (findColour (whiteNoteColourId)));

    const Colour lineColour (findColour (keySeparatorLineColourId));
    const Colour textColour (findColour (textLabelColourId));

    int octave;

    for (octave = 0; octave < 128; octave += 12)
    {
        for (int white = 0; white < 7; ++white)
        {
            const int noteNum = octave + whiteNotes [white];

            if (noteNum >= rangeStart && noteNum <= rangeEnd)
            {
                const Rectangle<int> pos (getWhiteNotePos (noteNum));

                drawWhiteNote (noteNum, g, pos.getX(), pos.getY(), pos.getWidth(), pos.getHeight(),
                               state.isNoteOnForChannels (midiInChannelMask, noteNum),
                               mouseOverNotes.contains (noteNum), lineColour, textColour);
            }
        }
    }

    float x1 = 0.0f, y1 = 0.0f, x2 = 0.0f, y2 = 0.0f;
    const int width = getWidth();
    const int height = getHeight();

    if (orientation == verticalKeyboardFacingLeft)
    {
        x1 = width - 1.0f;
        x2 = width - 5.0f;
    }
    else if (orientation == verticalKeyboardFacingRight)
        x2 = 5.0f;
    else
        y2 = 5.0f;

    g.setGradientFill (ColourGradient (Colours::black.withAlpha (0.3f), x1, y1,
                                       Colours::transparentBlack, x2, y2, false));

    int x, w;
    getKeyPos (rangeEnd, x, w);
    x += w;

    switch (orientation)
    {
        case horizontalKeyboard:            g.fillRect (0, 0, x, 5); break;
        case verticalKeyboardFacingLeft:    g.fillRect (width - 5, 0, 5, x); break;
        case verticalKeyboardFacingRight:   g.fillRect (0, 0, 5, x); break;
        default: break;
    }

    g.setColour (lineColour);

    switch (orientation)
    {
        case horizontalKeyboard:            g.fillRect (0, height - 1, x, 1); break;
        case verticalKeyboardFacingLeft:    g.fillRect (0, 0, 1, x); break;
        case verticalKeyboardFacingRight:   g.fillRect (width - 1, 0, 1, x); break;
        default: break;
    }

    const Colour blackNoteColour (findColour (blackNoteColourId));

    for (octave = 0; octave < 128; octave += 12)
    {
        for (int black = 0; black < 5; ++black)
        {
            const int noteNum = octave + blackNotes [black];

            if (noteNum >= rangeStart && noteNum <= rangeEnd)
            {
                getKeyPos (noteNum, x, w);
                Rectangle<int> pos;

                switch (orientation)
                {
                    case horizontalKeyboard:            pos.setBounds (x, 0, w, blackNoteLength); break;
                    case verticalKeyboardFacingLeft:    pos.setBounds (width - blackNoteLength, x, blackNoteLength, w); break;
                    case verticalKeyboardFacingRight:   pos.setBounds (0, height - x - w, blackNoteLength, w); break;
                    default: break;
                }

                drawBlackNote (noteNum, g, pos.getX(), pos.getY(), pos.getWidth(), pos.getHeight(),
                               state.isNoteOnForChannels (midiInChannelMask, noteNum),
                               mouseOverNotes.contains (noteNum), blackNoteColour);
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

    if (isDown)  c = findColour (keyDownOverlayColourId);
    if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));

    g.setColour (c);
    g.fillRect (x, y, w, h);

    const String text (getWhiteNoteText (midiNoteNumber));

    if (text.isNotEmpty())
    {
        g.setColour (textColour);
        g.setFont (Font (jmin (12.0f, keyWidth * 0.9f)).withHorizontalScale (0.8f));

        Justification justification (Justification::centredBottom);

        if (orientation == verticalKeyboardFacingLeft)
            justification = Justification::centredLeft;
        else if (orientation == verticalKeyboardFacingRight)
            justification = Justification::centredRight;

        g.drawFittedText (text, x + 2, y + 2, w - 4, h - 4, justification, 1);
    }

    g.setColour (lineColour);

    switch (orientation)
    {
        case horizontalKeyboard:            g.fillRect (x, y, 1, h); break;
        case verticalKeyboardFacingLeft:    g.fillRect (x, y, w, 1); break;
        case verticalKeyboardFacingRight:   g.fillRect (x, y + h - 1, w, 1); break;
        default: break;
    }

    if (midiNoteNumber == rangeEnd)
    {
        switch (orientation)
        {
            case horizontalKeyboard:            g.fillRect (x + w, y, 1, h); break;
            case verticalKeyboardFacingLeft:    g.fillRect (x, y + h, w, 1); break;
            case verticalKeyboardFacingRight:   g.fillRect (x, y - 1, w, 1); break;
            default: break;
        }
    }
}

void MidiKeyboardComponent::drawBlackNote (int /*midiNoteNumber*/,
                                           Graphics& g, int x, int y, int w, int h,
                                           bool isDown, bool isOver,
                                           const Colour& noteFillColour)
{
    Colour c (noteFillColour);

    if (isDown)  c = c.overlaidWith (findColour (keyDownOverlayColourId));
    if (isOver)  c = c.overlaidWith (findColour (mouseOverKeyOverlayColourId));

    g.setColour (c);
    g.fillRect (x, y, w, h);

    if (isDown)
    {
        g.setColour (noteFillColour);
        g.drawRect (x, y, w, h);
    }
    else
    {
        g.setColour (c.brighter());
        const int xIndent = jmax (1, jmin (w, h) / 8);

        switch (orientation)
        {
            case horizontalKeyboard:            g.fillRect (x + xIndent, y, w - xIndent * 2, 7 * h / 8); break;
            case verticalKeyboardFacingLeft:    g.fillRect (x + w / 8, y + xIndent, w - w / 8, h - xIndent * 2); break;
            case verticalKeyboardFacingRight:   g.fillRect (x, y + xIndent, 7 * w / 8, h - xIndent * 2); break;
            default: break;
        }
    }
}

void MidiKeyboardComponent::setOctaveForMiddleC (const int octaveNum)
{
    octaveNumForMiddleC = octaveNum;
    repaint();
}

String MidiKeyboardComponent::getWhiteNoteText (const int midiNoteNumber)
{
    if (keyWidth > 14.0f && midiNoteNumber % 12 == 0)
        return MidiMessage::getMidiNoteName (midiNoteNumber, true, true, octaveNumForMiddleC);

    return String::empty;
}

void MidiKeyboardComponent::drawUpDownButton (Graphics& g, int w, int h,
                                              const bool mouseOver,
                                              const bool buttonDown,
                                              const bool movesOctavesUp)
{
    g.fillAll (findColour (upDownButtonBackgroundColourId));

    float angle;

    switch (orientation)
    {
        case horizontalKeyboard:            angle = movesOctavesUp ? 0.0f  : 0.5f;  break;
        case verticalKeyboardFacingLeft:    angle = movesOctavesUp ? 0.25f : 0.75f; break;
        case verticalKeyboardFacingRight:   angle = movesOctavesUp ? 0.75f : 0.25f; break;
        default:                            jassertfalse; angle = 0; break;
    }

    Path path;
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (float_Pi * 2.0f * angle, 0.5f, 0.5f));

    g.setColour (findColour (upDownButtonArrowColourId)
                  .withAlpha (buttonDown ? 1.0f : (mouseOver ? 0.6f : 0.4f)));

    g.fillPath (path, path.getTransformToScaleToFit (1.0f, 1.0f, w - 2.0f, h - 2.0f, true));
}

void MidiKeyboardComponent::resized()
{
    int w = getWidth();
    int h = getHeight();

    if (w > 0 && h > 0)
    {
        if (orientation != horizontalKeyboard)
            std::swap (w, h);

        blackNoteLength = roundToInt (h * 0.7f);

        int kx2, kw2;
        getKeyPos (rangeEnd, kx2, kw2);

        kx2 += kw2;

        if ((int) firstKey != rangeStart)
        {
            int kx1, kw1;
            getKeyPos (rangeStart, kx1, kw1);

            if (kx2 - kx1 <= w)
            {
                firstKey = (float) rangeStart;
                sendChangeMessage();
                repaint();
            }
        }

        const bool showScrollButtons = canScroll && (((int) firstKey) > rangeStart || kx2 > w + xOffset * 2);

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
            else
            {
                scrollDown->setBounds (0, getHeight() - scrollButtonW, getWidth(), scrollButtonW);
                scrollUp->setBounds (0, 0, getWidth(), scrollButtonW);
            }

            int endOfLastKey, kw;
            getKeyPos (rangeEnd, endOfLastKey, kw);
            endOfLastKey += kw;

            float mousePositionVelocity;
            const int spaceAvailable = w - scrollButtonW * 2;
            const int lastStartKey = remappedXYToNote (Point<int> (endOfLastKey - spaceAvailable, 0), mousePositionVelocity) + 1;

            if (lastStartKey >= 0 && ((int) firstKey) > lastStartKey)
            {
                firstKey = (float) jlimit (rangeStart, rangeEnd, lastStartKey);
                sendChangeMessage();
            }

            int newOffset = 0;
            getKeyPos (((int) firstKey), newOffset, kw);
            xOffset = newOffset - scrollButtonW;
        }
        else
        {
            firstKey = (float) rangeStart;
        }

        repaint();
    }
}

//==============================================================================
void MidiKeyboardComponent::handleNoteOn (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/, float /*velocity*/)
{
    shouldCheckState = true; // (probably being called from the audio thread, so avoid blocking in here)
}

void MidiKeyboardComponent::handleNoteOff (MidiKeyboardState*, int /*midiChannel*/, int /*midiNoteNumber*/)
{
    shouldCheckState = true; // (probably being called from the audio thread, so avoid blocking in here)
}

//==============================================================================
void MidiKeyboardComponent::resetAnyKeysInUse()
{
    if (! keysPressed.isZero())
    {
        for (int i = 128; --i >= 0;)
            if (keysPressed[i])
                state.noteOff (midiChannel, i);

        keysPressed.clear();
    }

    for (int i = mouseDownNotes.size(); --i >= 0;)
    {
        const int noteDown = mouseDownNotes.getUnchecked(i);

        if (noteDown >= 0)
        {
            state.noteOff (midiChannel, noteDown);
            mouseDownNotes.set (i, -1);
        }

        mouseOverNotes.set (i, -1);
    }
}

void MidiKeyboardComponent::updateNoteUnderMouse (const MouseEvent& e, bool isDown)
{
    updateNoteUnderMouse (e.getPosition(), isDown, e.source.getIndex());
}

void MidiKeyboardComponent::updateNoteUnderMouse (Point<int> pos, bool isDown, int fingerNum)
{
    float mousePositionVelocity = 0.0f;
    const int newNote = xyToNote (pos, mousePositionVelocity);
    const int oldNote = mouseOverNotes.getUnchecked (fingerNum);

    if (oldNote != newNote)
    {
        repaintNote (oldNote);
        repaintNote (newNote);
        mouseOverNotes.set (fingerNum, newNote);
    }

    int oldNoteDown = mouseDownNotes.getUnchecked (fingerNum);

    if (isDown)
    {
        if (newNote != oldNoteDown)
        {
            if (oldNoteDown >= 0)
            {
                mouseDownNotes.set (fingerNum, -1);

                if (! mouseDownNotes.contains (oldNoteDown))
                    state.noteOff (midiChannel, oldNoteDown);
            }

            if (newNote >= 0)
            {
                if (! useMousePositionForVelocity)
                    mousePositionVelocity = 1.0f;

                state.noteOn (midiChannel, newNote, mousePositionVelocity * velocity);
                mouseDownNotes.set (fingerNum, newNote);
            }
        }
    }
    else if (oldNoteDown >= 0)
    {
        mouseDownNotes.set (fingerNum, -1);

        if (! mouseDownNotes.contains (oldNoteDown))
            state.noteOff (midiChannel, oldNoteDown);
    }
}

void MidiKeyboardComponent::mouseMove (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
    shouldCheckMousePos = false;
}

void MidiKeyboardComponent::mouseDrag (const MouseEvent& e)
{
    float mousePositionVelocity;
    const int newNote = xyToNote (e.getPosition(), mousePositionVelocity);

    if (newNote >= 0)
        mouseDraggedToKey (newNote, e);

    updateNoteUnderMouse (e, true);
}

bool MidiKeyboardComponent::mouseDownOnKey    (int, const MouseEvent&)  { return true; }
void MidiKeyboardComponent::mouseDraggedToKey (int, const MouseEvent&)  {}
void MidiKeyboardComponent::mouseUpOnKey      (int, const MouseEvent&)  {}

void MidiKeyboardComponent::mouseDown (const MouseEvent& e)
{
    float mousePositionVelocity;
    const int newNote = xyToNote (e.getPosition(), mousePositionVelocity);

    if (newNote >= 0 && mouseDownOnKey (newNote, e))
    {
        updateNoteUnderMouse (e, true);
        shouldCheckMousePos = true;
    }
}

void MidiKeyboardComponent::mouseUp (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
    shouldCheckMousePos = false;

    float mousePositionVelocity;
    const int note = xyToNote (e.getPosition(), mousePositionVelocity);
    if (note >= 0)
        mouseUpOnKey (note, e);
}

void MidiKeyboardComponent::mouseEnter (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void MidiKeyboardComponent::mouseExit (const MouseEvent& e)
{
    updateNoteUnderMouse (e, false);
}

void MidiKeyboardComponent::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    const float amount = (orientation == horizontalKeyboard && wheel.deltaX != 0)
                            ? wheel.deltaX : (orientation == verticalKeyboardFacingLeft ? wheel.deltaY
                                                                                        : -wheel.deltaY);

    setLowestVisibleKeyFloat (firstKey - amount * keyWidth);
}

void MidiKeyboardComponent::timerCallback()
{
    if (shouldCheckState)
    {
        shouldCheckState = false;

        for (int i = rangeStart; i <= rangeEnd; ++i)
        {
            if (keysCurrentlyDrawnDown[i] != state.isNoteOnForChannels (midiInChannelMask, i))
            {
                keysCurrentlyDrawnDown.setBit (i, state.isNoteOnForChannels (midiInChannelMask, i));
                repaintNote (i);
            }
        }
    }

    if (shouldCheckMousePos)
    {
        const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

        for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
            updateNoteUnderMouse (getLocalPoint (nullptr, mi->getScreenPosition()), mi->isDragging(), mi->getIndex());
    }
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
