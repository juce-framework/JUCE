/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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


Desktop::Desktop()
    : mouseClickCounter (0),
      kioskModeComponent (nullptr),
      allowedOrientations (allOrientations)
{
    createMouseInputSources();
    refreshMonitorSizes();
}

Desktop::~Desktop()
{
    setScreenSaverEnabled (true);

    jassert (instance == this);
    instance = nullptr;

    // doh! If you don't delete all your windows before exiting, you're going to
    // be leaking memory!
    jassert (desktopComponents.size() == 0);
}

Desktop& JUCE_CALLTYPE Desktop::getInstance()
{
    if (instance == nullptr)
        instance = new Desktop();

    return *instance;
}

Desktop* Desktop::instance = nullptr;

//==============================================================================
void Desktop::refreshMonitorSizes()
{
    Array <Rectangle<int> > oldClipped, oldUnclipped;
    oldClipped.swapWithArray (monitorCoordsClipped);
    oldUnclipped.swapWithArray (monitorCoordsUnclipped);

    getCurrentMonitorPositions (monitorCoordsClipped, true);
    getCurrentMonitorPositions (monitorCoordsUnclipped, false);
    jassert (monitorCoordsClipped.size() == monitorCoordsUnclipped.size());

    if (oldClipped != monitorCoordsClipped
         || oldUnclipped != monitorCoordsUnclipped)
    {
        for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            ComponentPeer* const p = ComponentPeer::getPeer (i);
            if (p != nullptr)
                p->handleScreenSizeChange();
        }
    }
}

int Desktop::getNumDisplayMonitors() const noexcept
{
    return monitorCoordsClipped.size();
}

Rectangle<int> Desktop::getDisplayMonitorCoordinates (const int index, const bool clippedToWorkArea) const noexcept
{
    return clippedToWorkArea ? monitorCoordsClipped [index]
                             : monitorCoordsUnclipped [index];
}

RectangleList Desktop::getAllMonitorDisplayAreas (const bool clippedToWorkArea) const
{
    RectangleList rl;

    for (int i = 0; i < getNumDisplayMonitors(); ++i)
        rl.addWithoutMerging (getDisplayMonitorCoordinates (i, clippedToWorkArea));

    return rl;
}

Rectangle<int> Desktop::getMainMonitorArea (const bool clippedToWorkArea) const noexcept
{
    return getDisplayMonitorCoordinates (0, clippedToWorkArea);
}

Rectangle<int> Desktop::getMonitorAreaContaining (const Point<int>& position, const bool clippedToWorkArea) const
{
    Rectangle<int> best (getMainMonitorArea (clippedToWorkArea));
    double bestDistance = 1.0e10;

    for (int i = getNumDisplayMonitors(); --i >= 0;)
    {
        const Rectangle<int> rect (getDisplayMonitorCoordinates (i, clippedToWorkArea));

        if (rect.contains (position))
            return rect;

        const double distance = rect.getCentre().getDistanceFrom (position);

        if (distance < bestDistance)
        {
            bestDistance = distance;
            best = rect;
        }
    }

    return best;
}

//==============================================================================
int Desktop::getNumComponents() const noexcept
{
    return desktopComponents.size();
}

Component* Desktop::getComponent (const int index) const noexcept
{
    return desktopComponents [index];
}

Component* Desktop::findComponentAt (const Point<int>& screenPosition) const
{
    for (int i = desktopComponents.size(); --i >= 0;)
    {
        Component* const c = desktopComponents.getUnchecked(i);

        if (c->isVisible())
        {
            const Point<int> relative (c->getLocalPoint (nullptr, screenPosition));

            if (c->contains (relative))
                return c->getComponentAt (relative);
        }
    }

    return nullptr;
}

//==============================================================================
LookAndFeel& Desktop::getDefaultLookAndFeel() noexcept
{
    if (currentLookAndFeel == nullptr)
    {
        if (defaultLookAndFeel == nullptr)
            defaultLookAndFeel = new LookAndFeel();

        currentLookAndFeel = defaultLookAndFeel;
    }

    return *currentLookAndFeel;
}

void Desktop::setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel)
{
    currentLookAndFeel = newDefaultLookAndFeel;

    for (int i = getNumComponents(); --i >= 0;)
    {
        Component* const c = getComponent (i);

        if (c != nullptr)
            c->sendLookAndFeelChange();
    }
}

//==============================================================================
void Desktop::addDesktopComponent (Component* const c)
{
    jassert (c != nullptr);
    jassert (! desktopComponents.contains (c));
    desktopComponents.addIfNotAlreadyThere (c);
}

void Desktop::removeDesktopComponent (Component* const c)
{
    desktopComponents.removeValue (c);
}

void Desktop::componentBroughtToFront (Component* const c)
{
    const int index = desktopComponents.indexOf (c);
    jassert (index >= 0);

    if (index >= 0)
    {
        int newIndex = -1;

        if (! c->isAlwaysOnTop())
        {
            newIndex = desktopComponents.size();

            while (newIndex > 0 && desktopComponents.getUnchecked (newIndex - 1)->isAlwaysOnTop())
                --newIndex;

            --newIndex;
        }

        desktopComponents.move (index, newIndex);
    }
}

//==============================================================================
Point<int> Desktop::getMousePosition()
{
    return getInstance().getMainMouseSource().getScreenPosition();
}

Point<int> Desktop::getLastMouseDownPosition()
{
    return getInstance().getMainMouseSource().getLastMouseDownPosition();
}

int Desktop::getMouseButtonClickCounter()
{
    return getInstance().mouseClickCounter;
}

