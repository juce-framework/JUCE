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

// Wayland is only considered for standalone processes. Individual peers may still use X11.
static bool isUsingWaylandBackend()
{
    return WaylandWindowSystem::shouldUseWaylandBackend()
        && WaylandWindowSystem::getInstance()->isWaylandAvailable();
}

static LinuxInputBackend getDefaultInputBackend()
{
    return isUsingWaylandBackend() ? LinuxInputBackend::wayland : LinuxInputBackend::x11;
}

// Process-wide input queries are answered by the backend whose window most recently gained the
// device, since the other backend's coordinates and button state do not apply to it.
static LinuxInputBackend getPointerBackend()
{
    return LinuxInputState::get().getPointerBackend().value_or (getDefaultInputBackend());
}

static LinuxInputBackend getKeyboardBackend()
{
    return LinuxInputState::get().getKeyboardBackend().value_or (getDefaultInputBackend());
}

//==============================================================================
ComponentPeer* Component::createNewPeer (int styleFlags, void* nativeWindowToAttachTo)
{
    if (isUsingWaylandBackend() && (styleFlags & ComponentPeer::windowRequiresX11) == 0)
    {
        if (nativeWindowToAttachTo == nullptr)
            return createWaylandComponentPeer (*this, styleFlags, nullptr);

        if ((styleFlags & ComponentPeer::windowIsTemporary) != 0)
            if (auto* peer = createWaylandComponentPeer (*this, styleFlags, nativeWindowToAttachTo))
                return peer;

        // An X11 parent needs ComponentPeer::windowRequiresX11 in the style flags.
        jassertfalse;
    }

    return createX11ComponentPeer (*this, styleFlags, nativeWindowToAttachTo);
}

//==============================================================================
JUCE_API bool JUCE_CALLTYPE Process::isForegroundProcess()
{
    // Wayland has no concept of an active application, so keyboard focus stands in for it.
    // Windows attached to native X11 parents still report activity through the X11 flag.
    if (isUsingWaylandBackend())
        return WaylandWindowSystem::getInstance()->hasKeyboardFocus()
            || isX11ApplicationActive();

    return isX11ApplicationActive();
}

JUCE_API void JUCE_CALLTYPE Process::makeForegroundProcess()  {}
JUCE_API void JUCE_CALLTYPE Process::hide()                   {}

//==============================================================================
void Desktop::setKioskComponent (Component* comp, bool enableOrDisable, bool)
{
    // A Wayland client cannot position or size a toplevel.
    // Kiosk mode has to ask the compositor for fullscreen.
    if (auto* peer = comp->getPeer(); isWaylandComponentPeer (peer))
    {
        peer->setFullScreen (enableOrDisable);
        return;
    }

    if (enableOrDisable)
        comp->setBounds (getDisplays().getDisplayForRect (comp->getScreenBounds())->logicalBounds.getSmallestIntegerContainer());
}

void Displays::findDisplays (const Desktop& desktop)
{
    if (isUsingWaylandBackend())
    {
        auto* windowSystem = WaylandWindowSystem::getInstance();

        // The callback re-enters findDisplays() through refresh(), so register it only once.
        if (! windowSystem->hasDisplaysChangedCallback())
            windowSystem->setDisplaysChangedCallback ([]
            {
                if (auto* currentDesktop = Desktop::getInstanceWithoutCreating())
                    currentDesktop->displays->refresh();
            });

        displays = windowSystem->findDisplays (desktop.getGlobalScaleFactor());
        return;
    }

    if (XWindowSystem::getInstance()->getDisplay() != nullptr)
    {
        displays = XWindowSystem::getInstance()->findDisplays (desktop.getGlobalScaleFactor());

        if (! displays.isEmpty())
            updateToLogical();
    }
}

bool Desktop::canUseSemiTransparentWindows() noexcept
{
    if (isUsingWaylandBackend())
        return true;

    return XWindowSystem::getInstance()->canUseSemiTransparentWindows();
}

class Desktop::NativeDarkModeChangeDetectorImpl final
{
public:
    NativeDarkModeChangeDetectorImpl()
    {
        if (! isUsingWaylandBackend())
            detector.emplace ([]
                              {
                                  if (auto* desktop = Desktop::getInstanceWithoutCreating())
                                      desktop->darkModeChanged();
                              });
    }

    bool isDarkModeEnabled() const noexcept
    {
        return detector.has_value() && detector->isDarkModeEnabled();
    }

private:
    std::optional<X11DarkModeChangeDetector> detector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeDarkModeChangeDetectorImpl)
};

std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
{
    return std::make_unique<NativeDarkModeChangeDetectorImpl>();
}

bool Desktop::isDarkModeActive() const
{
    return nativeDarkModeChangeDetectorImpl->isDarkModeEnabled();
}

static bool screenSaverAllowed = true;

void Desktop::setScreenSaverEnabled (bool isEnabled)
{
    if (screenSaverAllowed != isEnabled)
    {
        screenSaverAllowed = isEnabled;
        XWindowSystem::getInstance()->setScreenSaverEnabled (screenSaverAllowed);
    }
}

bool Desktop::isScreenSaverEnabled()
{
    return screenSaverAllowed;
}

double Desktop::getDefaultMasterScale()                             { return 1.0; }

Desktop::DisplayOrientation Desktop::getCurrentOrientation() const  { return upright; }
void Desktop::allowedOrientationsChanged()                          {}

