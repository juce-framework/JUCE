/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

Desktop::Desktop()
    : mouseSources (new detail::MouseInputSourceList()),
      masterScaleFactor ((float) getDefaultMasterScale()),
      nativeDarkModeChangeDetectorImpl (createNativeDarkModeChangeDetectorImpl())
{
    displays.reset (new Displays (*this));
}

Desktop::~Desktop()
{
    setScreenSaverEnabled (true);
    animator.cancelAllAnimations (false);

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
int Desktop::getNumComponents() const noexcept
{
    return desktopComponents.size();
}

Component* Desktop::getComponent (int index) const noexcept
{
    return desktopComponents [index];
}

Component* Desktop::findComponentAt (Point<int> screenPosition) const
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    for (int i = desktopComponents.size(); --i >= 0;)
    {
        auto* c = desktopComponents.getUnchecked (i);

        if (c->isVisible())
        {
            auto relative = c->getLocalPoint (nullptr, screenPosition);

            if (c->contains (relative))
                return c->getComponentAt (relative);
        }
    }

    return nullptr;
}

//==============================================================================
LookAndFeel& Desktop::getDefaultLookAndFeel() noexcept
{
    if (auto lf = currentLookAndFeel.get())
        return *lf;

    if (defaultLookAndFeel == nullptr)
        defaultLookAndFeel.reset (new LookAndFeel_V4());

    auto lf = defaultLookAndFeel.get();
    jassert (lf != nullptr);
    currentLookAndFeel = lf;
    return *lf;
}

void Desktop::setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel)
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    currentLookAndFeel = newDefaultLookAndFeel;

    for (int i = getNumComponents(); --i >= 0;)
        if (auto* c = getComponent (i))
            c->sendLookAndFeelChange();
}

//==============================================================================
void Desktop::addDesktopComponent (Component* c)
{
    jassert (c != nullptr);
    jassert (! desktopComponents.contains (c));
    desktopComponents.addIfNotAlreadyThere (c);
}

void Desktop::removeDesktopComponent (Component* c)
{
    desktopComponents.removeFirstMatchingValue (c);
}

void Desktop::componentBroughtToFront (Component* c)
{
    auto index = desktopComponents.indexOf (c);
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
    return getMousePositionFloat().roundToInt();
}

Point<float> Desktop::getMousePositionFloat()
{
    return getInstance().getMainMouseSource().getScreenPosition();
}

void Desktop::setMousePosition (Point<int> newPosition)
{
    getInstance().getMainMouseSource().setScreenPosition (newPosition.toFloat());
}

Point<int> Desktop::getLastMouseDownPosition()
{
    return getInstance().getMainMouseSource().getLastMouseDownPosition().roundToInt();
}

int Desktop::getMouseButtonClickCounter() const noexcept    { return mouseClickCounter; }
int Desktop::getMouseWheelMoveCounter() const noexcept      { return mouseWheelCounter; }

void Desktop::incrementMouseClickCounter() noexcept         { ++mouseClickCounter; }
void Desktop::incrementMouseWheelCounter() noexcept         { ++mouseWheelCounter; }

const Array<MouseInputSource>& Desktop::getMouseSources() const noexcept        { return mouseSources->sourceArray; }
int Desktop::getNumMouseSources() const noexcept                                { return mouseSources->sources.size(); }
int Desktop::getNumDraggingMouseSources() const noexcept                        { return mouseSources->getNumDraggingMouseSources(); }
MouseInputSource* Desktop::getMouseSource (int index) const noexcept            { return mouseSources->getMouseSource (index); }
MouseInputSource* Desktop::getDraggingMouseSource (int index) const noexcept    { return mouseSources->getDraggingMouseSource (index); }
MouseInputSource Desktop::getMainMouseSource() const noexcept                   { return MouseInputSource (mouseSources->sources.getUnchecked (0)); }
void Desktop::beginDragAutoRepeat (int interval)                                { mouseSources->beginDragAutoRepeat (interval); }

//==============================================================================
void Desktop::addFocusChangeListener    (FocusChangeListener* l)   { focusListeners.add (l); }
void Desktop::removeFocusChangeListener (FocusChangeListener* l)   { focusListeners.remove (l); }
void Desktop::triggerFocusCallback()                               { triggerAsyncUpdate(); }

void Desktop::updateFocusOutline()
{
    if (auto* currentFocus = Component::getCurrentlyFocusedComponent())
    {
        if (currentFocus->hasFocusOutline())
        {
            focusOutline = currentFocus->getLookAndFeel().createFocusOutlineForComponent (*currentFocus);

            if (focusOutline != nullptr)
                focusOutline->setOwner (currentFocus);

            return;
        }
    }

    focusOutline = nullptr;
}

