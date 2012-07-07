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

/** Keeps track of the active top level window. */
class TopLevelWindowManager  : public Timer,
                               public DeletedAtShutdown
{
public:
    //==============================================================================
    TopLevelWindowManager()
        : currentActive (nullptr)
    {
    }

    ~TopLevelWindowManager()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (TopLevelWindowManager);

    void timerCallback()
    {
        startTimer (jmin (1731, getTimerInterval() * 2));

        TopLevelWindow* active = nullptr;

        if (Process::isForegroundProcess())
        {
            active = currentActive;

            Component* const c = Component::getCurrentlyFocusedComponent();
            TopLevelWindow* tlw = dynamic_cast <TopLevelWindow*> (c);

            if (tlw == nullptr && c != nullptr)
                tlw = c->findParentComponentOfClass<TopLevelWindow>();

            if (tlw != nullptr)
                active = tlw;
        }

        if (active != currentActive)
        {
            currentActive = active;

            for (int i = windows.size(); --i >= 0;)
            {
                TopLevelWindow* const tlw = windows.getUnchecked (i);
                tlw->setWindowActive (isWindowActive (tlw));

                i = jmin (i, windows.size() - 1);
            }

            Desktop::getInstance().triggerFocusCallback();
        }
    }

    bool addWindow (TopLevelWindow* const w)
    {
        windows.add (w);
        startTimer (10);

        return isWindowActive (w);
    }

    void removeWindow (TopLevelWindow* const w)
    {
        startTimer (10);

        if (currentActive == w)
            currentActive = nullptr;

        windows.removeFirstMatchingValue (w);

        if (windows.size() == 0)
            deleteInstance();
    }

    Array <TopLevelWindow*> windows;

private:
    TopLevelWindow* currentActive;

    bool isWindowActive (TopLevelWindow* const tlw) const
    {
        return (tlw == currentActive
                 || tlw->isParentOf (currentActive)
                 || tlw->hasKeyboardFocus (true))
                && tlw->isShowing();
    }

    JUCE_DECLARE_NON_COPYABLE (TopLevelWindowManager);
};

juce_ImplementSingleton_SingleThreaded (TopLevelWindowManager)

void juce_CheckCurrentlyFocusedTopLevelWindow();
void juce_CheckCurrentlyFocusedTopLevelWindow()
{
    if (TopLevelWindowManager::getInstanceWithoutCreating() != nullptr)
        TopLevelWindowManager::getInstanceWithoutCreating()->startTimer (20);
}

//==============================================================================
TopLevelWindow::TopLevelWindow (const String& name,
                                const bool addToDesktop_)
    : Component (name),
      useDropShadow (true),
      useNativeTitleBar (false),
      windowIsActive_ (false)
{
    setOpaque (true);

    if (addToDesktop_)
        Component::addToDesktop (TopLevelWindow::getDesktopWindowStyleFlags());
    else
        setDropShadowEnabled (true);

    setWantsKeyboardFocus (true);
    setBroughtToFrontOnMouseClick (true);
    windowIsActive_ = TopLevelWindowManager::getInstance()->addWindow (this);
}

TopLevelWindow::~TopLevelWindow()
{
    shadower = nullptr;
    TopLevelWindowManager::getInstance()->removeWindow (this);
}

//==============================================================================
void TopLevelWindow::focusOfChildComponentChanged (FocusChangeType)
{
    if (hasKeyboardFocus (true))
        TopLevelWindowManager::getInstance()->timerCallback();
    else
        TopLevelWindowManager::getInstance()->startTimer (10);
}

void TopLevelWindow::setWindowActive (const bool isNowActive)
{
    if (windowIsActive_ != isNowActive)
    {
        windowIsActive_ = isNowActive;
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
    if (isShowing()
         && (getPeer()->getStyleFlags() & (ComponentPeer::windowIsTemporary
                                            | ComponentPeer::windowIgnoresKeyPresses)) == 0)
    {
        toFront (true);
    }
}

void TopLevelWindow::parentHierarchyChanged()
{
    setDropShadowEnabled (useDropShadow);
}

int TopLevelWindow::getDesktopWindowStyleFlags() const
{
    int styleFlags = ComponentPeer::windowAppearsOnTaskbar;

    if (useDropShadow)
        styleFlags |= ComponentPeer::windowHasDropShadow;

    if (useNativeTitleBar)
        styleFlags |= ComponentPeer::windowHasTitleBar;

    return styleFlags;
}

void TopLevelWindow::setDropShadowEnabled (const bool useShadow)
{
    useDropShadow = useShadow;

    if (isOnDesktop())
    {
        shadower = nullptr;
        Component::addToDesktop (getDesktopWindowStyleFlags());
    }
    else
    {
        if (useShadow && isOpaque())
        {
            if (shadower == nullptr)
            {
                shadower = getLookAndFeel().createDropShadowerForComponent (this);

                if (shadower != nullptr)
                    shadower->setOwner (this);
            }
        }
        else
        {
            shadower = nullptr;
        }
    }
}

void TopLevelWindow::setUsingNativeTitleBar (const bool useNativeTitleBar_)
{
    if (useNativeTitleBar != useNativeTitleBar_)
    {
        useNativeTitleBar = useNativeTitleBar_;
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
        Point<int> targetCentre (c->localPointToGlobal (c->getLocalBounds().getCentre()));
        Rectangle<int> parentArea (c->getParentMonitorArea());

        if (getParentComponent() != nullptr)
        {
            targetCentre = getParentComponent()->getLocalPoint (nullptr, targetCentre);
            parentArea = getParentComponent()->getLocalBounds();
        }

        parentArea.reduce (12, 12);

        setBounds (jlimit (parentArea.getX(), jmax (parentArea.getX(), parentArea.getRight() - width), targetCentre.getX() - width / 2),
                   jlimit (parentArea.getY(), jmax (parentArea.getY(), parentArea.getBottom() - height), targetCentre.getY() - height / 2),
                   width, height);
    }
}

//==============================================================================
int TopLevelWindow::getNumTopLevelWindows() noexcept
{
    return TopLevelWindowManager::getInstance()->windows.size();
}

TopLevelWindow* TopLevelWindow::getTopLevelWindow (const int index) noexcept
{
    return static_cast <TopLevelWindow*> (TopLevelWindowManager::getInstance()->windows [index]);
}

TopLevelWindow* TopLevelWindow::getActiveTopLevelWindow() noexcept
{
    TopLevelWindow* best = nullptr;
    int bestNumTWLParents = -1;

    for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
    {
        TopLevelWindow* const tlw = TopLevelWindow::getTopLevelWindow (i);

        if (tlw->isActiveWindow())
        {
            int numTWLParents = 0;

            const Component* c = tlw->getParentComponent();

            while (c != nullptr)
            {
                if (dynamic_cast <const TopLevelWindow*> (c) != nullptr)
                    ++numTWLParents;

                c = c->getParentComponent();
            }

            if (bestNumTWLParents < numTWLParents)
            {
                best = tlw;
                bestNumTWLParents = numTWLParents;
            }
        }
    }

    return best;
}
