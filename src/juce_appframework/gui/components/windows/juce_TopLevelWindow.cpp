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


#include "juce_TopLevelWindow.h"
#include "../juce_Desktop.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../special/juce_DropShadower.h"
#include "../../../../juce_core/threads/juce_Process.h"
#include "../../../../juce_core/basics/juce_Singleton.h"


//==============================================================================
/** Keeps track of the active top level window.
*/
class TopLevelWindowManager  : public Timer,
                               public DeletedAtShutdown
{
public:
    //==============================================================================
    TopLevelWindowManager()
        : windows (8),
          currentActive (0)
    {
    }

    ~TopLevelWindowManager()
    {
        clearSingletonInstance();
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (TopLevelWindowManager)

    void timerCallback()
    {
        startTimer (1731);
        TopLevelWindow* active = 0;

        if (Process::isForegroundProcess())
        {
            active = currentActive;

            Component* const c = Component::getCurrentlyFocusedComponent();

            TopLevelWindow* tlw = dynamic_cast <TopLevelWindow*> (c);

            if (tlw == 0 && c != 0)
                // (unable to use the syntax findParentComponentOfClass <TopLevelWindow> () because of a VC6 compiler bug)
                tlw = c->findParentComponentOfClass ((TopLevelWindow*) 0);

            if (tlw != 0)
                active = tlw;
        }

        if (active != currentActive)
        {
            currentActive = active;

            for (int i = windows.size(); --i >= 0;)
            {
                TopLevelWindow* const tlw = (TopLevelWindow*) windows.getUnchecked (i);
                tlw->setWindowActive (isWindowActive (tlw));

                i = jmin (i, windows.size() - 1);
            }

            Desktop::getInstance().triggerFocusCallback();
        }
    }

    bool addWindow (TopLevelWindow* const w) throw()
    {
        windows.add (w);
        startTimer (10);

        return isWindowActive (w);
    }

    void removeWindow (TopLevelWindow* const w) throw()
    {
        startTimer (10);

        if (currentActive == w)
            currentActive = 0;

        windows.removeValue (w);

        if (windows.size() == 0)
            deleteInstance();
    }

    VoidArray windows;

private:
    TopLevelWindow* currentActive;

    bool isWindowActive (TopLevelWindow* const tlw) const throw()
    {
        return (tlw == currentActive
                 || tlw->isParentOf (currentActive)
                 || tlw->hasKeyboardFocus (true))
                && tlw->isShowing();
    }

    TopLevelWindowManager (const TopLevelWindowManager&);
    const TopLevelWindowManager& operator= (const TopLevelWindowManager&);
};

juce_ImplementSingleton_SingleThreaded (TopLevelWindowManager)

void juce_CheckCurrentlyFocusedTopLevelWindow() throw()
{
    if (TopLevelWindowManager::getInstanceWithoutCreating() != 0)
        TopLevelWindowManager::getInstanceWithoutCreating()->startTimer (20);
}

//==============================================================================
TopLevelWindow::TopLevelWindow (const String& name,
                                const bool addToDesktop_)
    : Component (name),
      useDropShadow (true),
      useNativeTitleBar (false),
      windowIsActive_ (false),
      shadower (0)
{
    setOpaque (true);

    if (addToDesktop_)
        Component::addToDesktop (getDesktopWindowStyleFlags());
    else
        setDropShadowEnabled (true);

    setWantsKeyboardFocus (true);
    setBroughtToFrontOnMouseClick (true);
    windowIsActive_ = TopLevelWindowManager::getInstance()->addWindow (this);
}

TopLevelWindow::~TopLevelWindow()
{
    deleteAndZero (shadower);
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

void TopLevelWindow::setWindowActive (const bool isNowActive) throw()
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

void TopLevelWindow::parentHierarchyChanged()
{
    setDropShadowEnabled (useDropShadow);
}

void TopLevelWindow::visibilityChanged()
{
    if (isShowing())
        toFront (true);
}

int TopLevelWindow::getDesktopWindowStyleFlags() const
{
    int flags = ComponentPeer::windowAppearsOnTaskbar;

    if (useDropShadow)
        flags |= ComponentPeer::windowHasDropShadow;

    if (useNativeTitleBar)
        flags |= ComponentPeer::windowHasTitleBar;

    return flags;
}

void TopLevelWindow::setDropShadowEnabled (const bool useShadow)
{
    useDropShadow = useShadow;

    if (isOnDesktop())
    {
        deleteAndZero (shadower);
        Component::addToDesktop (getDesktopWindowStyleFlags());
    }
    else
    {
        if (useShadow && isOpaque())
        {
            if (shadower == 0)
            {
                shadower = getLookAndFeel().createDropShadowerForComponent (this);

                if (shadower != 0)
                    shadower->setOwner (this);
            }
        }
        else
        {
            deleteAndZero (shadower);
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

    jassert (windowStyleFlags == getDesktopWindowStyleFlags());

    Component::addToDesktop (windowStyleFlags, nativeWindowToAttachTo);

    if (windowStyleFlags != getDesktopWindowStyleFlags())
        sendLookAndFeelChange();
}

//==============================================================================
void TopLevelWindow::centreAroundComponent (Component* c, const int width, const int height)
{
    if (c == 0)
    {
        if (c == 0)
            c = TopLevelWindow::getActiveTopLevelWindow();

        if (c == 0)
        {
            c = Component::getCurrentlyFocusedComponent();

            if (c != 0)
                c = c->getTopLevelComponent();
        }
    }
    else
    {
        c = c->getTopLevelComponent();
    }

    if (c == 0)
    {
        centreWithSize (width, height);
    }
    else
    {
        int cx = c->getWidth() / 2;
        int cy = c->getHeight() / 2;
        c->relativePositionToGlobal (cx, cy);

        int x = cx - width / 2;
        int y = cy - height / 2;

        if (x <= cx
            && y <= cy
            && x + width >= cx + c->getWidth()
            && y + height >= cy + c->getHeight())
        {
            cx = 20;
            cy = 20;
            c->relativePositionToGlobal (cx, cy);
        }

        if (getParentComponent() != 0)
        {
            getParentComponent()->globalPositionToRelative (x, y);

            setBounds (jlimit (0, jmax (0, getParentWidth() - width), x),
                       jlimit (0, jmax (0, getParentHeight() - height), y),
                       width, height);
        }
        else
        {
            const Rectangle screenArea (getParentMonitorArea());

            setBounds (jlimit (screenArea.getX(), jmax (screenArea.getX(), screenArea.getWidth() - width), x),
                       jlimit (screenArea.getY(), jmax (screenArea.getY(), screenArea.getHeight() - height), y),
                       width, height);
        }
    }
}

//==============================================================================
int TopLevelWindow::getNumTopLevelWindows() throw()
{
    return TopLevelWindowManager::getInstance()->windows.size();
}

TopLevelWindow* TopLevelWindow::getTopLevelWindow (const int index) throw()
{
    return (TopLevelWindow*) TopLevelWindowManager::getInstance()->windows [index];
}

TopLevelWindow* TopLevelWindow::getActiveTopLevelWindow() throw()
{
    TopLevelWindow* best = 0;
    int bestNumTWLParents = -1;

    for (int i = TopLevelWindow::getNumTopLevelWindows(); --i >= 0;)
    {
        TopLevelWindow* const tlw = TopLevelWindow::getTopLevelWindow (i);

        if (tlw->isActiveWindow())
        {
            int numTWLParents = 0;

            const Component* c = tlw->getParentComponent();

            while (c != 0)
            {
                if (dynamic_cast <const TopLevelWindow*> (c) != 0)
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


END_JUCE_NAMESPACE
