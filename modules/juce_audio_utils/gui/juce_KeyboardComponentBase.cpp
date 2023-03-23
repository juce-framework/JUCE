/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

constexpr uint8 whiteNotes[] = { 0, 2, 4, 5, 7, 9, 11 };
constexpr uint8 blackNotes[] = { 1, 3, 6, 8, 10 };

//==============================================================================
struct KeyboardComponentBase::UpDownButton  : public Button
{
    UpDownButton (KeyboardComponentBase& c, int d)
        : Button ({}), owner (c), delta (d)
    {
    }

    void clicked() override
    {
        auto note = owner.getLowestVisibleKey();

        note = delta < 0 ? (note - 1) / 12 : note / 12 + 1;

        owner.setLowestVisibleKey (note * 12);
    }

    using Button::clicked;

    void paintButton (Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        owner.drawUpDownButton (g, getWidth(), getHeight(),
                                shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown,
                                delta > 0);
    }

private:
    KeyboardComponentBase& owner;
    int delta;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UpDownButton)
};

//==============================================================================
KeyboardComponentBase::KeyboardComponentBase (Orientation o)  : orientation (o)
{
    scrollDown = std::make_unique<UpDownButton> (*this, -1);
    scrollUp   = std::make_unique<UpDownButton> (*this, 1);

    addChildComponent (*scrollDown);
    addChildComponent (*scrollUp);

    colourChanged();
}

//==============================================================================
void KeyboardComponentBase::setKeyWidth (float widthInPixels)
{
    jassert (widthInPixels > 0);

    if (! approximatelyEqual (keyWidth, widthInPixels)) // Prevent infinite recursion if the width is being computed in a 'resized()' callback
    {
        keyWidth = widthInPixels;
        resized();
    }
}

void KeyboardComponentBase::setScrollButtonWidth (int widthInPixels)
{
    jassert (widthInPixels > 0);

    if (scrollButtonWidth != widthInPixels)
    {
        scrollButtonWidth = widthInPixels;
        resized();
    }
}

void KeyboardComponentBase::setOrientation (Orientation newOrientation)
{
    if (orientation != newOrientation)
    {
        orientation = newOrientation;
        resized();
    }
}

void KeyboardComponentBase::setAvailableRange (int lowestNote, int highestNote)
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

void KeyboardComponentBase::setLowestVisibleKey (int noteNumber)
{
    setLowestVisibleKeyFloat ((float) noteNumber);
}

void KeyboardComponentBase::setLowestVisibleKeyFloat (float noteNumber)
{
    noteNumber = jlimit ((float) rangeStart, (float) rangeEnd, noteNumber);

    if (! approximatelyEqual (noteNumber, firstKey))
    {
        bool hasMoved = (((int) firstKey) != (int) noteNumber);
        firstKey = noteNumber;

        if (hasMoved)
            sendChangeMessage();

        resized();
    }
}

float KeyboardComponentBase::getWhiteNoteLength() const noexcept
{
    return (orientation == horizontalKeyboard) ? (float) getHeight() : (float) getWidth();
}

void KeyboardComponentBase::setBlackNoteLengthProportion (float ratio) noexcept
{
    jassert (ratio >= 0.0f && ratio <= 1.0f);

    if (! approximatelyEqual (blackNoteLengthRatio, ratio))
    {
        blackNoteLengthRatio = ratio;
        resized();
    }
}

float KeyboardComponentBase::getBlackNoteLength() const noexcept
{
    auto whiteNoteLength = orientation == horizontalKeyboard ? getHeight() : getWidth();
    return (float) whiteNoteLength * blackNoteLengthRatio;
}

void KeyboardComponentBase::setBlackNoteWidthProportion (float ratio) noexcept
{
    jassert (ratio >= 0.0f && ratio <= 1.0f);

    if (! approximatelyEqual (blackNoteWidthRatio, ratio))
    {
        blackNoteWidthRatio = ratio;
        resized();
    }
}

void KeyboardComponentBase::setScrollButtonsVisible (bool newCanScroll)
{
    if (canScroll != newCanScroll)
    {
        canScroll = newCanScroll;
        resized();
    }
}

//==============================================================================
Range<float> KeyboardComponentBase::getKeyPos (int midiNoteNumber) const
{
    return getKeyPosition (midiNoteNumber, keyWidth)
             - xOffset
             - getKeyPosition (rangeStart, keyWidth).getStart();
}

