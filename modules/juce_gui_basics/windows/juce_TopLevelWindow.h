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

    @tags{GUI}
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
    ~TopLevelWindow() override;

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
        them, you can use this to show it in front of the relevant parent window, which
        is a bit neater than just having it appear in the middle of the screen.

        If componentToCentreAround is nullptr, then the currently active TopLevelWindow will
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

    /** Sets whether an OS-native title bar will be used, or a JUCE one.
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

    /** Adds the window to the desktop using the default flags. */
    void addToDesktop();

    //==============================================================================
    /** @internal */
    void addToDesktop (int windowStyleFlags, void* nativeWindowToAttachTo = nullptr) override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

protected:
    //==============================================================================
    /** This callback happens when this window becomes active or inactive.
        @see isActiveWindow
    */
    virtual void activeWindowStatusChanged();


    //==============================================================================
    /** @internal */
    void focusOfChildComponentChanged (FocusChangeType) override;
    /** @internal */
    void parentHierarchyChanged() override;
    /** @internal */
    virtual int getDesktopWindowStyleFlags() const;
    /** @internal */
    void recreateDesktopWindow();
    /** @internal */
    void visibilityChanged() override;

private:
    friend class detail::TopLevelWindowManager;
    friend class ResizableWindow;
    bool useDropShadow = true, useNativeTitleBar = false, isCurrentlyActive = false;
    std::unique_ptr<DropShadower> shadower;

    void setWindowActive (bool);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopLevelWindow)
};

} // namespace juce
