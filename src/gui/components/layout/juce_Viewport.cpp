/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "juce_Viewport.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
Viewport::Viewport (const String& componentName)
  : Component (componentName),
    contentComp (0),
    scrollBarThickness (0),
    singleStepX (16),
    singleStepY (16),
    showHScrollbar (true),
    showVScrollbar (true)
{
    // content holder is used to clip the contents so they don't overlap the scrollbars
    addAndMakeVisible (contentHolder = new Component());
    contentHolder->setInterceptsMouseClicks (false, true);

    verticalScrollBar = new ScrollBar (true);
    horizontalScrollBar = new ScrollBar (false);

    addChildComponent (verticalScrollBar);
    addChildComponent (horizontalScrollBar);

    verticalScrollBar->addListener (this);
    horizontalScrollBar->addListener (this);

    setInterceptsMouseClicks (false, true);
    setWantsKeyboardFocus (true);
}

Viewport::~Viewport()
{
    contentHolder->deleteAllChildren();
    deleteAllChildren();
}

//==============================================================================
void Viewport::visibleAreaChanged (int, int, int, int)
{
}

//==============================================================================
void Viewport::setViewedComponent (Component* const newViewedComponent)
{
    if (contentComp.getComponent() != newViewedComponent)
    {
        {
            ScopedPointer<Component> oldCompDeleter (contentComp);
            contentComp = 0;
        }

        contentComp = newViewedComponent;

        if (contentComp != 0)
        {
            contentComp->setTopLeftPosition (0, 0);
            contentHolder->addAndMakeVisible (contentComp);
            contentComp->addComponentListener (this);
        }

        updateVisibleRegion();
    }
}

int Viewport::getMaximumVisibleWidth() const throw()
{
    return jmax (0, getWidth() - (verticalScrollBar->isVisible() ? getScrollBarThickness() : 0));
}

int Viewport::getMaximumVisibleHeight() const throw()
{
    return jmax (0, getHeight() - (horizontalScrollBar->isVisible() ? getScrollBarThickness() : 0));
}

void Viewport::setViewPosition (const int xPixelsOffset, const int yPixelsOffset)
{
    if (contentComp != 0)
        contentComp->setTopLeftPosition (-xPixelsOffset,
                                         -yPixelsOffset);
}

void Viewport::setViewPositionProportionately (const double x, const double y)
{
    if (contentComp != 0)
        setViewPosition (jmax (0, roundToInt (x * (contentComp->getWidth() - getWidth()))),
                         jmax (0, roundToInt (y * (contentComp->getHeight() - getHeight()))));
}

bool Viewport::autoScroll (int mouseX, int mouseY, int activeBorderThickness, int maximumSpeed)
{
    if (contentComp != 0)
    {
        int dx = 0, dy = 0;

        if (horizontalScrollBar->isVisible())
        {
            if (mouseX < activeBorderThickness)
                dx = activeBorderThickness - mouseX;
            else if (mouseX >= contentHolder->getWidth() - activeBorderThickness)
                dx = (contentHolder->getWidth() - activeBorderThickness) - mouseX;

            if (dx < 0)
                dx = jmax (dx, -maximumSpeed, contentHolder->getWidth() - contentComp->getRight());
            else
                dx = jmin (dx, maximumSpeed, -contentComp->getX());
        }

        if (verticalScrollBar->isVisible())
        {
            if (mouseY < activeBorderThickness)
                dy = activeBorderThickness - mouseY;
            else if (mouseY >= contentHolder->getHeight() - activeBorderThickness)
                dy = (contentHolder->getHeight() - activeBorderThickness) - mouseY;

            if (dy < 0)
                dy = jmax (dy, -maximumSpeed, contentHolder->getHeight() - contentComp->getBottom());
            else
                dy = jmin (dy, maximumSpeed, -contentComp->getY());
        }

        if (dx != 0 || dy != 0)
        {
            contentComp->setTopLeftPosition (contentComp->getX() + dx,
                                             contentComp->getY() + dy);

            return true;
        }
    }

    return false;
}

void Viewport::componentMovedOrResized (Component&, bool, bool)
{
    updateVisibleRegion();
}

void Viewport::resized()
{
    updateVisibleRegion();
}


