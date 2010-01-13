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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_Desktop.h"
#include "juce_ComponentDeletionWatcher.h"
#include "../graphics/geometry/juce_RectangleList.h"

extern void juce_updateMultiMonitorInfo (Array <Rectangle>& monitorCoords,
                                         const bool clipToWorkArea);


//==============================================================================
static Desktop* juce_desktopInstance = 0;

Desktop::Desktop() throw()
    : lastMouseX (0),
      lastMouseY (0),
      kioskModeComponent (0)
{
    refreshMonitorSizes();
}

Desktop::~Desktop() throw()
{
    jassert (juce_desktopInstance == this);
    juce_desktopInstance = 0;

    // doh! If you don't delete all your windows before exiting, you're going to
    // be leaking memory!
    jassert (desktopComponents.size() == 0);
}

Desktop& JUCE_CALLTYPE Desktop::getInstance() throw()
{
    if (juce_desktopInstance == 0)
        juce_desktopInstance = new Desktop();

    return *juce_desktopInstance;
}

//==============================================================================
void Desktop::refreshMonitorSizes() throw()
{
    const Array <Rectangle> oldClipped (monitorCoordsClipped);
    const Array <Rectangle> oldUnclipped (monitorCoordsUnclipped);

    monitorCoordsClipped.clear();
    monitorCoordsUnclipped.clear();
    juce_updateMultiMonitorInfo (monitorCoordsClipped, true);
    juce_updateMultiMonitorInfo (monitorCoordsUnclipped, false);
    jassert (monitorCoordsClipped.size() > 0
              && monitorCoordsClipped.size() == monitorCoordsUnclipped.size());

    if (oldClipped != monitorCoordsClipped
         || oldUnclipped != monitorCoordsUnclipped)
    {
        for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            ComponentPeer* const p = ComponentPeer::getPeer (i);
            if (p != 0)
                p->handleScreenSizeChange();
        }
    }
}

int Desktop::getNumDisplayMonitors() const throw()
{
    return monitorCoordsClipped.size();
}

const Rectangle Desktop::getDisplayMonitorCoordinates (const int index, const bool clippedToWorkArea) const throw()
{
    return clippedToWorkArea ? monitorCoordsClipped [index]
                             : monitorCoordsUnclipped [index];
}

const RectangleList Desktop::getAllMonitorDisplayAreas (const bool clippedToWorkArea) const throw()
{
    RectangleList rl;

    for (int i = 0; i < getNumDisplayMonitors(); ++i)
        rl.addWithoutMerging (getDisplayMonitorCoordinates (i, clippedToWorkArea));

    return rl;
}

const Rectangle Desktop::getMainMonitorArea (const bool clippedToWorkArea) const throw()
{
    return getDisplayMonitorCoordinates (0, clippedToWorkArea);
}

const Rectangle Desktop::getMonitorAreaContaining (int cx, int cy, const bool clippedToWorkArea) const throw()
{
    Rectangle best (getMainMonitorArea (clippedToWorkArea));
    double bestDistance = 1.0e10;

    for (int i = getNumDisplayMonitors(); --i >= 0;)
    {
        const Rectangle rect (getDisplayMonitorCoordinates (i, clippedToWorkArea));

        if (rect.contains (cx, cy))
            return rect;

        const double distance = juce_hypot ((double) (rect.getCentreX() - cx),
                                            (double) (rect.getCentreY() - cy));

        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = rect;
        }
    }

    return best;
}

//==============================================================================
int Desktop::getNumComponents() const throw()
{
    return desktopComponents.size();
}

Component* Desktop::getComponent (const int index) const throw()
{
    return (Component*) desktopComponents [index];
}

Component* Desktop::findComponentAt (const int screenX,
                                     const int screenY) const
{
    for (int i = desktopComponents.size(); --i >= 0;)
    {
        Component* const c = (Component*) desktopComponents.getUnchecked(i);

        int x = screenX, y = screenY;
        c->globalPositionToRelative (x, y);

        if (c->contains (x, y))
            return c->getComponentAt (x, y);
    }

    return 0;
}

//==============================================================================
void Desktop::addDesktopComponent (Component* const c) throw()
{
    jassert (c != 0);
    jassert (! desktopComponents.contains (c));
    desktopComponents.addIfNotAlreadyThere (c);
}

void Desktop::removeDesktopComponent (Component* const c) throw()
{
    desktopComponents.removeValue (c);
}