void Desktop::incrementMouseClickCounter() noexcept
{
    ++mouseClickCounter;
}

int Desktop::getNumDraggingMouseSources() const noexcept
{
    int num = 0;
    for (int i = mouseSources.size(); --i >= 0;)
        if (mouseSources.getUnchecked(i)->isDragging())
            ++num;

    return num;
}

MouseInputSource* Desktop::getDraggingMouseSource (int index) const noexcept
{
    int num = 0;
    for (int i = mouseSources.size(); --i >= 0;)
    {
        MouseInputSource* const mi = mouseSources.getUnchecked(i);

        if (mi->isDragging())
        {
            if (index == num)
                return mi;

            ++num;
        }
    }

    return nullptr;
}

//==============================================================================
class MouseDragAutoRepeater  : public Timer
{
public:
    MouseDragAutoRepeater() {}

    void timerCallback()
    {
        Desktop& desktop = Desktop::getInstance();
        int numMiceDown = 0;

        for (int i = desktop.getNumMouseSources(); --i >= 0;)
        {
            MouseInputSource* const source = desktop.getMouseSource(i);
            if (source->isDragging())
            {
                source->triggerFakeMove();
                ++numMiceDown;
            }
        }

        if (numMiceDown == 0)
            desktop.beginDragAutoRepeat (0);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (MouseDragAutoRepeater);
};

void Desktop::beginDragAutoRepeat (const int interval)
{
    if (interval > 0)
    {
        if (dragRepeater == nullptr)
            dragRepeater = new MouseDragAutoRepeater();

        if (dragRepeater->getTimerInterval() != interval)
            dragRepeater->startTimer (interval);
    }
    else
    {
        dragRepeater = nullptr;
    }
}

//==============================================================================
void Desktop::addFocusChangeListener (FocusChangeListener* const listener)
{
    focusListeners.add (listener);
}

void Desktop::removeFocusChangeListener (FocusChangeListener* const listener)
{
    focusListeners.remove (listener);
}

void Desktop::triggerFocusCallback()
{
    triggerAsyncUpdate();
}

void Desktop::handleAsyncUpdate()
{
    // The component may be deleted during this operation, but we'll use a SafePointer rather than a
    // BailOutChecker so that any remaining listeners will still get a callback (with a null pointer).
    WeakReference<Component> currentFocus (Component::getCurrentlyFocusedComponent());
    focusListeners.call (&FocusChangeListener::globalFocusChanged, currentFocus);
}

//==============================================================================
void Desktop::resetTimer()
{
    if (mouseListeners.size() == 0)
        stopTimer();
    else
        startTimer (100);

    lastFakeMouseMove = getMousePosition();
}

ListenerList <MouseListener>& Desktop::getMouseListeners()
{
    resetTimer();
    return mouseListeners;
}

void Desktop::addGlobalMouseListener (MouseListener* const listener)
{
    mouseListeners.add (listener);
    resetTimer();
}

void Desktop::removeGlobalMouseListener (MouseListener* const listener)
{
    mouseListeners.remove (listener);
    resetTimer();
}

void Desktop::timerCallback()
{
    if (lastFakeMouseMove != getMousePosition())
        sendMouseMove();
}

void Desktop::sendMouseMove()
{
    if (! mouseListeners.isEmpty())
    {
        startTimer (20);

        lastFakeMouseMove = getMousePosition();

        Component* const target = findComponentAt (lastFakeMouseMove);

        if (target != nullptr)
        {
            Component::BailOutChecker checker (target);
            const Point<int> pos (target->getLocalPoint (nullptr, lastFakeMouseMove));
            const Time now (Time::getCurrentTime());

            const MouseEvent me (getMainMouseSource(), pos, ModifierKeys::getCurrentModifiers(),
                                 target, target, now, pos, now, 0, false);

            if (me.mods.isAnyMouseButtonDown())
                mouseListeners.callChecked (checker, &MouseListener::mouseDrag, me);
            else
                mouseListeners.callChecked (checker, &MouseListener::mouseMove, me);
        }
    }
}

//==============================================================================
void Desktop::setKioskModeComponent (Component* componentToUse, const bool allowMenusAndBars)
{
    if (kioskModeComponent != componentToUse)
    {
        // agh! Don't delete or remove a component from the desktop while it's still the kiosk component!
        jassert (kioskModeComponent == nullptr || ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

        if (kioskModeComponent != nullptr)
        {
            setKioskComponent (kioskModeComponent, false, allowMenusAndBars);

            kioskModeComponent->setBounds (kioskComponentOriginalBounds);
        }

        kioskModeComponent = componentToUse;

        if (kioskModeComponent != nullptr)
        {
            // Only components that are already on the desktop can be put into kiosk mode!
            jassert (ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

            kioskComponentOriginalBounds = kioskModeComponent->getBounds();

            setKioskComponent (kioskModeComponent, true, allowMenusAndBars);
        }
    }
}

//==============================================================================
void Desktop::setOrientationsEnabled (const int newOrientations)
{
    // Dodgy set of flags being passed here! Make sure you specify at least one permitted orientation.
    jassert (newOrientations != 0 && (newOrientations & ~allOrientations) == 0);

    allowedOrientations = newOrientations;
}

bool Desktop::isOrientationEnabled (const DisplayOrientation orientation) const noexcept
{
    // Make sure you only pass one valid flag in here...
    jassert (orientation == upright || orientation == upsideDown || orientation == rotatedClockwise || orientation ==  rotatedAntiClockwise);

    return (allowedOrientations & orientation) != 0;
}