//==============================================================================
bool detail::MouseInputSourceList::addSource()
{
    auto numSources = sources.size();

    if (numSources == 0 || canUseTouch())
    {
        addSource (numSources, numSources == 0 ? MouseInputSource::InputSourceType::mouse
                                               : MouseInputSource::InputSourceType::touch);
        return true;
    }

    return false;
}

bool detail::MouseInputSourceList::canUseTouch() const
{
    if (isUsingWaylandBackend())
        return WaylandWindowSystem::getInstance()->isTouchBound();

    return XWindowSystem::getInstance()->canUseMultiTouch();
}

Point<float> MouseInputSource::getCurrentRawMousePosition()
{
    if (getPointerBackend() == LinuxInputBackend::wayland)
        return WaylandWindowSystem::getInstance()->getCurrentMousePosition();

    return detail::ScalingHelpers::convertPhysicalScreenPointToLogical (XWindowSystem::getInstance()->getCurrentMousePosition());
}

void MouseInputSource::setRawMousePosition (Point<float> newPosition)
{
    // Compositor support for wp_pointer_warp_v1 is currently spotty.
    if (getPointerBackend() == LinuxInputBackend::wayland)
        return;

    XWindowSystem::getInstance()->setMousePosition (detail::ScalingHelpers::convertLogicalScreenPointToPhysical (newPosition));
}

//==============================================================================
class MouseCursor::PlatformSpecificHandle
{
public:
    explicit PlatformSpecificHandle (MouseCursor::StandardCursorType type)
        : cursorInfo (type) {}

    explicit PlatformSpecificHandle (const detail::CustomMouseCursorInfo& info)
        : cursorInfo (info) {}

    static void showInWindow (PlatformSpecificHandle* handle, ComponentPeer* peer)
    {
        if (peer == nullptr)
            return;

        if (isWaylandComponentPeer (peer))
            WaylandMouseCursor::showInWindow (handle != nullptr ? &handle->getCursor (handle->waylandCursor) : nullptr, *peer);
        else
            X11MouseCursor::showInWindow (handle != nullptr ? &handle->getCursor (handle->x11Cursor) : nullptr, *peer);
    }

private:
    template <typename CursorType>
    CursorType& getCursor (std::optional<CursorType>& cursor)
    {
        if (! cursor.has_value())
        {
            if (const auto* type = std::get_if<MouseCursor::StandardCursorType> (&cursorInfo))
                cursor.emplace (*type);
            else if (const auto* info = std::get_if<detail::CustomMouseCursorInfo> (&cursorInfo))
                cursor.emplace (*info);
        }

        return *cursor;
    }

    std::variant<MouseCursor::StandardCursorType, detail::CustomMouseCursorInfo> cursorInfo;
    std::optional<X11MouseCursor> x11Cursor;
    std::optional<WaylandMouseCursor> waylandCursor;

    JUCE_DECLARE_NON_COPYABLE (PlatformSpecificHandle)
    JUCE_DECLARE_NON_MOVEABLE (PlatformSpecificHandle)
};

//==============================================================================
static ComponentPeer* getPeerForDragEvent (Component* sourceComp)
{
    if (sourceComp == nullptr)
        if (auto* draggingSource = Desktop::getInstance().getDraggingMouseSource (0))
            sourceComp = draggingSource->getComponentUnderMouse();

    if (sourceComp != nullptr)
        return sourceComp->getPeer();

    return nullptr;
}

bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, bool canMoveFiles,
                                                           Component* sourceComp, std::function<void()> callback)
{
    if (files.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent (sourceComp))
    {
        if (isWaylandComponentPeer (peer))
            return performWaylandExternalDragDropOfFiles (*peer, files, canMoveFiles, std::move (callback));

        return XWindowSystem::getInstance()->externalDragFileInit (peer, files, canMoveFiles, std::move (callback));
    }

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text, Component* sourceComp,
                                                          std::function<void()> callback)
{
    if (text.isEmpty())
        return false;

    if (auto* peer = getPeerForDragEvent (sourceComp))
    {
        if (isWaylandComponentPeer (peer))
            return performWaylandExternalDragDropOfText (*peer, text, std::move (callback));

        return XWindowSystem::getInstance()->externalDragTextInit (peer, text, std::move (callback));
    }

    // This method must be called in response to a component's mouseDown or mouseDrag event!
    jassertfalse;
    return false;
}

//==============================================================================
// The compositor mirrors the clipboard between its Wayland clients and Xwayland, but a Wayland
// client only receives selection offers while one of its surfaces holds the keyboard.
void SystemClipboard::copyTextToClipboard (const String& clipText)
{
    if (getKeyboardBackend() == LinuxInputBackend::wayland)
    {
        WaylandWindowSystem::getInstance()->copyTextToClipboard (clipText);
        return;
    }

    XWindowSystem::getInstance()->copyTextToClipboard (clipText);
}

String SystemClipboard::getTextFromClipboard()
{
    if (getKeyboardBackend() == LinuxInputBackend::wayland)
        return WaylandWindowSystem::getInstance()->getTextFromClipboard();

    return XWindowSystem::getInstance()->getTextFromClipboard();
}

//==============================================================================
bool KeyPress::isKeyCurrentlyDown (int keyCode)
{
    if (getKeyboardBackend() == LinuxInputBackend::wayland)
        return WaylandWindowSystem::getInstance()->isKeyCurrentlyDown (keyCode);

    return XWindowSystem::getInstance()->isKeyCurrentlyDown (keyCode);
}

void LookAndFeel::playAlertSound()
{
    std::cout << "\a" << std::flush;
}

//==============================================================================
Image detail::WindowingHelpers::createIconForFile (const File&)
{
    return {};
}

} // namespace juce
