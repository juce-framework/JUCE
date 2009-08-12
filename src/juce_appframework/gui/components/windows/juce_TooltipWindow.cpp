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


#include "juce_TooltipWindow.h"
#include "../../../../juce_core/basics/juce_Time.h"
#include "../../../../juce_core/threads/juce_Process.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../juce_Desktop.h"


//==============================================================================
TooltipWindow::TooltipWindow (Component* const parentComponent,
                              const int millisecondsBeforeTipAppears_)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (millisecondsBeforeTipAppears_),
      mouseX (0),
      mouseY (0),
      lastHideTime (0),
      lastComponentUnderMouse (0),
      changedCompsSinceShown (true)
{
    startTimer (123);

    setAlwaysOnTop (true);
    setOpaque (true);

    if (parentComponent != 0)
        parentComponent->addChildComponent (this);
}

TooltipWindow::~TooltipWindow()
{
    hide();
}

void TooltipWindow::setMillisecondsBeforeTipAppears (const int newTimeMs) throw()
{
    millisecondsBeforeTipAppears = newTimeMs;
}

void TooltipWindow::paint (Graphics& g)
{
    getLookAndFeel().drawTooltip (g, tipShowing, getWidth(), getHeight());
}

void TooltipWindow::mouseEnter (const MouseEvent&)
{
    hide();
}

void TooltipWindow::showFor (Component* const c, const String& tip)
{
    jassert (tip.isNotEmpty());
    tipShowing = tip;

    int mx, my;
    Desktop::getMousePosition (mx, my);

    if (getParentComponent() != 0)
        getParentComponent()->globalPositionToRelative (mx, my);

    int x, y, w, h;
    getLookAndFeel().getTooltipSize (tip, w, h);

    if (mx > getParentWidth() / 2)
        x = mx - (w + 12);
    else
        x = mx + 24;

    if (my > getParentHeight() / 2)
        y = my - (h + 6);
    else
        y = my + 6;

    setBounds (x, y, w, h);
    setVisible (true);

    if (getParentComponent() == 0)
    {
        addToDesktop (ComponentPeer::windowHasDropShadow
                        | ComponentPeer::windowIsTemporary);
    }

    toFront (false);
}

const String TooltipWindow::getTipFor (Component* const c)
{
    if (c->isValidComponent()
         && Process::isForegroundProcess()
         && ! Component::isMouseButtonDownAnywhere())
    {
        TooltipClient* const ttc = dynamic_cast <TooltipClient*> (c);

        if (ttc != 0 && ! c->isCurrentlyBlockedByAnotherModalComponent())
            return ttc->getTooltip();
    }

    return String::empty;
}

void TooltipWindow::hide()
{
    tipShowing = String::empty;
    removeFromDesktop();
    setVisible (false);
}

void TooltipWindow::timerCallback()
{
    const unsigned int now = Time::getApproximateMillisecondCounter();
    Component* const newComp = Component::getComponentUnderMouse();
    const String newTip (getTipFor (newComp));

    const bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
    lastComponentUnderMouse = newComp;
    lastTipUnderMouse = newTip;

    const int clickCount = Desktop::getInstance().getMouseButtonClickCounter();
    const bool mouseWasClicked = clickCount > mouseClicks;
    mouseClicks = clickCount;

    int mx, my;
    Desktop::getMousePosition (mx, my);
    const bool mouseMovedQuickly = (abs (mx - mouseX) + abs (my - mouseY) > 12);
    mouseX = mx;
    mouseY = my;

    if (tipChanged || mouseWasClicked || mouseMovedQuickly)
        lastCompChangeTime = now;

    if (isVisible() || now < lastHideTime + 500)
    {
        // if a tip is currently visible (or has just disappeared), update to a new one
        // immediately if needed..
        if (newComp == 0 || mouseWasClicked || newTip.isEmpty())
        {
            if (isVisible())
            {
                lastHideTime = now;
                hide();
            }
        }
        else if (tipChanged)
        {
            showFor (newComp, newTip);
        }
    }
    else
    {
        // if there isn't currently a tip, but one is needed, only let it
        // appear after a timeout..
        if (newTip.isNotEmpty()
             && newTip != tipShowing
             && now > lastCompChangeTime + millisecondsBeforeTipAppears)
        {
            showFor (newComp, newTip);
        }
    }
}

END_JUCE_NAMESPACE
