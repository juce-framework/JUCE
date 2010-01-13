/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
static const unsigned int specialId             = WM_APP + 0x4400;
static const unsigned int broadcastId           = WM_APP + 0x4403;
static const unsigned int specialCallbackId     = WM_APP + 0x4402;

static const TCHAR* const messageWindowName     = _T("JUCEWindow");

HWND juce_messageWindowHandle = 0;

extern long improbableWindowNumber; // defined in windowing.cpp

#ifndef WM_APPCOMMAND
 #define WM_APPCOMMAND 0x0319
#endif


//==============================================================================
static LRESULT CALLBACK juce_MessageWndProc (HWND h,
                                             const UINT message,
                                             const WPARAM wParam,
                                             const LPARAM lParam) throw()
{
    JUCE_TRY
    {
        if (h == juce_messageWindowHandle)
        {
            if (message == specialCallbackId)
            {
                MessageCallbackFunction* const func = (MessageCallbackFunction*) wParam;
                return (LRESULT) (*func) ((void*) lParam);
            }
            else if (message == specialId)
            {
                // these are trapped early in the dispatch call, but must also be checked
                // here in case there are windows modal dialog boxes doing their own
                // dispatch loop and not calling our version

                MessageManager::getInstance()->deliverMessage ((void*) lParam);
                return 0;
            }
            else if (message == broadcastId)
            {
                const ScopedPointer <String> messageString ((String*) lParam);
                MessageManager::getInstance()->deliverBroadcastMessage (*messageString);
                return 0;
            }
            else if (message == WM_COPYDATA && ((const COPYDATASTRUCT*) lParam)->dwData == broadcastId)
            {
                const String messageString ((const juce_wchar*) ((const COPYDATASTRUCT*) lParam)->lpData,
                                            ((const COPYDATASTRUCT*) lParam)->cbData / sizeof (juce_wchar));

                PostMessage (juce_messageWindowHandle, broadcastId, 0, (LPARAM) new String (messageString));
                return 0;
            }
        }
    }
    JUCE_CATCH_EXCEPTION

    return DefWindowProc (h, message, wParam, lParam);
}

static bool isEventBlockedByModalComps (MSG& m)
{
    if (Component::getNumCurrentlyModalComponents() == 0
         || GetWindowLong (m.hwnd, GWLP_USERDATA) == improbableWindowNumber)
        return false;

    switch (m.message)
    {
        case WM_MOUSEMOVE:
        case WM_NCMOUSEMOVE:
        case 0x020A: /* WM_MOUSEWHEEL */
        case 0x020E: /* WM_MOUSEHWHEEL */
        case WM_KEYUP:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_APPCOMMAND:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MOUSEACTIVATE:
        case WM_NCMOUSEHOVER:
        case WM_MOUSEHOVER:
            return true;

        case WM_NCLBUTTONDOWN:
        case WM_NCLBUTTONDBLCLK:
        case WM_NCRBUTTONDOWN:
        case WM_NCRBUTTONDBLCLK:
        case WM_NCMBUTTONDOWN:
        case WM_NCMBUTTONDBLCLK:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            Component* const modal = Component::getCurrentlyModalComponent (0);
            if (modal != 0)
                modal->inputAttemptWhenModal();

            return true;
        }

        default:
            break;
    }

    return false;
}

bool juce_dispatchNextMessageOnSystemQueue (const bool returnIfNoPendingMessages)
{
    MSG m;

    if (returnIfNoPendingMessages && ! PeekMessage (&m, (HWND) 0, 0, 0, 0))
        return false;

    if (GetMessage (&m, (HWND) 0, 0, 0) > 0)
    {
        if (m.message == specialId
             && m.hwnd == juce_messageWindowHandle)
        {
            MessageManager::getInstance()->deliverMessage ((void*) m.lParam);
        }
        else if (! isEventBlockedByModalComps (m))
        {
            if (GetWindowLong (m.hwnd, GWLP_USERDATA) != improbableWindowNumber
                 && (m.message == WM_LBUTTONDOWN || m.message == WM_RBUTTONDOWN))
            {
                // if it's someone else's window being clicked on, and the focus is
                // currently on a juce window, pass the kb focus over..
                HWND currentFocus = GetFocus();

                if (currentFocus == 0 || GetWindowLong (currentFocus, GWLP_USERDATA) == improbableWindowNumber)
                    SetFocus (m.hwnd);
            }

            TranslateMessage (&m);
            DispatchMessage (&m);
        }
    }

    return true;
}

