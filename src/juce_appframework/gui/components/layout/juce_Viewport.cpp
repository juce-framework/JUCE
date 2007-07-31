/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Viewport.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
Viewport::Viewport (const String& componentName)
  : Component (componentName),
    contentComp (0),
    lastVX (0),
    lastVY (0),
    lastVW (0),
    lastVH (0),
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
    if (contentComp != newViewedComponent)
    {
        if (contentComp->isValidComponent())
        {
            Component* const oldComp = contentComp;
            contentComp = 0;
            delete oldComp;
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

void Viewport::setViewPosition (const int xPixelsOffset,
                                const int yPixelsOffset)
{
    if (contentComp != 0)
        contentComp->setTopLeftPosition (-xPixelsOffset,
                                         -yPixelsOffset);
}

void Viewport::setViewPositionProportionately (const double x,
                                               const double y)
{
    if (contentComp != 0)
        setViewPosition (jmax (0, roundDoubleToInt (x * (contentComp->getWidth() - getWidth()))),
                         jmax (0, roundDoubleToInt (y * (contentComp->getHeight() - getHeight()))));
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
        const int newVX = -contentComp->getX();
        const int newVY = -contentComp->getY();

        if (newVX == 0 && newVY == 0
            && contentComp->getWidth() <= getWidth()
            && contentComp->getHeight() <= getHeight())
        {
            horizontalScrollBar->setVisible (false);
            verticalScrollBar->setVisible (false);
        }

        if ((contentComp->getWidth() > 0) && showHScrollbar
             && getHeight() > getScrollBarThickness())
        {
            horizontalScrollBar->setRangeLimits (0.0, contentComp->getWidth());
            horizontalScrollBar->setCurrentRange (newVX, getMaximumVisibleWidth());
            horizontalScrollBar->setSingleStepSize (singleStepX);
        }
        else
        {
            horizontalScrollBar->setVisible (false);
        }

        if ((contentComp->getHeight() > 0) && showVScrollbar
             && getWidth() > getScrollBarThickness())
        {
            verticalScrollBar->setRangeLimits (0.0, contentComp->getHeight());
            verticalScrollBar->setCurrentRange (newVY, getMaximumVisibleHeight());
            verticalScrollBar->setSingleStepSize (singleStepY);
        }
        else
        {
            verticalScrollBar->setVisible (false);
        }

        if (verticalScrollBar->isVisible())
        {
            horizontalScrollBar->setCurrentRange (newVX, getMaximumVisibleWidth());
            verticalScrollBar->setCurrentRange (newVY, getMaximumVisibleHeight());

            verticalScrollBar
                ->setBounds (getMaximumVisibleWidth(), 0,
                             getScrollBarThickness(), getMaximumVisibleHeight());
        }

        if (horizontalScrollBar->isVisible())
        {
            horizontalScrollBar
                ->setBounds (0, getMaximumVisibleHeight(),
                             getMaximumVisibleWidth(), getScrollBarThickness());
        }

        contentHolder->setSize (getMaximumVisibleWidth(),
                                getMaximumVisibleHeight());

        const int newVW = jmin (contentComp->getRight(),  getMaximumVisibleWidth());
        const int newVH = jmin (contentComp->getBottom(), getMaximumVisibleHeight());

        if (newVX != lastVX
             || newVY != lastVY
             || newVW != lastVW
             || newVH != lastVH)
        {
            lastVX = newVX;
            lastVY = newVY;
            lastVW = newVW;
            lastVH = newVH;

            visibleAreaChanged (newVX, newVY, newVW, newVH);
        }

        horizontalScrollBar->handleUpdateNowIfNeeded();
        verticalScrollBar->handleUpdateNowIfNeeded();
    }
    else
    {
        horizontalScrollBar->setVisible (false);
        verticalScrollBar->setVisible (false);
    }
}

//==============================================================================
void Viewport::setSingleStepSizes (const int stepX,
                                   const int stepY)
{
    singleStepX = stepX;
    singleStepY = stepY;
    updateVisibleRegion();
}

void Viewport::setScrollBarsShown (const bool showVerticalScrollbarIfNeeded,
                                   const bool showHorizontalScrollbarIfNeeded)
{
    showVScrollbar = showVerticalScrollbarIfNeeded;
    showHScrollbar = showHorizontalScrollbarIfNeeded;
    updateVisibleRegion();
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

void Viewport::scrollBarMoved (ScrollBar* scrollBarThatHasMoved, const double newRangeStart)
{
    if (scrollBarThatHasMoved == horizontalScrollBar)
    {
        setViewPosition (roundDoubleToInt (newRangeStart), getViewPositionY());
    }
    else if (scrollBarThatHasMoved == verticalScrollBar)
    {
        setViewPosition (getViewPositionX(), roundDoubleToInt (newRangeStart));
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