void Desktop::handleAsyncUpdate()
{
    // The component may be deleted during this operation, but we'll use a SafePointer rather than a
    // BailOutChecker so that any remaining listeners will still get a callback (with a null pointer).
    focusListeners.call ([currentFocus = WeakReference<Component> { Component::getCurrentlyFocusedComponent() }] (FocusChangeListener& l)
    {
        l.globalFocusChanged (currentFocus.get());
    });

    updateFocusOutline();
}

//==============================================================================
void Desktop::addDarkModeSettingListener    (DarkModeSettingListener* l)  { darkModeSettingListeners.add (l); }
void Desktop::removeDarkModeSettingListener (DarkModeSettingListener* l)  { darkModeSettingListeners.remove (l); }

void Desktop::darkModeChanged()  { darkModeSettingListeners.call ([] (auto& l) { l.darkModeSettingChanged(); }); }

//==============================================================================
void Desktop::resetTimer()
{
    if (mouseListeners.size() == 0)
        stopTimer();
    else
        startTimer (100);

    lastFakeMouseMove = getMousePositionFloat();
}

ListenerList<MouseListener>& Desktop::getMouseListeners()
{
    resetTimer();
    return mouseListeners;
}

void Desktop::addGlobalMouseListener (MouseListener* listener)
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    mouseListeners.add (listener);
    resetTimer();
}

void Desktop::removeGlobalMouseListener (MouseListener* listener)
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED
    mouseListeners.remove (listener);
    resetTimer();
}

void Desktop::timerCallback()
{
    if (lastFakeMouseMove != getMousePositionFloat())
        sendMouseMove();
}

void Desktop::sendMouseMove()
{
    if (! mouseListeners.isEmpty())
    {
        startTimer (20);

        lastFakeMouseMove = getMousePositionFloat();

        if (auto* target = findComponentAt (lastFakeMouseMove.roundToInt()))
        {
            Component::BailOutChecker checker (target);
            auto pos = target->getLocalPoint (nullptr, lastFakeMouseMove);
            auto now = Time::getCurrentTime();

            const MouseEvent me (getMainMouseSource(), pos, ModifierKeys::currentModifiers, MouseInputSource::defaultPressure,
                                 MouseInputSource::defaultOrientation, MouseInputSource::defaultRotation,
                                 MouseInputSource::defaultTiltX, MouseInputSource::defaultTiltY,
                                 target, target, now, pos, now, 0, false);

            if (me.mods.isAnyMouseButtonDown())
                mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseDrag (me); });
            else
                mouseListeners.callChecked (checker, [&] (MouseListener& l) { l.mouseMove (me); });
        }
    }
}

//==============================================================================
void Desktop::setKioskModeComponent (Component* componentToUse, bool allowMenusAndBars)
{
    if (kioskModeReentrant)
        return;

    const ScopedValueSetter<bool> setter (kioskModeReentrant, true, false);

    if (kioskModeComponent != componentToUse)
    {
        // agh! Don't delete or remove a component from the desktop while it's still the kiosk component!
        jassert (kioskModeComponent == nullptr || ComponentPeer::getPeerFor (kioskModeComponent) != nullptr);

        if (auto* oldKioskComp = kioskModeComponent)
        {
            kioskModeComponent = nullptr; // (to make sure that isKioskMode() returns false when resizing the old one)
            setKioskComponent (oldKioskComp, false, allowMenusAndBars);
            oldKioskComp->setBounds (kioskComponentOriginalBounds);
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
void Desktop::setOrientationsEnabled (int newOrientations)
{
    if (allowedOrientations != newOrientations)
    {
        // Dodgy set of flags being passed here! Make sure you specify at least one permitted orientation.
        jassert (newOrientations != 0 && (newOrientations & ~allOrientations) == 0);

        allowedOrientations = newOrientations;
        allowedOrientationsChanged();
    }
}

int Desktop::getOrientationsEnabled() const noexcept
{
    return allowedOrientations;
}

bool Desktop::isOrientationEnabled (DisplayOrientation orientation) const noexcept
{
    // Make sure you only pass one valid flag in here...
    jassert (orientation == upright || orientation == upsideDown
              || orientation == rotatedClockwise || orientation == rotatedAntiClockwise);

    return (allowedOrientations & orientation) != 0;
}

void Desktop::setGlobalScaleFactor (float newScaleFactor) noexcept
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    if (! approximatelyEqual (masterScaleFactor, newScaleFactor))
    {
        masterScaleFactor = newScaleFactor;
        displays->refresh();
    }
}

bool Desktop::isHeadless() const noexcept
{
    return displays->displays.isEmpty();
}

bool Desktop::supportsBorderlessNonClientResize() const
{
   #if JUCE_WINDOWS || JUCE_MAC
    return true;
   #else
    return false;
   #endif
}

} // namespace juce
