/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

class ScrollBar::ScrollbarButton  : public Button
{
public:
    ScrollbarButton (int direc, ScrollBar& s)
        : Button (String()), direction (direc), owner (s)
    {
        setWantsKeyboardFocus (false);
    }

    void paintButton (Graphics& g, bool over, bool down) override
    {
        getLookAndFeel().drawScrollbarButton (g, owner, getWidth(), getHeight(),
                                              direction, owner.isVertical(), over, down);
    }

    void clicked() override
    {
        owner.moveScrollbarInSteps ((direction == 1 || direction == 2) ? 1 : -1);
    }

    using Button::clicked;

    int direction;

private:
    ScrollBar& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScrollbarButton)
};


//==============================================================================
ScrollBar::ScrollBar (bool shouldBeVertical)  : vertical (shouldBeVertical)
{
    setRepaintsOnMouseActivity (true);
    setFocusContainerType (FocusContainerType::keyboardFocusContainer);
}

ScrollBar::~ScrollBar()
{
    upButton.reset();
    downButton.reset();
}

//==============================================================================
void ScrollBar::setRangeLimits (Range<double> newRangeLimit, NotificationType notification)
{
    if (totalRange != newRangeLimit)
    {
        totalRange = newRangeLimit;
        setCurrentRange (visibleRange, notification);
        updateThumbPosition();
    }
}

void ScrollBar::setRangeLimits (double newMinimum, double newMaximum, NotificationType notification)
{
    jassert (newMaximum >= newMinimum); // these can't be the wrong way round!
    setRangeLimits (Range<double> (newMinimum, newMaximum), notification);
}

bool ScrollBar::setCurrentRange (Range<double> newRange, NotificationType notification)
{
    auto constrainedRange = totalRange.constrainRange (newRange);

    if (visibleRange != constrainedRange)
    {
        visibleRange = constrainedRange;

        updateThumbPosition();

        if (notification != dontSendNotification)
            triggerAsyncUpdate();

        if (notification == sendNotificationSync)
            handleUpdateNowIfNeeded();

        return true;
    }

    return false;
}

void ScrollBar::setCurrentRange (double newStart, double newSize, NotificationType notification)
{
    setCurrentRange (Range<double> (newStart, newStart + newSize), notification);
}

void ScrollBar::setCurrentRangeStart (double newStart, NotificationType notification)
{
    setCurrentRange (visibleRange.movedToStartAt (newStart), notification);
}

void ScrollBar::setSingleStepSize (double newSingleStepSize) noexcept
{
    singleStepSize = newSingleStepSize;
}

bool ScrollBar::moveScrollbarInSteps (int howManySteps, NotificationType notification)
{
    return setCurrentRange (visibleRange + howManySteps * singleStepSize, notification);
}

bool ScrollBar::moveScrollbarInPages (int howManyPages, NotificationType notification)
{
    return setCurrentRange (visibleRange + howManyPages * visibleRange.getLength(), notification);
}

bool ScrollBar::scrollToTop (NotificationType notification)
{
    return setCurrentRange (visibleRange.movedToStartAt (getMinimumRangeLimit()), notification);
}

bool ScrollBar::scrollToBottom (NotificationType notification)
{
    return setCurrentRange (visibleRange.movedToEndAt (getMaximumRangeLimit()), notification);
}

void ScrollBar::setButtonRepeatSpeed (int newInitialDelay,
                                      int newRepeatDelay,
                                      int newMinimumDelay)
{
    initialDelayInMillisecs = newInitialDelay;
    repeatDelayInMillisecs  = newRepeatDelay;
    minimumDelayInMillisecs = newMinimumDelay;

    if (upButton != nullptr)
    {
        upButton  ->setRepeatSpeed (newInitialDelay, newRepeatDelay, newMinimumDelay);
        downButton->setRepeatSpeed (newInitialDelay, newRepeatDelay, newMinimumDelay);
    }
}

//==============================================================================
void ScrollBar::addListener (Listener* listener)
{
    listeners.add (listener);
}

void ScrollBar::removeListener (Listener* listener)
{
    listeners.remove (listener);
}

void ScrollBar::handleAsyncUpdate()
{
    auto start = visibleRange.getStart(); // (need to use a temp variable for VC7 compatibility)
    listeners.call ([this, start] (Listener& l) { l.scrollBarMoved (this, start); });
}