float KeyboardComponentBase::getKeyStartPosition (int midiNoteNumber) const
{
    return getKeyPos (midiNoteNumber).getStart();
}

float KeyboardComponentBase::getTotalKeyboardWidth() const noexcept
{
    return getKeyPos (rangeEnd).getEnd();
}

KeyboardComponentBase::NoteAndVelocity KeyboardComponentBase::getNoteAndVelocityAtPosition (Point<float> pos, bool children)
{
    if (! reallyContains (pos, children))
        return { -1, 0.0f };

    auto p = pos;

    if (orientation != horizontalKeyboard)
    {
        p = { p.y, p.x };

        if (orientation == verticalKeyboardFacingLeft)
            p = { p.x, (float) getWidth() - p.y };
        else
            p = { (float) getHeight() - p.x, p.y };
    }

    return remappedXYToNote (p + Point<float> (xOffset, 0));
}

KeyboardComponentBase::NoteAndVelocity KeyboardComponentBase::remappedXYToNote (Point<float> pos) const
{
    auto blackNoteLength = getBlackNoteLength();

    if (pos.getY() < blackNoteLength)
    {
        for (int octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
        {
            for (int i = 0; i < 5; ++i)
            {
                auto note = octaveStart + blackNotes[i];

                if (rangeStart <= note && note <= rangeEnd)
                {
                    if (getKeyPos (note).contains (pos.x - xOffset))
                    {
                        return { note, jmax (0.0f, pos.y / blackNoteLength) };
                    }
                }
            }
        }
    }

    for (int octaveStart = 12 * (rangeStart / 12); octaveStart <= rangeEnd; octaveStart += 12)
    {
        for (int i = 0; i < 7; ++i)
        {
            auto note = octaveStart + whiteNotes[i];

            if (note >= rangeStart && note <= rangeEnd)
            {
                if (getKeyPos (note).contains (pos.x - xOffset))
                {
                    auto whiteNoteLength = (orientation == horizontalKeyboard) ? getHeight() : getWidth();
                    return { note, jmax (0.0f, pos.y / (float) whiteNoteLength) };
                }
            }
        }
    }

    return { -1, 0 };
}

Rectangle<float> KeyboardComponentBase::getRectangleForKey (int note) const
{
    jassert (note >= rangeStart && note <= rangeEnd);

    auto pos = getKeyPos (note);
    auto x = pos.getStart();
    auto w = pos.getLength();

    if (MidiMessage::isMidiNoteBlack (note))
    {
        auto blackNoteLength = getBlackNoteLength();

        switch (orientation)
        {
            case horizontalKeyboard:            return { x, 0, w, blackNoteLength };
            case verticalKeyboardFacingLeft:    return { (float) getWidth() - blackNoteLength, x, blackNoteLength, w };
            case verticalKeyboardFacingRight:   return { 0, (float) getHeight() - x - w, blackNoteLength, w };
            default:                            jassertfalse; break;
        }
    }
    else
    {
        switch (orientation)
        {
            case horizontalKeyboard:            return { x, 0, w, (float) getHeight() };
            case verticalKeyboardFacingLeft:    return { 0, x, (float) getWidth(), w };
            case verticalKeyboardFacingRight:   return { 0, (float) getHeight() - x - w, (float) getWidth(), w };
            default:                            jassertfalse; break;
        }
    }

    return {};
}

//==============================================================================
void KeyboardComponentBase::setOctaveForMiddleC (int octaveNum)
{
    octaveNumForMiddleC = octaveNum;
    repaint();
}

//==============================================================================
void KeyboardComponentBase::drawUpDownButton (Graphics& g, int w, int h, bool mouseOver, bool buttonDown, bool movesOctavesUp)
{
    g.fillAll (findColour (upDownButtonBackgroundColourId));

    float angle = 0;

    switch (getOrientation())
    {
        case horizontalKeyboard:            angle = movesOctavesUp ? 0.0f  : 0.5f;  break;
        case verticalKeyboardFacingLeft:    angle = movesOctavesUp ? 0.25f : 0.75f; break;
        case verticalKeyboardFacingRight:   angle = movesOctavesUp ? 0.75f : 0.25f; break;
        default:                            jassertfalse; break;
    }

    Path path;
    path.addTriangle (0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform (AffineTransform::rotation (MathConstants<float>::twoPi * angle, 0.5f, 0.5f));

    g.setColour (findColour (upDownButtonArrowColourId)
                  .withAlpha (buttonDown ? 1.0f : (mouseOver ? 0.6f : 0.4f)));

    g.fillPath (path, path.getTransformToScaleToFit (1.0f, 1.0f, (float) w - 2.0f, (float) h - 2.0f, true));
}

Range<float> KeyboardComponentBase::getKeyPosition (int midiNoteNumber, float targetKeyWidth) const
{
    auto ratio = getBlackNoteWidthProportion();

    static const float notePos[] = { 0.0f, 1 - ratio * 0.6f,
                                     1.0f, 2 - ratio * 0.4f,
                                     2.0f,
                                     3.0f, 4 - ratio * 0.7f,
                                     4.0f, 5 - ratio * 0.5f,
                                     5.0f, 6 - ratio * 0.3f,
                                     6.0f };

    auto octave = midiNoteNumber / 12;
    auto note   = midiNoteNumber % 12;

    auto start = (float) octave * 7.0f * targetKeyWidth + notePos[note] * targetKeyWidth;
    auto width = MidiMessage::isMidiNoteBlack (note) ? blackNoteWidthRatio * targetKeyWidth : targetKeyWidth;

    return { start, start + width };
}

//==============================================================================
void KeyboardComponentBase::paint (Graphics& g)
{
    drawKeyboardBackground (g, getLocalBounds().toFloat());

    for (int octaveBase = 0; octaveBase < 128; octaveBase += 12)
    {
        for (auto noteNum : whiteNotes)
        {
            const auto key = octaveBase + noteNum;

            if (rangeStart <= key && key <= rangeEnd)
                drawWhiteKey (key, g, getRectangleForKey (key));
        }

        for (auto noteNum : blackNotes)
        {
            const auto key = octaveBase + noteNum;

            if (rangeStart <= key && key <= rangeEnd)
                drawBlackKey (key, g, getRectangleForKey (key));
        }
    }
}

void KeyboardComponentBase::resized()
{
    auto w = getWidth();
    auto h = getHeight();

    if (w > 0 && h > 0)
    {
        if (orientation != horizontalKeyboard)
            std::swap (w, h);

        auto kx2 = getKeyPos (rangeEnd).getEnd();

        if ((int) firstKey != rangeStart)
        {
            auto kx1 = getKeyPos (rangeStart).getStart();

            if (kx2 - kx1 <= (float) w)
            {
                firstKey = (float) rangeStart;
                sendChangeMessage();
                repaint();
            }
        }

        scrollDown->setVisible (canScroll && firstKey > (float) rangeStart);

        xOffset = 0;

        if (canScroll)
        {
            auto scrollButtonW = jmin (scrollButtonWidth, w / 2);
            auto r = getLocalBounds();

            if (orientation == horizontalKeyboard)
            {
                scrollDown->setBounds (r.removeFromLeft  (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromRight (scrollButtonW));
            }
            else if (orientation == verticalKeyboardFacingLeft)
            {
                scrollDown->setBounds (r.removeFromTop    (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromBottom (scrollButtonW));
            }
            else
            {
                scrollDown->setBounds (r.removeFromBottom (scrollButtonW));
                scrollUp  ->setBounds (r.removeFromTop    (scrollButtonW));
            }

            auto endOfLastKey = getKeyPos (rangeEnd).getEnd();

            auto spaceAvailable = w;
            auto lastStartKey = remappedXYToNote ({ endOfLastKey - (float) spaceAvailable, 0 }).note + 1;

            if (lastStartKey >= 0 && ((int) firstKey) > lastStartKey)
            {
                firstKey = (float) jlimit (rangeStart, rangeEnd, lastStartKey);
                sendChangeMessage();
            }

            xOffset = getKeyPos ((int) firstKey).getStart();
        }
        else
        {
            firstKey = (float) rangeStart;
        }

        scrollUp->setVisible (canScroll && getKeyPos (rangeEnd).getStart() > (float) w);
        repaint();
    }
}

//==============================================================================
void KeyboardComponentBase::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    auto amount = (orientation == horizontalKeyboard && ! approximatelyEqual (wheel.deltaX, 0.0f))
                       ? wheel.deltaX : (orientation == verticalKeyboardFacingLeft ? wheel.deltaY
                                                                                   : -wheel.deltaY);

    setLowestVisibleKeyFloat (firstKey - amount * keyWidth);
}

} // namespace juce
