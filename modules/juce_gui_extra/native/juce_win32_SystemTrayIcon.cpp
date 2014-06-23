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
                 || lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONDBLCLK)
            {
                if (Component* const current = Component::getCurrentlyModalComponent())
                    current->inputAttemptWhenModal();
            }
        }
        else
        {
            ModifierKeys eventMods (ModifierKeys::getCurrentModifiersRealtime());

            if (lParam == WM_LBUTTONDOWN || lParam == WM_LBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::leftButtonModifier);
            else if (lParam == WM_RBUTTONDOWN || lParam == WM_RBUTTONDBLCLK)
                eventMods = eventMods.withFlags (ModifierKeys::rightButtonModifier);
            else if (lParam == WM_LBUTTONUP || lParam == WM_RBUTTONUP)
                eventMods = eventMods.withoutMouseButtons();

            const Time eventTime (getMouseEventTime());

            const MouseEvent e (Desktop::getInstance().getMainMouseSource(),
                                Point<float>(), eventMods, &owner, &owner, eventTime,
                                Point<float>(), eventTime, 1, false);

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
                    return iconComp->pimpl;

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
            pimpl = new Pimpl (*this, hicon, (HWND) getWindowHandle());
        else
            pimpl->updateIcon (hicon);
    }
    else
    {
        pimpl = nullptr;
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
    showInfoBubble (String::empty, String::empty);
}

void* SystemTrayIconComponent::getNativeHandle() const
{
    return pimpl != nullptr ? &(pimpl->iconData) : nullptr;
}