//==============================================================================
void ScrollBar::updateThumbPosition()
{
    auto minimumScrollBarThumbSize = getLookAndFeel().getMinimumScrollbarThumbSize (*this);

    int newThumbSize = roundToInt (totalRange.getLength() > 0 ? (visibleRange.getLength() * thumbAreaSize) / totalRange.getLength()
                                                              : thumbAreaSize);

    if (newThumbSize < minimumScrollBarThumbSize)
        newThumbSize = jmin (minimumScrollBarThumbSize, thumbAreaSize - 1);

    if (newThumbSize > thumbAreaSize)
        newThumbSize = thumbAreaSize;

    int newThumbStart = thumbAreaStart;

    if (totalRange.getLength() > visibleRange.getLength())
        newThumbStart += roundToInt (((visibleRange.getStart() - totalRange.getStart()) * (thumbAreaSize - newThumbSize))
                                         / (totalRange.getLength() - visibleRange.getLength()));

    Component::setVisible (getVisibility());

    if (thumbStart != newThumbStart  || thumbSize != newThumbSize)
    {
        auto repaintStart = jmin (thumbStart, newThumbStart) - 4;
        auto repaintSize = jmax (thumbStart + thumbSize, newThumbStart + newThumbSize) + 8 - repaintStart;

        if (vertical)
            repaint (0, repaintStart, getWidth(), repaintSize);
        else
            repaint (repaintStart, 0, repaintSize, getHeight());

        thumbStart = newThumbStart;
        thumbSize = newThumbSize;
    }
}

void ScrollBar::setOrientation (bool shouldBeVertical)
{
    if (vertical != shouldBeVertical)
    {
        vertical = shouldBeVertical;

        if (upButton != nullptr)
        {
            upButton->direction    = vertical ? 0 : 3;
            downButton->direction  = vertical ? 2 : 1;
        }

        updateThumbPosition();
    }
}

void ScrollBar::setAutoHide (bool shouldHideWhenFullRange)
{
    autohides = shouldHideWhenFullRange;
    updateThumbPosition();
}

bool ScrollBar::autoHides() const noexcept
{
    return autohides;
}

//==============================================================================
void ScrollBar::paint (Graphics& g)
{
    if (thumbAreaSize > 0)
    {
        auto& lf = getLookAndFeel();

        auto thumb = (thumbAreaSize > lf.getMinimumScrollbarThumbSize (*this))
                       ? thumbSize : 0;

        if (vertical)
            lf.drawScrollbar (g, *this, 0, thumbAreaStart, getWidth(), thumbAreaSize,
                              vertical, thumbStart, thumb, isMouseOver(), isMouseButtonDown());
        else
            lf.drawScrollbar (g, *this, thumbAreaStart, 0, thumbAreaSize, getHeight(),
                              vertical, thumbStart, thumb, isMouseOver(), isMouseButtonDown());
    }
}

void ScrollBar::lookAndFeelChanged()
{
    setComponentEffect (getLookAndFeel().getScrollbarEffect());

    if (isVisible())
        resized();
}

void ScrollBar::resized()
{
    auto length = vertical ? getHeight() : getWidth();

    auto& lf = getLookAndFeel();
    bool buttonsVisible = lf.areScrollbarButtonsVisible();
    int buttonSize = 0;

    if (buttonsVisible)
    {
        if (upButton == nullptr)
        {
            upButton  .reset (new ScrollbarButton (vertical ? 0 : 3, *this));
            downButton.reset (new ScrollbarButton (vertical ? 2 : 1, *this));

            addAndMakeVisible (upButton.get());
            addAndMakeVisible (downButton.get());

            setButtonRepeatSpeed (initialDelayInMillisecs, repeatDelayInMillisecs, minimumDelayInMillisecs);
        }

        buttonSize = jmin (lf.getScrollbarButtonSize (*this), length / 2);
    }
    else
    {
        upButton.reset();
        downButton.reset();
    }

    if (length < 32 + lf.getMinimumScrollbarThumbSize (*this))
    {
        thumbAreaStart = length / 2;
        thumbAreaSize = 0;
    }
    else
    {
        thumbAreaStart = buttonSize;
        thumbAreaSize = length - 2 * buttonSize;
    }

    if (upButton != nullptr)
    {
        auto r = getLocalBounds();

        if (vertical)
        {
            upButton->setBounds (r.removeFromTop (buttonSize));
            downButton->setBounds (r.removeFromBottom (buttonSize));
        }
        else
        {
            upButton->setBounds (r.removeFromLeft (buttonSize));
            downButton->setBounds (r.removeFromRight (buttonSize));
        }
    }

    updateThumbPosition();
}

