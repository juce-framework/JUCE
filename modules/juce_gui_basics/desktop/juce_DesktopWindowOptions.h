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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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
    Describes the desktop window that Component::addToDesktop() should create for a component.

    @see Component::addToDesktop

    @tags{GUI}
*/
class JUCE_API  DesktopWindowOptions  final
{
public:
    /** Sets the window's style flags, a combination of the values in ComponentPeer::StyleFlags. */
    [[nodiscard]] DesktopWindowOptions withStyleFlags (int newFlags) const                  { return withMember (*this, &DesktopWindowOptions::styleFlags, newFlags); }

    /** Sets a UIView in which the component should place itself, instead of creating a
        window of its own. This is used on iOS and has no effect on other platforms.
    */
    [[nodiscard]] DesktopWindowOptions withParentUIView (void* view) const                  { return withParent<UIViewParent> (view); }

    /** Sets an Android Activity whose content view the component should become, instead of
        creating a window of its own. This is used on Android and has no effect on other
        platforms.
    */
    [[nodiscard]] DesktopWindowOptions withParentAndroidActivity (void* activity) const     { return withParent<AndroidActivityParent> (activity); }

    /** Sets an Android ViewGroup in which the component should place itself, instead of
        creating a window of its own. This is used on Android and has no effect on other
        platforms.
    */
    [[nodiscard]] DesktopWindowOptions withParentAndroidViewGroup (void* viewGroup) const   { return withParent<AndroidViewGroupParent> (viewGroup); }

    /** Sets an NSView in which the component should place itself, instead of creating a
        window of its own. This is used on macOS and has no effect on other platforms.
    */
    [[nodiscard]] DesktopWindowOptions withParentNSView (void* view) const                  { return withParent<NSViewParent> (view); }

    /** Sets a HWND in which the component should place itself as a child window, instead
        of creating a top-level window. This is used on Windows and has no effect on other
        platforms.
    */
    [[nodiscard]] DesktopWindowOptions withParentHWND (void* hwnd) const                    { return withParent<HWNDParent> (hwnd); }

    /** Sets an X11 window in which the component should place itself as a child window,
        instead of creating a top-level window. This is used on Linux and BSD and has no
        effect on other platforms.
    */
    [[nodiscard]] DesktopWindowOptions withParentX11Window (uint64_t window) const          { return withParent<X11WindowParent> (window); }

    /** Sets the component that the window is opened from.

        The new window is kept above the window containing that component without being
        kept above other applications, and is minimised and restored with it. Where a window
        can position itself, it is also moved when that window is moved. A temporary window
        such as a menu, a tooltip or a drag image should name the component it was opened
        for, and a tool window that floats over a main window should name a component in
        that window, normally omitting ComponentPeer::windowAppearsOnTaskbar from the window
        style flags as well.

        The component's window is looked up when the desktop window is created, so the
        component should be on the desktop, or inside a component that is, by then. It has
        no effect on iOS or Android. On macOS a window takes the level of the window it is
        associated with, so Component::setAlwaysOnTop() has no effect on it.

        A window cannot be associated with a component in itself, or in a window that was
        opened from it, directly or indirectly. Such an association is ignored.

        @see ComponentPeer::getAssociatedComponent, TopLevelWindow::setAssociatedComponent
    */
    [[nodiscard]] DesktopWindowOptions withAssociatedComponent (Component* component) const { return withMember (*this, &DesktopWindowOptions::associatedComponent, component); }

    /** Returns the window's style flags. */
    int getStyleFlags() const noexcept                  { return styleFlags; }

    /** Returns the UIView to attach to. */
    void* getParentUIView() const noexcept              { return getParent<UIViewParent>(); }

    /** Returns the Android Activity to attach to. */
    void* getParentAndroidActivity() const noexcept     { return getParent<AndroidActivityParent>(); }

    /** Returns the Android ViewGroup to attach to. */
    void* getParentAndroidViewGroup() const noexcept    { return getParent<AndroidViewGroupParent>(); }

    /** Returns the NSView to attach to. */
    void* getParentNSView() const noexcept              { return getParent<NSViewParent>(); }

    /** Returns the HWND to attach to. */
    void* getParentHWND() const noexcept                { return getParent<HWNDParent>(); }

    /** Returns the X11 window to attach to. */
    uint64_t getParentX11Window() const noexcept        { return getParent<X11WindowParent>(); }

    /** Returns the component that the window is opened from, or nullptr if none was set. */
    Component* getAssociatedComponent() const noexcept  { return associatedComponent; }

private:
    struct UIViewParent             { void* handle = nullptr; };
    struct AndroidActivityParent    { void* handle = nullptr; };
    struct AndroidViewGroupParent   { void* handle = nullptr; };
    struct NSViewParent             { void* handle = nullptr; };
    struct HWNDParent               { void* handle = nullptr; };
    struct X11WindowParent          { uint64_t handle = 0; };

    using NativeParent = std::variant<std::monostate,
                                      UIViewParent,
                                      AndroidActivityParent,
                                      AndroidViewGroupParent,
                                      NSViewParent,
                                      HWNDParent,
                                      X11WindowParent>;

    template <typename Parent, typename Handle>
    DesktopWindowOptions withParent (Handle handle) const
    {
        return withMember (*this, &DesktopWindowOptions::nativeParent, NativeParent { Parent { handle } });
    }

    template <typename Parent>
    decltype (Parent::handle) getParent() const noexcept
    {
        const auto* parent = std::get_if<Parent> (&nativeParent);
        return parent != nullptr ? parent->handle : decltype (Parent::handle){};
    }

    int styleFlags = 0;
    NativeParent nativeParent;
    Component* associatedComponent = nullptr;
};

} // namespace juce
