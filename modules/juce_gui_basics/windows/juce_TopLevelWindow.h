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

#ifndef __JUCE_TOPLEVELWINDOW_JUCEHEADER__
#define __JUCE_TOPLEVELWINDOW_JUCEHEADER__

#include "../components/juce_Component.h"
#include "../misc/juce_DropShadower.h"


//==============================================================================
/**
    A base class for top-level windows.

    This class is used for components that are considered a major part of your
    application - e.g. ResizableWindow, DocumentWindow, DialogWindow, AlertWindow,
    etc. Things like menus that pop up briefly aren't derived from it.

    A TopLevelWindow is probably on the desktop, but this isn't mandatory - it
    could itself be the child of another component.

    The class manages a list of all instances of top-level windows that are in use,
    and each one is also given the concept of being "active". The active window is
    one that is actively being used by the user. This isn't quite the same as the
    component with the keyboard focus, because there may be a popup menu or other
    temporary window which gets keyboard focus while the active top level window is
    unchanged.

    A top-level window also has an optional drop-shadow.

    @see ResizableWindow, DocumentWindow, DialogWindow
*/
class JUCE_API  TopLevelWindow  : public Component
{
public:
    //==============================================================================
    /** Creates a TopLevelWindow.

        @param name                 the name to give the component
        @param addToDesktop         if true, the window will be automatically added to the
                                    desktop; if false, you can use it as a child component
    */
    TopLevelWindow (const String& name, bool addToDesktop);

    /** Destructor. */
    ~TopLevelWindow();

    //==============================================================================
    /** True if this is currently the TopLevelWindow that is actively being used.

        This isn't quite the same as having keyboard focus, because the focus may be
        on a child component or a temporary pop-up menu, etc, while this window is
        still considered to be active.

        @see activeWindowStatusChanged
    */
    bool isActiveWindow() const noexcept                    { return isCurrentlyActive; }

    //==============================================================================
    /** This will set the bounds of the window so that it's centred in front of another
        window.

        If your app has a few windows open and want to pop up a dialog box for one of
        them, you can use this to show it in front of the relevent parent window, which
        is a bit neater than just having it appear in the middle of the screen.

        If componentToCentreAround is 0, then the currently active TopLevelWindow will
        be used instead. If no window is focused, it'll just default to the middle of the
        screen.
    */
    void centreAroundComponent (Component* componentToCentreAround,
                                int width, int height);

    //==============================================================================
    /** Turns the drop-shadow on and off. */
    void setDropShadowEnabled (bool useShadow);

    /** True if drop-shadowing is enabled. */
    bool isDropShadowEnabled() const noexcept               { return useDropShadow; }

    /** Sets whether an OS-native title bar will be used, or a Juce one.
        @see isUsingNativeTitleBar
    */
    void setUsingNativeTitleBar (bool useNativeTitleBar);

    /** Returns true if the window is currently using an OS-native title bar.
        @see setUsingNativeTitleBar
    */
    bool isUsingNativeTitleBar() const noexcept;

    //==============================================================================
    /** Returns the number of TopLevelWindow objects currently in use.
        @see getTopLevelWindow
    */
    static int getNumTopLevelWindows() noexcept;

    /** Returns one of the TopLevelWindow objects currently in use.
        The index is 0 to (getNumTopLevelWindows() - 1).
    */
    static TopLevelWindow* getTopLevelWindow (int index) noexcept;

    /** Returns the currently-active top level window.
        There might not be one, of course, so this can return nullptr.
    */
    static TopLevelWindow* getActiveTopLevelWindow() noexcept;


    //==============================================================================
    /** @internal */
    virtual void addToDesktop (int windowStyleFlags, void* nativeWindowToAttachTo = nullptr);

protected:
    //==============================================================================
    /** This callback happens when this window becomes active or inactive.
        @see isActiveWindow
    */
    virtual void activeWindowStatusChanged();


    //==============================================================================
    /** @internal */
    void focusOfChildComponentChanged (FocusChangeType cause);
    /** @internal */
    void parentHierarchyChanged();
    /** @internal */
    virtual int getDesktopWindowStyleFlags() const;
    /** @internal */
    void recreateDesktopWindow();
    /** @internal */
    void visibilityChanged();

private:
    friend class TopLevelWindowManager;
    bool useDropShadow, useNativeTitleBar, isCurrentlyActive;
    ScopedPointer <DropShadower> shadower;

    void setWindowActive (bool isNowActive);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopLevelWindow)
};


#endif   // __JUCE_TOPLEVELWINDOW_JUCEHEADER__