void ScrollBar::parentHierarchyChanged()
{
    lookAndFeelChanged();
}

void ScrollBar::mouseDown (const MouseEvent& e)
{
    isDraggingThumb = false;
    lastMousePos = vertical ? e.y : e.x;
    dragStartMousePos = lastMousePos;
    dragStartRange = visibleRange.getStart();

    if (dragStartMousePos < thumbStart)
    {
        moveScrollbarInPages (-1);
        startTimer (400);
    }
    else if (dragStartMousePos >= thumbStart + thumbSize)
    {
        moveScrollbarInPages (1);
        startTimer (400);
    }
    else
    {
        isDraggingThumb = (thumbAreaSize > getLookAndFeel().getMinimumScrollbarThumbSize (*this))
                            && (thumbAreaSize > thumbSize);
    }
}

void ScrollBar::mouseDrag (const MouseEvent& e)
{
    auto mousePos = vertical ? e.y : e.x;

    if (isDraggingThumb && lastMousePos != mousePos && thumbAreaSize > thumbSize)
    {
        auto deltaPixels = mousePos - dragStartMousePos;

        setCurrentRangeStart (dragStartRange
                                + deltaPixels * (totalRange.getLength() - visibleRange.getLength())
                                    / (thumbAreaSize - thumbSize));
    }

    lastMousePos = mousePos;
}

void ScrollBar::mouseUp (const MouseEvent&)
{
    isDraggingThumb = false;
    stopTimer();
    repaint();
}

void ScrollBar::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    float increment = 10.0f * (vertical ? wheel.deltaY : wheel.deltaX);

    if (increment < 0)
        increment = jmin (increment, -1.0f);
    else if (increment > 0)
        increment = jmax (increment, 1.0f);

    setCurrentRange (visibleRange - singleStepSize * increment);
}

void ScrollBar::timerCallback()
{
    if (isMouseButtonDown())
    {
        startTimer (40);

        if (lastMousePos < thumbStart)
            setCurrentRange (visibleRange - visibleRange.getLength());
        else if (lastMousePos > thumbStart + thumbSize)
            setCurrentRangeStart (visibleRange.getEnd());
    }
    else
    {
        stopTimer();
    }
}

bool ScrollBar::keyPressed (const KeyPress& key)
{
    if (isVisible())
    {
        if (key == KeyPress::upKey || key == KeyPress::leftKey)    return moveScrollbarInSteps (-1);
        if (key == KeyPress::downKey || key == KeyPress::rightKey) return moveScrollbarInSteps (1);
        if (key == KeyPress::pageUpKey)                            return moveScrollbarInPages (-1);
        if (key == KeyPress::pageDownKey)                          return moveScrollbarInPages (1);
        if (key == KeyPress::homeKey)                              return scrollToTop();
        if (key == KeyPress::endKey)                               return scrollToBottom();
    }

    return false;
}

void ScrollBar::setVisible (bool shouldBeVisible)
{
    if (userVisibilityFlag != shouldBeVisible)
    {
        userVisibilityFlag = shouldBeVisible;
        Component::setVisible (getVisibility());
    }
}

bool ScrollBar::getVisibility() const noexcept
{
    if (! userVisibilityFlag)
        return false;

    return (! autohides) || (totalRange.getLength() > visibleRange.getLength()
                                    && visibleRange.getLength() > 0.0);
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ScrollBar::createAccessibilityHandler()
{
    class ValueInterface  : public AccessibilityRangedNumericValueInterface
    {
    public:
        explicit ValueInterface (ScrollBar& scrollBarToWrap)  : scrollBar (scrollBarToWrap) {}

        bool isReadOnly() const override          { return false; }

        double getCurrentValue() const override   { return scrollBar.getCurrentRangeStart(); }
        void setValue (double newValue) override  { scrollBar.setCurrentRangeStart (newValue); }

        AccessibleValueRange getRange() const override
        {
            if (scrollBar.getRangeLimit().isEmpty())
                return {};

            return { { scrollBar.getMinimumRangeLimit(), scrollBar.getMaximumRangeLimit() },
                     scrollBar.getSingleStepSize() };
        }

    private:
        ScrollBar& scrollBar;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueInterface)
    };

    return std::make_unique<AccessibilityHandler> (*this,
                                                   AccessibilityRole::scrollBar,
                                                   AccessibilityActions{},
                                                   AccessibilityHandler::Interfaces { std::make_unique<ValueInterface> (*this) });
}

} // namespace juce