void Desktop::componentBroughtToFront (Component* const c) throw()
{
    const int index = desktopComponents.indexOf (c);
    jassert (index >= 0);

    if (index >= 0)
        desktopComponents.move (index, -1);
}

//==============================================================================
// from Component.cpp
extern int juce_recentMouseDownX [4];
extern int juce_recentMouseDownY [4];
extern int juce_MouseClickCounter;

void Desktop::getLastMouseDownPosition (int& x, int& y) throw()
{
    x = juce_recentMouseDownX [0];
    y = juce_recentMouseDownY [0];
}

int Desktop::getMouseButtonClickCounter() throw()
{
    return juce_MouseClickCounter;
}

//==============================================================================
void Desktop::addGlobalMouseListener (MouseListener* const listener) throw()
{
    jassert (listener != 0);

    if (listener != 0)
    {
        mouseListeners.add (listener);
        resetTimer();
    }
}

void Desktop::removeGlobalMouseListener (MouseListener* const listener) throw()
{
    mouseListeners.removeValue (listener);
    resetTimer();
}

//==============================================================================
void Desktop::addFocusChangeListener (FocusChangeListener* const listener) throw()
{
    jassert (listener != 0);

    if (listener != 0)
        focusListeners.add (listener);
}

void Desktop::removeFocusChangeListener (FocusChangeListener* const listener) throw()
{
    focusListeners.removeValue (listener);
}

void Desktop::triggerFocusCallback() throw()
{
    triggerAsyncUpdate();
}

void Desktop::handleAsyncUpdate()
{
    for (int i = focusListeners.size(); --i >= 0;)
    {
        ((FocusChangeListener*) focusListeners.getUnchecked (i))->globalFocusChanged (Component::getCurrentlyFocusedComponent());
        i = jmin (i, focusListeners.size());
    }
}

//==============================================================================
void Desktop::timerCallback()
{
    int x, y;
    getMousePosition (x, y);

    if (lastMouseX != x || lastMouseY != y)
        sendMouseMove();
}

void Desktop::sendMouseMove()
{
    if (mouseListeners.size() > 0)
    {
        startTimer (20);

        int x, y;
        getMousePosition (x, y);
        lastMouseX = x;
        lastMouseY = y;

        Component* const target = findComponentAt (x, y);

        if (target != 0)
        {
            target->globalPositionToRelative (x, y);

            ComponentDeletionWatcher deletionChecker (target);

            const MouseEvent me (x, y,
                                 ModifierKeys::getCurrentModifiers(),
                                 target,
                                 Time::getCurrentTime(),
                                 x, y,
                                 Time::getCurrentTime(),
                                 0, false);

            for (int i = mouseListeners.size(); --i >= 0;)
            {
                if (ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown())
                    ((MouseListener*) mouseListeners[i])->mouseDrag (me);
                else
                    ((MouseListener*) mouseListeners[i])->mouseMove (me);

                if (deletionChecker.hasBeenDeleted())
                    return;

                i = jmin (i, mouseListeners.size());
            }
        }
    }
}

void Desktop::resetTimer() throw()
{
    if (mouseListeners.size() == 0)
        stopTimer();
    else
        startTimer (100);

    getMousePosition (lastMouseX, lastMouseY);
}

//==============================================================================
extern void juce_setKioskComponent (Component* kioskModeComponent, bool enableOrDisable, bool allowMenusAndBars);

void Desktop::setKioskModeComponent (Component* componentToUse, const bool allowMenusAndBars)
{
    if (kioskModeComponent != componentToUse)
    {
        // agh! Don't delete a component without first stopping it being the kiosk comp
        jassert (kioskModeComponent == 0 || kioskModeComponent->isValidComponent());
        // agh! Don't remove a component from the desktop if it's the kiosk comp!
        jassert (kioskModeComponent == 0 || kioskModeComponent->isOnDesktop());

        if (kioskModeComponent->isValidComponent())
        {
            juce_setKioskComponent (kioskModeComponent, false, allowMenusAndBars);

            kioskModeComponent->setBounds (kioskComponentOriginalBounds);
        }

        kioskModeComponent = componentToUse;

        if (kioskModeComponent != 0)
        {
            jassert (kioskModeComponent->isValidComponent());

            // Only components that are already on the desktop can be put into kiosk mode!
            jassert (kioskModeComponent->isOnDesktop());

            kioskComponentOriginalBounds = kioskModeComponent->getBounds();

            juce_setKioskComponent (kioskModeComponent, true, allowMenusAndBars);
        }
    }
}


END_JUCE_NAMESPACE
