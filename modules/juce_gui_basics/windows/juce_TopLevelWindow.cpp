/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

/** Keeps track of the active top level window. */
class TopLevelWindowManager  : private Timer,
                               private DeletedAtShutdown
{
public:
    //==============================================================================
    TopLevelWindowManager()  : currentActive (nullptr)
    {
    }

    ~TopLevelWindowManager()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (TopLevelWindowManager);

    void checkFocusAsync()
    {
        startTimer (10);
    }

    void checkFocus()
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
        checkFocusAsync();

        return isWindowActive (w);
    }

    void removeWindow (TopLevelWindow* const w)
    {
        checkFocusAsync();

        if (currentActive == w)
            currentActive = nullptr;

        windows.removeFirstMatchingValue (w);

        if (windows.size() == 0)
            deleteInstance();
    }

    Array <TopLevelWindow*> windows;

private:
    TopLevelWindow* currentActive;

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

    JUCE_DECLARE_NON_COPYABLE (TopLevelWindowManager)
};

juce_ImplementSingleton_SingleThreaded (TopLevelWindowManager)

void juce_checkCurrentlyFocusedTopLevelWindow();
void juce_checkCurrentlyFocusedTopLevelWindow()
{
    if (TopLevelWindowManager* const wm = TopLevelWindowManager::getInstanceWithoutCreating())
        wm->checkFocusAsync();
}

//==============================================================================
TopLevelWindow::TopLevelWindow (const String& name, const bool shouldAddToDesktop)
    : Component (name),
      useDropShadow (true),
      useNativeTitleBar (false),
      isCurrentlyActive (false)
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
    shadower = nullptr;
    TopLevelWindowManager::getInstance()->removeWindow (this);
}

//==============================================================================
void TopLevelWindow::focusOfChildComponentChanged (FocusChangeType)
{
    TopLevelWindowManager* const wm = TopLevelWindowManager::getInstance();

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

    if (useDropShadow)       styleFlags |= ComponentPeer::windowHasDropShadow;
    if (useNativeTitleBar)   styleFlags |= ComponentPeer::windowHasTitleBar;

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

void TopLevelWindow::setUsingNativeTitleBar (const bool shouldUseNativeTitleBar)
{
    if (useNativeTitleBar != shouldUseNativeTitleBar)
    {
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
    shadower = nullptr;
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
        Point<int> targetCentre (c->localPointToGlobal (c->getLocalBounds().getCentre()));
        Rectangle<int> parentArea (c->getParentMonitorArea());

        if (Component* const parent = getParentComponent())
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
        TopLevelWindow* const tlw = TopLevelWindow::getTopLevelWindow (i);

        if (tlw->isActiveWindow())
        {
            int numTWLParents = 0;

            for (const Component* c = tlw->getParentComponent(); c != nullptr; c = c->getParentComponent())
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
