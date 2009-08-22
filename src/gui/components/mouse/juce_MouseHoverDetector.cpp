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

#include "../juce_Component.h"
#include "juce_MouseHoverDetector.h"


//==============================================================================
MouseHoverDetector::MouseHoverDetector (const int hoverTimeMillisecs_)
    : source (0),
      hoverTimeMillisecs (hoverTimeMillisecs_),
      hasJustHovered (false)
{
    internalTimer.owner = this;
}

MouseHoverDetector::~MouseHoverDetector()
{
    setHoverComponent (0);
}

void MouseHoverDetector::setHoverTimeMillisecs (const int newTimeInMillisecs)
{
    hoverTimeMillisecs = newTimeInMillisecs;
}

void MouseHoverDetector::setHoverComponent (Component* const newSourceComponent)
{
    if (source != newSourceComponent)
    {
        internalTimer.stopTimer();
        hasJustHovered = false;

        if (source != 0)
        {
            // ! you need to delete the hover detector before deleting its component
            jassert (source->isValidComponent());

            source->removeMouseListener (&internalTimer);
        }

        source = newSourceComponent;

        if (newSourceComponent != 0)
            newSourceComponent->addMouseListener (&internalTimer, false);
    }
}

void MouseHoverDetector::hoverTimerCallback()
{
    internalTimer.stopTimer();

    if (source != 0)
    {
        int mx, my;
        source->getMouseXYRelative (mx, my);

        if (source->reallyContains (mx, my, false))
        {
            hasJustHovered = true;
            mouseHovered (mx, my);
        }
    }
}

void MouseHoverDetector::checkJustHoveredCallback()
{
    if (hasJustHovered)
    {
        hasJustHovered = false;
        mouseMovedAfterHover();
    }
}

//==============================================================================
void MouseHoverDetector::HoverDetectorInternal::timerCallback()
{
    owner->hoverTimerCallback();
}

void MouseHoverDetector::HoverDetectorInternal::mouseEnter (const MouseEvent&)
{
    stopTimer();
    owner->checkJustHoveredCallback();
}

void MouseHoverDetector::HoverDetectorInternal::mouseExit (const MouseEvent&)
{
    stopTimer();
    owner->checkJustHoveredCallback();
}

void MouseHoverDetector::HoverDetectorInternal::mouseDown (const MouseEvent&)
{
    stopTimer();
    owner->checkJustHoveredCallback();
}

void MouseHoverDetector::HoverDetectorInternal::mouseUp (const MouseEvent&)
{
    stopTimer();
    owner->checkJustHoveredCallback();
}

void MouseHoverDetector::HoverDetectorInternal::mouseMove (const MouseEvent& e)
{
    if (lastX != e.x || lastY != e.y) // to avoid fake mouse-moves setting it off
    {
        lastX = e.x;
        lastY = e.y;

        if (owner->source != 0)
            startTimer (owner->hoverTimeMillisecs);

        owner->checkJustHoveredCallback();
    }
}

void MouseHoverDetector::HoverDetectorInternal::mouseWheelMove (const MouseEvent&, float, float)
{
    stopTimer();
    owner->checkJustHoveredCallback();
}

END_JUCE_NAMESPACE
