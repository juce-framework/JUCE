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
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../juce_Desktop.h"


//==============================================================================
TooltipWindow::TooltipWindow (Component* const parentComponent,
                              const int millisecondsBeforeTipAppears_)
    : Component ("tooltip"),
      millisecondsBeforeTipAppears (millisecondsBeforeTipAppears_),
      mouseX (0),
      mouseY (0),
      lastMouseMoveTime (0),
      lastHideTime (0),
      lastComponentUnderMouse (0),
      changedCompsSinceShown (true)
{
    startTimer (123);

    setAlwaysOnTop (true);
    setOpaque (true);

    if (parentComponent != 0)
    {
        parentComponent->addChildComponent (this);
    }
    else
    {
        setSize (1, 1); // to keep the OS happy by not having zero-size windows
        addToDesktop (ComponentPeer::windowHasDropShadow
                        | ComponentPeer::windowIsTemporary);
    }
}

TooltipWindow::~TooltipWindow()
{
}

void TooltipWindow::paint (Graphics& g)
{
    getLookAndFeel().drawTooltip (g, tip, getWidth(), getHeight());
}

void TooltipWindow::mouseEnter (const MouseEvent&)
{
    setVisible (false);
}

void TooltipWindow::showFor (Component* const c)
{
    TooltipClient* const ttc = dynamic_cast <TooltipClient*> (c);

    if (ttc != 0 && ! c->isCurrentlyBlockedByAnotherModalComponent())
        tip = ttc->getTooltip();
    else
        tip = String::empty;

    if (tip.isEmpty())
    {
        setVisible (false);
    }
    else
    {
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
        toFront (false);
    }
}

void TooltipWindow::timerCallback()
{
    int mx, my;
    Desktop::getMousePosition (mx, my);

    const unsigned int now = Time::getApproximateMillisecondCounter();
    Component* const underMouse = Component::getComponentUnderMouse();
    const bool changedComp = (underMouse != lastComponentUnderMouse);
    lastComponentUnderMouse = underMouse;

    if (changedComp
         || abs (mx - mouseX) > 4
         || abs (my - mouseY) > 4
         || Desktop::getInstance().getMouseButtonClickCounter() > mouseClicks)
    {
        lastMouseMoveTime = now;

        if (isVisible())
        {
            lastHideTime = now;
            setVisible (false);
        }

        changedCompsSinceShown = changedCompsSinceShown || changedComp;

        tip = String::empty;

        mouseX = mx;
        mouseY = my;
    }

    if (changedCompsSinceShown)
    {
        if ((now > lastMouseMoveTime + millisecondsBeforeTipAppears
              || now < lastHideTime + 500)
             && ! isVisible())
        {
            if (underMouse->isValidComponent())
                showFor (underMouse);

            changedCompsSinceShown = false;
        }
    }

    mouseClicks = Desktop::getInstance().getMouseButtonClickCounter();
}

END_JUCE_NAMESPACE