//==============================================================================
bool juce_postMessageToSystemQueue (void* message)
{
    return PostMessage (juce_messageWindowHandle, specialId, 0, (LPARAM) message) != 0;
}

//==============================================================================
void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* callback,
                                                   void* userData)
{
    if (MessageManager::getInstance()->isThisTheMessageThread())
    {
        return (*callback) (userData);
    }
    else
    {
        // If a thread has a MessageManagerLock and then tries to call this method, it'll
        // deadlock because the message manager is blocked from running, and can't
        // call your function..
        jassert (! MessageManager::getInstance()->currentThreadHasLockedMessageManager());

        return (void*) SendMessage (juce_messageWindowHandle,
                                    specialCallbackId,
                                    (WPARAM) callback,
                                    (LPARAM) userData);
    }
}

//==============================================================================
static BOOL CALLBACK BroadcastEnumWindowProc (HWND hwnd, LPARAM lParam)
{
    if (hwnd != juce_messageWindowHandle)
        (reinterpret_cast <VoidArray*> (lParam))->add ((void*) hwnd);

    return TRUE;
}

void MessageManager::broadcastMessage (const String& value) throw()
{
    VoidArray windows;
    EnumWindows (&BroadcastEnumWindowProc, (LPARAM) &windows);

    const String localCopy (value);

    COPYDATASTRUCT data;
    data.dwData = broadcastId;
    data.cbData = (localCopy.length() + 1) * sizeof (juce_wchar);
    data.lpData = (void*) (const juce_wchar*) localCopy;

    for (int i = windows.size(); --i >= 0;)
    {
        HWND hwnd = (HWND) windows.getUnchecked(i);

        TCHAR windowName [64]; // no need to read longer strings than this
        GetWindowText (hwnd, windowName, 64);
        windowName [63] = 0;

        if (String (windowName) == String (messageWindowName))
        {
            DWORD_PTR result;
            SendMessageTimeout (hwnd, WM_COPYDATA,
                                (WPARAM) juce_messageWindowHandle,
                                (LPARAM) &data,
                                SMTO_BLOCK | SMTO_ABORTIFHUNG,
                                8000,
                                &result);
        }
    }
}

//==============================================================================
static const String getMessageWindowClassName()
{
    // this name has to be different for each app/dll instance because otherwise
    // poor old Win32 can get a bit confused (even despite it not being a process-global
    // window class).

    static int number = 0;
    if (number == 0)
        number = 0x7fffffff & (int) Time::getHighResolutionTicks();

    return T("JUCEcs_") + String (number);
}

void MessageManager::doPlatformSpecificInitialisation()
{
    OleInitialize (0);

    const String className (getMessageWindowClassName());

    HMODULE hmod = (HMODULE) PlatformUtilities::getCurrentModuleInstanceHandle();

    WNDCLASSEX wc;
    zerostruct (wc);

    wc.cbSize         = sizeof (wc);
    wc.lpfnWndProc    = (WNDPROC) juce_MessageWndProc;
    wc.cbWndExtra     = 4;
    wc.hInstance      = hmod;
    wc.lpszClassName  = className;

    RegisterClassEx (&wc);

    juce_messageWindowHandle = CreateWindow (wc.lpszClassName,
                                             messageWindowName,
                                             0, 0, 0, 0, 0, 0, 0,
                                             hmod, 0);
}

void MessageManager::doPlatformSpecificShutdown()
{
    DestroyWindow (juce_messageWindowHandle);
    UnregisterClass (getMessageWindowClassName(), 0);
    OleUninitialize();
}


#endif
