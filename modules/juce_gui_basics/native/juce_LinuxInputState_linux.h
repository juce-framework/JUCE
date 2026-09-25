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

enum class LinuxInputBackend
{
    x11,
    wayland
};

//==============================================================================
/*  Records which windowing backend last received the pointer and the keyboard.

    X11 and Wayland windows can coexist in one process, but each backend has its own coordinate
    space and its own view of the mouse buttons and modifier keys. Process-wide queries such as
    ComponentPeer::getCurrentModifiersRealtime() and MouseInputSource::getCurrentRawMousePosition()
    are answered by the backend whose window most recently gained the device, and only that backend
    may change the corresponding flags in ModifierKeys::currentModifiers. The other backend's view
    is stale: an Xwayland window stops receiving modifier updates once the compositor moves the
    keyboard to a Wayland surface, and it never sees the buttons held during a Wayland drag.

    A device stays with its backend when it leaves a window, so that queries made while the pointer
    is outside every window still go to the backend that last saw it. It is forgotten when the
    window holding it is destroyed or the backend loses the device altogether. The enter and focus
    events of the two backends arrive on separate connections, so a leave can be processed after the
    other backend's enter, and forgetting only applies to the backend that still holds the device.
*/
class LinuxInputState final
{
public:
    LinuxInputState() = default;

    static LinuxInputState& get();

    // A window can only be entered while no button grab is active, so buttons the other backend
    // still records are released here on its behalf.
    void pointerEntered  (LinuxInputBackend);
    void keyboardEntered (LinuxInputBackend);

    void pointerLeft  (LinuxInputBackend);
    void keyboardLeft (LinuxInputBackend);

    // The backend whose window most recently gained the device
    std::optional<LinuxInputBackend> getPointerBackend()  const noexcept  { return pointerBackend; }
    std::optional<LinuxInputBackend> getKeyboardBackend() const noexcept  { return keyboardBackend; }

    // Whether a backend may change the mouse button or modifier key flags of the shared
    // ModifierKeys::currentModifiers. Any backend may while no window has gained the device.
    bool mayUpdatePointerState  (LinuxInputBackend) const noexcept;
    bool mayUpdateKeyboardState (LinuxInputBackend) const noexcept;

private:
    std::optional<LinuxInputBackend> pointerBackend, keyboardBackend;

    JUCE_LEAK_DETECTOR (LinuxInputState)
};

ModifierKeys getLinuxRealtimeModifiers (LinuxInputBackend fallback);

} // namespace juce
