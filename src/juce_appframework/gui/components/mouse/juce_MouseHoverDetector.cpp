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
