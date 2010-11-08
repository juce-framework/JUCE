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

#include "juce_TooltipWindow.h"
#include "../windows/juce_ComponentPeer.h"
#include "../../../core/juce_Time.h"
#include "../../../threads/juce_Process.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../juce_Desktop.h"
#include "../mouse/juce_MouseInputSource.h"


//==============================================================================
TooltipWindow::TooltipWindow (Component* const parentComponent,
                              const int millisecondsBeforeTipAppears_)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (millisecondsBeforeTipAppears_),
      mouseClicks (0),
      lastHideTime (0),
      lastComponentUnderMouse (0),
      changedCompsSinceShown (true)
{
    if (Desktop::getInstance().getMainMouseSource().canHover())
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

void TooltipWindow::showFor (const String& tip)
{
    jassert (tip.isNotEmpty());
    if (tipShowing != tip)
        repaint();

    tipShowing = tip;

    Point<int> mousePos (Desktop::getMousePosition());

    if (getParentComponent() != 0)
        mousePos = getParentComponent()->getLocalPoint (0, mousePos);

    int x, y, w, h;
    getLookAndFeel().getTooltipSize (tip, w, h);

    if (mousePos.getX() > getParentWidth() / 2)
        x = mousePos.getX() - (w + 12);
    else
        x = mousePos.getX() + 24;

    if (mousePos.getY() > getParentHeight() / 2)
        y = mousePos.getY() - (h + 6);
    else
        y = mousePos.getY() + 6;

    setBounds (x, y, w, h);
    setVisible (true);

    if (getParentComponent() == 0)
    {
        addToDesktop (ComponentPeer::windowHasDropShadow
                        | ComponentPeer::windowIsTemporary
                        | ComponentPeer::windowIgnoresKeyPresses);
    }

    toFront (false);
}

const String TooltipWindow::getTipFor (Component* const c)
{
    if (c != 0
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
    Component* const newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();
    const String newTip (getTipFor (newComp));

    const bool tipChanged = (newTip != lastTipUnderMouse || newComp != lastComponentUnderMouse);
    lastComponentUnderMouse = newComp;
    lastTipUnderMouse = newTip;

    const int clickCount = Desktop::getInstance().getMouseButtonClickCounter();
    const bool mouseWasClicked = clickCount > mouseClicks;
    mouseClicks = clickCount;

    const Point<int> mousePos (Desktop::getMousePosition());
    const bool mouseMovedQuickly = mousePos.getDistanceFrom (lastMousePos) > 12;
    lastMousePos = mousePos;

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
            showFor (newTip);
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
            showFor (newTip);
        }
    }
}

END_JUCE_NAMESPACE
