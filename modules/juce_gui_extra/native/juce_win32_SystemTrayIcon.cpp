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

// (these functions are from juce_win32_Windowing.cpp)
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

        Shell_NotifyIcon (NIM_ADD, &iconData);

        // In order to receive the "TaskbarCreated" message, we need to request that it's not filtered out.
        // (Need to load dynamically, as ChangeWindowMessageFilter is only available in Vista and later)
        typedef BOOL (WINAPI* ChangeWindowMessageFilterType) (UINT, DWORD);
        ChangeWindowMessageFilterType changeWindowMessageFilter
            = (ChangeWindowMessageFilterType) getUser32Function ("ChangeWindowMessageFilter");

        if (changeWindowMessageFilter != nullptr)
            changeWindowMessageFilter (taskbarCreatedMessage, 1 /* MSGFLT_ADD */);
    }

    ~Pimpl()
    {
        SetWindowLongPtr (iconData.hWnd, GWLP_WNDPROC, (LONG_PTR) originalWndProc);

        iconData.uFlags = 0;
        Shell_NotifyIcon (NIM_DELETE, &iconData);
        DestroyIcon (iconData.hIcon);
    }

    void updateIcon (HICON hicon)
    {
        HICON oldIcon = iconData.hIcon;

        iconData.hIcon = hicon;
        iconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        Shell_NotifyIcon (NIM_MODIFY, &iconData);

        DestroyIcon (oldIcon);
    }

    void setToolTip (const String& toolTip)
    {
        iconData.uFlags = NIF_TIP;
        toolTip.copyToUTF16 (iconData.szTip, sizeof (iconData.szTip) - 1);
        Shell_NotifyIcon (NIM_MODIFY, &iconData);
    }

    void handleTaskBarEvent (const LPARAM lParam)
    {
        if (owner.isCurrentlyBlockedByAnotherModalComponent())
        {
            if (lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN
                 || lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONDBLCLK)
            {
                Component* const current = Component::getCurrentlyModalComponent();

                if (current != nullptr)
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

            const MouseEvent e (Desktop::getInstance().getMainMouseSource(),
                                Point<int>(), eventMods, &owner, &owner, Time (getMouseEventTime()),
                                Point<int>(), Time (getMouseEventTime()), 1, false);

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
            else if (lParam == WM_LBUTTONDBLCLK || lParam == WM_LBUTTONDBLCLK)
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
        {
            ComponentPeer* peer = (ComponentPeer*) GetWindowLongPtr (hwnd, 8);

            if (peer != nullptr)
            {
                SystemTrayIconComponent* const iconComp = dynamic_cast<SystemTrayIconComponent*> (&(peer->getComponent()));

                if (iconComp != nullptr)
                    return iconComp->pimpl;
            }
        }

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
            Shell_NotifyIcon (NIM_ADD, &iconData);
        }

        return CallWindowProc (originalWndProc, hwnd, message, wParam, lParam);
    }

private:
    SystemTrayIconComponent& owner;
    NOTIFYICONDATA iconData;
    WNDPROC originalWndProc;
    const DWORD taskbarCreatedMessage;
    enum { WM_TRAYNOTIFY = WM_USER + 100 };

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
