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

extern void* getUser32Function (const char*);

namespace IconConverters
{
    extern HICON createHICONFromImage (const Image&, BOOL isIcon, int hotspotX, int hotspotY);
}

//==============================================================================
class SystemTrayIconComponent::Pimpl
{
public:
    Pimpl (SystemTrayIconComponent& owner_, HICON hicon, HWND hwnd)
        : owner (owner_),
          originalWndProc ((WNDPROC) GetWindowLongPtr (hwnd, GWLP_WNDPROC)),
          taskbarCreatedMessage (RegisterWindowMessage (TEXT ("TaskbarCreated")))
    {
        SetWindowLongPtr (hwnd, GWLP_WNDPROC, (LONG_PTR) hookedWndProc);

        zerostruct (iconData);
        iconData.cbSize = sizeof (iconData);
        iconData.hWnd = hwnd;
        iconData.uID = (UINT) (pointer_sized_int) hwnd;
        iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        iconData.uCallbackMessage = WM_TRAYNOTIFY;
        iconData.hIcon = hicon;

        notify (NIM_ADD);

        // In order to receive the "TaskbarCreated" message, we need to request that it's not filtered out.
        // (Need to load dynamically, as ChangeWindowMessageFilter is only available in Vista and later)
        typedef BOOL (WINAPI* ChangeWindowMessageFilterType) (UINT, DWORD);

        if (ChangeWindowMessageFilterType changeWindowMessageFilter
                = (ChangeWindowMessageFilterType) getUser32Function ("ChangeWindowMessageFilter"))
            changeWindowMessageFilter (taskbarCreatedMessage, 1 /* MSGFLT_ADD */);
    }

    ~Pimpl()
    {
        SetWindowLongPtr (iconData.hWnd, GWLP_WNDPROC, (LONG_PTR) originalWndProc);

        iconData.uFlags = 0;
        notify (NIM_DELETE);
        DestroyIcon (iconData.hIcon);
    }

    void updateIcon (HICON hicon)
    {
        HICON oldIcon = iconData.hIcon;

        iconData.hIcon = hicon;
        iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        notify (NIM_MODIFY);

        DestroyIcon (oldIcon);
    }

    void setToolTip (const String& toolTip)
    {
        iconData.uFlags = NIF_TIP;
        toolTip.copyToUTF16 (iconData.szTip, sizeof (iconData.szTip) - 1);
        notify (NIM_MODIFY);
    }

    void handleTaskBarEvent (const LPARAM lParam)
    {
        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN
                 || lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONDBLCLK)
            {
                if (auto* current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
            }
        }
        else
        {
            ModifierKeys eventMods (ComponentPeer::getCurrentModifiersRealtime());

            if (lParam == WM_LBUTTONDOWN || lParam == WM_LBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::leftButtonModifier);
            else if (lParam == WM_RBUTTONDOWN || lParam == WM_RBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::rightButtonModifier);
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
                eventMods = eventMods.withoutMouseButtons();

            const Time eventTime (getMouseEventTime());

            const MouseEvent e (Desktop::getInstance().getMainMouseSource(), {}, eventMods,
                                MouseInputSource::invalidPressure, MouseInputSource::invalidOrientation,
                                MouseInputSource::invalidRotation, MouseInputSource::invalidTiltX, MouseInputSource::invalidTiltY,
                                &owner, &owner, eventTime, {}, eventTime, 1, false);

            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN)
            {
                SetFocus (iconData.hWnd);
                SetForegroundWindow (iconData.hWnd);
                owner.mouseDown (e);
            }
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
            {
                owner.mouseUp (e);
            }
            else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_RBUTTONDBLCLK)
            {
                owner.mouseDoubleClick (e);
            }
            else if (lParam == WM_MOUSEMOVE)
            {
                owner.mouseMove (e);
            }
        }
    }

    static Pimpl* getPimpl (HWND hwnd)
    {
        if (JuceWindowIdentifier::isJUCEWindow (hwnd))
            if (ComponentPeer* peer = (ComponentPeer*) GetWindowLongPtr (hwnd, 8))
                if (SystemTrayIconComponent* const iconComp = dynamic_cast<SystemTrayIconComponent*> (&(peer->getComponent())))
                    return iconComp->pimpl.get();

        return nullptr;
    }

    static LRESULT CALLBACK hookedWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (Pimpl* const p = getPimpl (hwnd))
            return p->windowProc  (hwnd, message, wParam, lParam);

        return DefWindowProcW (hwnd, message, wParam, lParam);
   }

    LRESULT windowProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        if (message == WM_TRAYNOTIFY)
        {
            handleTaskBarEvent (lParam);
        }
        else if (message == taskbarCreatedMessage)
        {
            iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
            notify (NIM_ADD);
        }

        return CallWindowProc (originalWndProc, hwnd, message, wParam, lParam);
    }

    void showBubble (const String& title, const String& content)
    {
        iconData.uFlags = 0x10 /*NIF_INFO*/;
        title.copyToUTF16 (iconData.szInfoTitle, sizeof (iconData.szInfoTitle) - 1);
        content.copyToUTF16 (iconData.szInfo, sizeof (iconData.szInfo) - 1);
        notify (NIM_MODIFY);
    }

    SystemTrayIconComponent& owner;
    NOTIFYICONDATA iconData;

private:
    WNDPROC originalWndProc;
    const DWORD taskbarCreatedMessage;
    enum { WM_TRAYNOTIFY = WM_USER + 100 };

    void notify (DWORD message) noexcept    { Shell_NotifyIcon (message, &iconData); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

//==============================================================================
void SystemTrayIconComponent::setIconImage (const Image& newImage)
{
    if (newImage.isValid())
    {
        HICON hicon = IconConverters::createHICONFromImage (newImage, TRUE, 0, 0);

        if (pimpl == nullptr)
            pimpl.reset (new Pimpl (*this, hicon, (HWND) getWindowHandle()));
        else
            pimpl->updateIcon (hicon);
    }
    else
    {
        pimpl.reset();
    }
}

void SystemTrayIconComponent::setIconTooltip (const String& tooltip)
{
    if (pimpl != nullptr)
        pimpl->setToolTip (tooltip);
}

void SystemTrayIconComponent::setHighlighted (bool)
{
    // N/A on Windows.
}

void SystemTrayIconComponent::showInfoBubble (const String& title, const String& content)
{
    if (pimpl != nullptr)
        pimpl->showBubble (title, content);
}

void SystemTrayIconComponent::hideInfoBubble()
{
    showInfoBubble (String(), String());
}

void* SystemTrayIconComponent::getNativeHandle() const
{
    return pimpl != nullptr ? &(pimpl->iconData) : nullptr;
}

} // namespace juce