//==============================================================================
void Viewport::updateVisibleRegion()
{
    if (contentComp != 0)
    {
        Rectangle<int> newViewPos;
        newViewPos.setPosition (-contentComp->getPosition());

        if (newViewPos.getX() == 0 && newViewPos.getY() == 0
            && contentComp->getWidth() <= getWidth()
            && contentComp->getHeight() <= getHeight())
        {
            horizontalScrollBar->setVisible (! horizontalScrollBar->autoHides());
            verticalScrollBar->setVisible (! verticalScrollBar->autoHides());
        }

        horizontalScrollBar->setRangeLimits (0.0, contentComp->getWidth());
        horizontalScrollBar->setCurrentRange (newViewPos.getX(), getMaximumVisibleWidth());
        horizontalScrollBar->setSingleStepSize (singleStepX);

        if (! (contentComp->getWidth() > 0 && showHScrollbar && getHeight() > getScrollBarThickness()))
            horizontalScrollBar->setVisible (! horizontalScrollBar->autoHides());

        verticalScrollBar->setRangeLimits (0.0, contentComp->getHeight());
        verticalScrollBar->setCurrentRange (newViewPos.getY(), getMaximumVisibleHeight());
        verticalScrollBar->setSingleStepSize (singleStepY);

        if (! (contentComp->getHeight() > 0 && showVScrollbar && getWidth() > getScrollBarThickness()))
            verticalScrollBar->setVisible (! verticalScrollBar->autoHides());

        if (verticalScrollBar->isVisible())
        {
            horizontalScrollBar->setCurrentRange (newViewPos.getX(), getMaximumVisibleWidth());
            verticalScrollBar->setCurrentRange (newViewPos.getY(), getMaximumVisibleHeight());

            verticalScrollBar
                ->setBounds (getMaximumVisibleWidth(), 0,
                             getScrollBarThickness(), getMaximumVisibleHeight());
        }

        if (horizontalScrollBar->isVisible())
        {
            horizontalScrollBar->setCurrentRange (newViewPos.getX(), getMaximumVisibleWidth());

            horizontalScrollBar
                ->setBounds (0, getMaximumVisibleHeight(),
                             getMaximumVisibleWidth(), getScrollBarThickness());
        }

        contentHolder->setSize (getMaximumVisibleWidth(),
                                getMaximumVisibleHeight());

        newViewPos.setSize (jmin (contentComp->getRight(),  getMaximumVisibleWidth()),
                            jmin (contentComp->getBottom(), getMaximumVisibleHeight()));

        if (lastViewPos != newViewPos)
        {
            lastViewPos = newViewPos;
            visibleAreaChanged (newViewPos.getX(), newViewPos.getY(), newViewPos.getWidth(), newViewPos.getHeight());
        }

        horizontalScrollBar->handleUpdateNowIfNeeded();
        verticalScrollBar->handleUpdateNowIfNeeded();
    }
    else
    {
        horizontalScrollBar->setVisible (! horizontalScrollBar->autoHides());
        verticalScrollBar->setVisible (! verticalScrollBar->autoHides());
    }
}

//==============================================================================
void Viewport::setSingleStepSizes (const int stepX, const int stepY)
{
    singleStepX = stepX;
    singleStepY = stepY;
    updateVisibleRegion();
}

void Viewport::setScrollBarsShown (const bool showVerticalScrollbarIfNeeded,
                                   const bool showHorizontalScrollbarIfNeeded)
{
    if (showVScrollbar != showVerticalScrollbarIfNeeded
         || showHScrollbar != showHorizontalScrollbarIfNeeded)
    {
        showVScrollbar = showVerticalScrollbarIfNeeded;
        showHScrollbar = showHorizontalScrollbarIfNeeded;
        horizontalScrollBar->setVisible (true);
        verticalScrollBar->setVisible (true);
        updateVisibleRegion();
    }
}

void Viewport::setScrollBarThickness (const int thickness)
{
    scrollBarThickness = thickness;
    updateVisibleRegion();
}

int Viewport::getScrollBarThickness() const throw()
{
    return (scrollBarThickness > 0) ? scrollBarThickness
                                    : getLookAndFeel().getDefaultScrollbarWidth();
}

void Viewport::setScrollBarButtonVisibility (const bool buttonsVisible)
{
    verticalScrollBar->setButtonVisibility (buttonsVisible);
    horizontalScrollBar->setButtonVisibility (buttonsVisible);
}

void Viewport::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart)
{
    if (scrollBarThatHasMoved == horizontalScrollBar)
    {
        setViewPosition (roundToInt (newRangeStart), getViewPositionY());
    }
    else if (scrollBarThatHasMoved == verticalScrollBar)
    {
        setViewPosition (getViewPositionX(), roundToInt (newRangeStart));
    }
}

void Viewport::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    if (! useMouseWheelMoveIfNeeded (e, wheelIncrementX, wheelIncrementY))
        Component::mouseWheelMove (e, wheelIncrementX, wheelIncrementY);
}

bool Viewport::useMouseWheelMoveIfNeeded (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    if (! (e.mods.isAltDown() || e.mods.isCtrlDown()))
    {
        const bool hasVertBar = verticalScrollBar->isVisible();
        const bool hasHorzBar = horizontalScrollBar->isVisible();

        if (hasHorzBar && (wheelIncrementX != 0 || e.mods.isShiftDown() || ! hasVertBar))
        {
            if (wheelIncrementX == 0 && ! hasVertBar)
                wheelIncrementX = wheelIncrementY;

            horizontalScrollBar->mouseWheelMove (e.getEventRelativeTo (horizontalScrollBar),
                                                 wheelIncrementX, wheelIncrementY);
            return true;
        }
        else if (hasVertBar && wheelIncrementY != 0)
        {
            verticalScrollBar->mouseWheelMove (e.getEventRelativeTo (verticalScrollBar),
                                               wheelIncrementX, wheelIncrementY);
            return true;
        }
    }

    return false;
}

bool Viewport::keyPressed (const KeyPress& key)
{
    const bool isUpDownKey = key.isKeyCode (KeyPress::upKey)
                                || key.isKeyCode (KeyPress::downKey)
                                || key.isKeyCode (KeyPress::pageUpKey)
                                || key.isKeyCode (KeyPress::pageDownKey)
                                || key.isKeyCode (KeyPress::homeKey)
                                || key.isKeyCode (KeyPress::endKey);

    if (verticalScrollBar->isVisible() && isUpDownKey)
        return verticalScrollBar->keyPressed (key);

    const bool isLeftRightKey = key.isKeyCode (KeyPress::leftKey)
                                 || key.isKeyCode (KeyPress::rightKey);

    if (horizontalScrollBar->isVisible() && (isUpDownKey || isLeftRightKey))
        return horizontalScrollBar->keyPressed (key);

    return false;
}


END_JUCE_NAMESPACE
