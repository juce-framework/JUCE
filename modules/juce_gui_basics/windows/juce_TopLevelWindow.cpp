/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** Keeps track of the active top level window. */
class TopLevelWindowManager  : private Timer,
                               private DeletedAtShutdown
{
public:
    TopLevelWindowManager()     {}
    ~TopLevelWindowManager()    { clearSingletonInstance(); }

    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (TopLevelWindowManager)

    void checkFocusAsync()
    {
        startTimer (10);
    }

    void checkFocus()
    {
        startTimer (jmin (1731, getTimerInterval() * 2));

        auto* newActive = findCurrentlyActiveWindow();

        if (newActive != currentActive)
        {
            currentActive = newActive;

            for (int i = windows.size(); --i >= 0;)
                if (auto* tlw = windows[i])
                    tlw->setWindowActive (isWindowActive (tlw));

            Desktop::getInstance().triggerFocusCallback();
        }
    }

    bool addWindow (TopLevelWindow* const w)
    {
        windows.add (w);
        checkFocusAsync();

        return isWindowActive (w);
    }

    void removeWindow (TopLevelWindow* const w)
    {
        checkFocusAsync();

        if (currentActive == w)
            currentActive = nullptr;

        windows.removeFirstMatchingValue (w);

        if (windows.isEmpty())
            deleteInstance();
    }

    Array<TopLevelWindow*> windows;

private:
    TopLevelWindow* currentActive = nullptr;

    void timerCallback() override
    {
        checkFocus();
    }

    bool isWindowActive (TopLevelWindow* const tlw) const
    {
        return (tlw == currentActive
                 || tlw->isParentOf (currentActive)
                 || tlw->hasKeyboardFocus (true))
                && tlw->isShowing();
    }

    TopLevelWindow* findCurrentlyActiveWindow() const
    {
        if (Process::isForegroundProcess())
        {
            auto* focusedComp = Component::getCurrentlyFocusedComponent();
            auto* w = dynamic_cast<TopLevelWindow*> (focusedComp);

            if (w == nullptr && focusedComp != nullptr)
                w = focusedComp->findParentComponentOfClass<TopLevelWindow>();

            if (w == nullptr)
                w = currentActive;

            if (w != nullptr && w->isShowing())
                return w;
        }

        return nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE (TopLevelWindowManager)
};

JUCE_IMPLEMENT_SINGLETON (TopLevelWindowManager)

void juce_checkCurrentlyFocusedTopLevelWindow();
void juce_checkCurrentlyFocusedTopLevelWindow()
{
    if (auto* wm = TopLevelWindowManager::getInstanceWithoutCreating())
        wm->checkFocusAsync();
}

//==============================================================================
TopLevelWindow::TopLevelWindow (const String& name, const bool shouldAddToDesktop)
    : Component (name)
{
    setOpaque (true);

    if (shouldAddToDesktop)
        Component::addToDesktop (TopLevelWindow::getDesktopWindowStyleFlags());
    else
        setDropShadowEnabled (true);

    setWantsKeyboardFocus (true);
    setBroughtToFrontOnMouseClick (true);
    isCurrentlyActive = TopLevelWindowManager::getInstance()->addWindow (this);
}

TopLevelWindow::~TopLevelWindow()
{
    shadower.reset();
    TopLevelWindowManager::getInstance()->removeWindow (this);
}

//==============================================================================
void TopLevelWindow::focusOfChildComponentChanged (FocusChangeType)
{
    auto* wm = TopLevelWindowManager::getInstance();

    if (hasKeyboardFocus (true))
        wm->checkFocus();
    else
        wm->checkFocusAsync();
}

void TopLevelWindow::setWindowActive (const bool isNowActive)
{
    if (isCurrentlyActive != isNowActive)
    {
        isCurrentlyActive = isNowActive;
        activeWindowStatusChanged();
    }
}

void TopLevelWindow::activeWindowStatusChanged()
{
}

bool TopLevelWindow::isUsingNativeTitleBar() const noexcept
{
    return useNativeTitleBar && (isOnDesktop() || ! isShowing());
}

void TopLevelWindow::visibilityChanged()
{
    if (isShowing())
        if (auto* p = getPeer())
            if ((p->getStyleFlags() & (ComponentPeer::windowIsTemporary
                                        | ComponentPeer::windowIgnoresKeyPresses)) == 0)
                toFront (true);
}

void TopLevelWindow::parentHierarchyChanged()
{
    setDropShadowEnabled (useDropShadow);
}

int TopLevelWindow::getDesktopWindowStyleFlags() const
{
    int styleFlags = ComponentPeer::windowAppearsOnTaskbar;

    if (useDropShadow)       styleFlags |= ComponentPeer::windowHasDropShadow;
    if (useNativeTitleBar)   styleFlags |= ComponentPeer::windowHasTitleBar;

    return styleFlags;
}

void TopLevelWindow::setDropShadowEnabled (const bool useShadow)
{
    useDropShadow = useShadow;

    if (isOnDesktop())
    {
        shadower.reset();
        Component::addToDesktop (getDesktopWindowStyleFlags());
    }
    else
    {
        if (useShadow && isOpaque())
        {
            if (shadower == nullptr)
            {
                shadower.reset (getLookAndFeel().createDropShadowerForComponent (this));

                if (shadower != nullptr)
                    shadower->setOwner (this);
            }
        }
        else
        {
            shadower.reset();
        }
    }
}

void TopLevelWindow::setUsingNativeTitleBar (const bool shouldUseNativeTitleBar)
{
    if (useNativeTitleBar != shouldUseNativeTitleBar)
    {
        FocusRestorer focusRestorer;
        useNativeTitleBar = shouldUseNativeTitleBar;
        recreateDesktopWindow();
        sendLookAndFeelChange();
    }
}

void TopLevelWindow::recreateDesktopWindow()
{
    if (isOnDesktop())
    {
        Component::addToDesktop (getDesktopWindowStyleFlags());
        toFront (true);
    }
}

void TopLevelWindow::addToDesktop()
{
    shadower.reset();
    Component::addToDesktop (getDesktopWindowStyleFlags());
    setDropShadowEnabled (isDropShadowEnabled()); // force an update to clear away any fake shadows if necessary.
}

void TopLevelWindow::addToDesktop (int windowStyleFlags, void* nativeWindowToAttachTo)
{
    /* It's not recommended to change the desktop window flags directly for a TopLevelWindow,
       because this class needs to make sure its layout corresponds with settings like whether
       it's got a native title bar or not.

       If you need custom flags for your window, you can override the getDesktopWindowStyleFlags()
       method. If you do this, it's best to call the base class's getDesktopWindowStyleFlags()
       method, then add or remove whatever flags are necessary from this value before returning it.
    */

    jassert ((windowStyleFlags & ~ComponentPeer::windowIsSemiTransparent)
               == (getDesktopWindowStyleFlags() & ~ComponentPeer::windowIsSemiTransparent));

    Component::addToDesktop (windowStyleFlags, nativeWindowToAttachTo);

    if (windowStyleFlags != getDesktopWindowStyleFlags())
        sendLookAndFeelChange();
}

//==============================================================================
void TopLevelWindow::centreAroundComponent (Component* c, const int width, const int height)
{
    if (c == nullptr)
        c = TopLevelWindow::getActiveTopLevelWindow();

    if (c == nullptr || c->getBounds().isEmpty())
    {
        centreWithSize (width, height);
    }
    else
    {
        auto targetCentre = c->localPointToGlobal (c->getLocalBounds().getCentre());
        auto parentArea = c->getParentMonitorArea();

        if (auto* parent = getParentComponent())
        {
            targetCentre = parent->getLocalPoint (nullptr, targetCentre);
            parentArea   = parent->getLocalBounds();
        }

        setBounds (Rectangle<int> (targetCentre.x - width / 2,
                                   targetCentre.y - height / 2,
                                   width, height)
                     .constrainedWithin (parentArea.reduced (12, 12)));
    }
}

//==============================================================================
int TopLevelWindow::getNumTopLevelWindows() noexcept
{
    return TopLevelWindowManager::getInstance()->windows.size();
}

TopLevelWindow* TopLevelWindow::getTopLevelWindow (const int index) noexcept
{
    return TopLevelWindowManager::getInstance()->windows [index];
}

TopLevelWindow* TopLevelWindow::getActiveTopLevelWindow() noexcept
{
    TopLevelWindow* best = nullptr;
    int bestNumTWLParents = -1;

    for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
    {
        auto* tlw = TopLevelWindow::getTopLevelWindow (i);

        if (tlw->isActiveWindow())
        {
            int numTWLParents = 0;

            for (auto* c = tlw->getParentComponent(); c != nullptr; c = c->getParentComponent())
                if (dynamic_cast<const TopLevelWindow*> (c) != nullptr)
                    ++numTWLParents;

            if (bestNumTWLParents < numTWLParents)
            {
                best = tlw;
                bestNumTWLParents = numTWLParents;
            }
        }
    }

    return best;
}

} // namespace juce
