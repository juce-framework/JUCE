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

#ifndef __JUCE_SCROLLBAR_JUCEHEADER__
#define __JUCE_SCROLLBAR_JUCEHEADER__

#include "../../../events/juce_AsyncUpdater.h"
#include "../../../events/juce_Timer.h"
#include "../buttons/juce_Button.h"
class ScrollBar;


//==============================================================================
/**
    A class for receiving events from a ScrollBar.

    You can register a ScrollBarListener with a ScrollBar using the ScrollBar::addListener()
    method, and it will be called when the bar's position changes.

    @see ScrollBar::addListener, ScrollBar::removeListener
*/
class JUCE_API  ScrollBarListener
{
public:
    /** Destructor. */
    virtual ~ScrollBarListener() {}

    /** Called when a ScrollBar is moved.

        @param scrollBarThatHasMoved    the bar that has moved
        @param newRangeStart            the new range start of this bar
    */
    virtual void scrollBarMoved (ScrollBar* scrollBarThatHasMoved,
                                 const double newRangeStart) = 0;
};


//==============================================================================
/**
    A scrollbar component.

    To use a scrollbar, set up its total range using the setRangeLimits() method - this
    sets the range of values it can represent. Then you can use setCurrentRange() to
    change the position and size of the scrollbar's 'thumb'.

    Registering a ScrollBarListener with the scrollbar will allow you to find out when
    the user moves it, and you can use the getCurrentRangeStart() to find out where
    they moved it to.

    The scrollbar will adjust its own visibility according to whether its thumb size
    allows it to actually be scrolled.

    For most purposes, it's probably easier to use a ViewportContainer or ListBox
    instead of handling a scrollbar directly.

    @see ScrollBarListener
*/
class JUCE_API  ScrollBar  : public Component,
                             public AsyncUpdater,
                             private Timer
{
public:
    //==============================================================================
    /** Creates a Scrollbar.

        @param isVertical           whether it should be a vertical or horizontal bar
        @param buttonsAreVisible    whether to show the up/down or left/right buttons
    */
    ScrollBar (const bool isVertical,
               const bool buttonsAreVisible = true);

    /** Destructor. */
    ~ScrollBar();

    //==============================================================================
    /** Returns true if the scrollbar is vertical, false if it's horizontal. */
    bool isVertical() const throw()                                 { return vertical; }

    /** Changes the scrollbar's direction.

        You'll also need to resize the bar appropriately - this just changes its internal
        layout.

        @param shouldBeVertical     true makes it vertical; false makes it horizontal.
    */
    void setOrientation (const bool shouldBeVertical) throw();

    /** Shows or hides the scrollbar's buttons. */
    void setButtonVisibility (const bool buttonsAreVisible);

    /** Tells the scrollbar whether to make itself invisible when not needed.

        The default behaviour is for a scrollbar to become invisible when the thumb
        fills the whole of its range (i.e. when it can't be moved). Setting this
        value to false forces the bar to always be visible.
    */
    void setAutoHide (const bool shouldHideWhenFullRange);

    //==============================================================================
    /** Sets the minimum and maximum values that the bar will move between.

        The bar's thumb will always be constrained so that the top of the thumb
        will be >= minimum, and the bottom of the thumb <= maximum.

        @see setCurrentRange
    */
    void setRangeLimits (const double minimum,
                         const double maximum) throw();

    /** Returns the lower value that the thumb can be set to.

        This is the value set by setRangeLimits().
    */
    double getMinimumRangeLimit() const throw()                     { return minimum; }

    /** Returns the upper value that the thumb can be set to.

        This is the value set by setRangeLimits().
    */
    double getMaximumRangeLimit() const throw()                     { return maximum; }

    //==============================================================================
    /** Changes the position of the scrollbar's 'thumb'.

        This sets both the position and size of the thumb - to just set the position without
        changing the size, you can use setCurrentRangeStart().

        If this method call actually changes the scrollbar's position, it will trigger an
        asynchronous call to ScrollBarListener::scrollBarMoved() for all the listeners that
        are registered.

        @param newStart     the top (or left) of the thumb, in the range
                            getMinimumRangeLimit() <= newStart <= getMaximumRangeLimit(). If the
                            value is beyond these limits, it will be clipped.
        @param newSize      the size of the thumb, such that
                            getMinimumRangeLimit() <= newStart + newSize <= getMaximumRangeLimit(). If the
                            size is beyond these limits, it will be clipped.
        @see setCurrentRangeStart, getCurrentRangeStart, getCurrentRangeSize
    */
    void setCurrentRange (double newStart,
                          double newSize) throw();

    /** Moves the bar's thumb position.

        This will move the thumb position without changing the thumb size. Note
        that the maximum thumb start position is (getMaximumRangeLimit() - getCurrentRangeSize()).

        If this method call actually changes the scrollbar's position, it will trigger an
        asynchronous call to ScrollBarListener::scrollBarMoved() for all the listeners that
        are registered.

        @see setCurrentRange
    */
    void setCurrentRangeStart (double newStart) throw();

    /** Returns the position of the top of the thumb.

        @see setCurrentRangeStart
    */
    double getCurrentRangeStart() const throw()                     { return rangeStart; }

    /** Returns the current size of the thumb.

        @see setCurrentRange
    */
    double getCurrentRangeSize() const throw()                      { return rangeSize; }

    //==============================================================================
    /** Sets the amount by which the up and down buttons will move the bar.

        The value here is in terms of the total range, and is added or subtracted
        from the thumb position when the user clicks an up/down (or left/right) button.
    */
    void setSingleStepSize (const double newSingleStepSize) throw();

    /** Moves the scrollbar by a number of single-steps.

        This will move the bar by a multiple of its single-step interval (as
        specified using the setSingleStepSize() method).

        A positive value here will move the bar down or to the right, a negative
        value moves it up or to the left.
    */
    void moveScrollbarInSteps (const int howManySteps) throw();

    /** Moves the scroll bar up or down in pages.

        This will move the bar by a multiple of its current thumb size, effectively
        doing a page-up or down.

        A positive value here will move the bar down or to the right, a negative
        value moves it up or to the left.
    */
    void moveScrollbarInPages (const int howManyPages) throw();

    /** Scrolls to the top (or left).

        This is the same as calling setCurrentRangeStart (getMinimumRangeLimit());
    */
    void scrollToTop() throw();

    /** Scrolls to the bottom (or right).

        This is the same as calling setCurrentRangeStart (getMaximumRangeLimit() - getCurrentRangeSize());
    */
    void scrollToBottom() throw();

    /** Changes the delay before the up and down buttons autorepeat when they are held
        down.

        For an explanation of what the parameters are for, see Button::setRepeatSpeed().

        @see Button::setRepeatSpeed
    */
    void setButtonRepeatSpeed (const int initialDelayInMillisecs,
                               const int repeatDelayInMillisecs,
                               const int minimumDelayInMillisecs = -1) throw();

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId          = 0x1000300,    /**< The background colour of the scrollbar. */
        thumbColourId               = 0x1000400,    /**< A base colour to use for the thumb. The look and feel will probably use variations on this colour. */
        trackColourId               = 0x1000401     /**< A base colour to use for the slot area of the bar. The look and feel will probably use variations on this colour. */
    };

    //==============================================================================
    /** Registers a listener that will be called when the scrollbar is moved. */
    void addListener (ScrollBarListener* const listener) throw();

    /** Deregisters a previously-registered listener. */
    void removeListener (ScrollBarListener* const listener) throw();

    //==============================================================================
    /** @internal */
    bool keyPressed (const KeyPress& key);
    /** @internal */
    void mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY);
    /** @internal */
    void lookAndFeelChanged();
    /** @internal */
    void handleAsyncUpdate();
    /** @internal */
    void mouseDown (const MouseEvent& e);
    /** @internal */
    void mouseDrag (const MouseEvent& e);
    /** @internal */
    void mouseUp   (const MouseEvent& e);
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void resized();

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    double minimum, maximum;
    double rangeStart, rangeSize;
    double singleStepSize, dragStartRange;
    int thumbAreaStart, thumbAreaSize, thumbStart, thumbSize;
    int dragStartMousePos, lastMousePos;
    int initialDelayInMillisecs, repeatDelayInMillisecs, minimumDelayInMillisecs;
    bool vertical, isDraggingThumb, alwaysVisible;
    Button* upButton;
    Button* downButton;
    SortedSet <void*> listeners;

    void updateThumbPosition() throw();
    void timerCallback();

    ScrollBar (const ScrollBar&);
    const ScrollBar& operator= (const ScrollBar&);
};


#endif   // __JUCE_SCROLLBAR_JUCEHEADER__
