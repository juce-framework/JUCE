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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
class HiddenMessageWindow
{
public:
    HiddenMessageWindow (const TCHAR* const messageWindowName, WNDPROC wndProc)
    {
        String className ("JUCE_");
        className << String::toHexString (Time::getHighResolutionTicks());

        HMODULE moduleHandle = (HMODULE) PlatformUtilities::getCurrentModuleInstanceHandle();

        WNDCLASSEX wc = { 0 };
        wc.cbSize         = sizeof (wc);
        wc.lpfnWndProc    = wndProc;
        wc.cbWndExtra     = 4;
        wc.hInstance      = moduleHandle;
        wc.lpszClassName  = className.toWideCharPointer();

        atom = RegisterClassEx (&wc);
        jassert (atom != 0);

        hwnd = CreateWindow (getClassNameFromAtom(), messageWindowName,
                             0, 0, 0, 0, 0, 0, 0, moduleHandle, 0);
        jassert (hwnd != 0);
    }

    ~HiddenMessageWindow()
    {
        DestroyWindow (hwnd);
        UnregisterClass (getClassNameFromAtom(), 0);
    }

    inline HWND getHWND() const noexcept     { return hwnd; }

private:
    ATOM atom;
    HWND hwnd;

    LPCTSTR getClassNameFromAtom() noexcept  { return (LPCTSTR) MAKELONG (atom, 0); }
};


//==============================================================================
HWND juce_messageWindowHandle = 0;  // (this is referred to by other parts of the codebase)

//==============================================================================
class JuceWindowIdentifier
{
public:
    static bool isJUCEWindow (HWND hwnd) noexcept
    {
        return GetWindowLong (hwnd, GWLP_USERDATA) == improbableWindowNumber;
    }

    static void setAsJUCEWindow (HWND hwnd, bool isJuceWindow) noexcept
    {
        SetWindowLongPtr (hwnd, GWLP_USERDATA, isJuceWindow ? improbableWindowNumber : 0);
    }

private:
    enum { improbableWindowNumber = 0xf965aa01 };
};


//==============================================================================
namespace WindowsMessageHelpers
{
    const unsigned int specialId             = WM_APP + 0x4400;
    const unsigned int broadcastId           = WM_APP + 0x4403;
    const unsigned int specialCallbackId     = WM_APP + 0x4402;

    const TCHAR messageWindowName[] = _T("JUCEWindow");
    ScopedPointer<HiddenMessageWindow> messageWindow;

    //==============================================================================
    LRESULT CALLBACK messageWndProc (HWND h, const UINT message, const WPARAM wParam, const LPARAM lParam) noexcept
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

                    Message* const message = reinterpret_cast <Message*> (lParam);
                    MessageManager::getInstance()->deliverMessage (message);
                    message->decReferenceCount();
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
                    const COPYDATASTRUCT* data = (COPYDATASTRUCT*) lParam;

                    const String messageString (CharPointer_UTF32 ((const CharPointer_UTF32::CharType*) data->lpData),
                                                data->cbData / sizeof (CharPointer_UTF32::CharType));

                    PostMessage (juce_messageWindowHandle, broadcastId, 0, (LPARAM) new String (messageString));
                    return 0;
                }
            }
        }
        JUCE_CATCH_EXCEPTION

        return DefWindowProc (h, message, wParam, lParam);
    }

    bool isHWNDBlockedByModalComponents (HWND h) noexcept
    {
        for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
        {
            Component* const c = Desktop::getInstance().getComponent (i);

            if (c != nullptr
                  && (! c->isCurrentlyBlockedByAnotherModalComponent())
                  && IsChild ((HWND) c->getWindowHandle(), h))
                return false;
        }

        return true;
    }

    bool isEventBlockedByModalComps (MSG& m)
    {
        if (Component::getNumCurrentlyModalComponents() == 0 || JuceWindowIdentifier::isJUCEWindow (m.hwnd))
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
                return isHWNDBlockedByModalComponents (m.hwnd);

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
                if (isHWNDBlockedByModalComponents (m.hwnd))
                {
                    Component* const modal = Component::getCurrentlyModalComponent (0);
                    if (modal != nullptr)
                        modal->inputAttemptWhenModal();

                    return true;
                }
                break;

            default:
                break;
        }

        return false;
    }

    BOOL CALLBACK broadcastEnumWindowProc (HWND hwnd, LPARAM lParam)
    {
        if (hwnd != juce_messageWindowHandle)
            reinterpret_cast <Array<HWND>*> (lParam)->add (hwnd);

        return TRUE;
    }
}

//==============================================================================
bool MessageManager::dispatchNextMessageOnSystemQueue (const bool returnIfNoPendingMessages)
{
    using namespace WindowsMessageHelpers;
    MSG m;

    if (returnIfNoPendingMessages && ! PeekMessage (&m, (HWND) 0, 0, 0, 0))
        return false;

    if (GetMessage (&m, (HWND) 0, 0, 0) >= 0)
    {
        if (m.message == specialId && m.hwnd == juce_messageWindowHandle)
        {
            Message* const message = reinterpret_cast <Message*> (m.lParam);
            MessageManager::getInstance()->deliverMessage (message);
            message->decReferenceCount();
        }
        else if (m.message == WM_QUIT)
        {
            if (JUCEApplication::getInstance() != nullptr)
                JUCEApplication::getInstance()->systemRequestedQuit();
        }
        else if (! WindowsMessageHelpers::isEventBlockedByModalComps (m))
        {
            if ((m.message == WM_LBUTTONDOWN || m.message == WM_RBUTTONDOWN)
                 && ! JuceWindowIdentifier::isJUCEWindow (m.hwnd))
            {
                // if it's someone else's window being clicked on, and the focus is
                // currently on a juce window, pass the kb focus over..
                HWND currentFocus = GetFocus();

                if (currentFocus == 0 || JuceWindowIdentifier::isJUCEWindow (currentFocus))
                    SetFocus (m.hwnd);
            }

            TranslateMessage (&m);
            DispatchMessage (&m);
        }
    }

    return true;
}

bool MessageManager::postMessageToSystemQueue (Message* message)
{
    message->incReferenceCount();
    return PostMessage (juce_messageWindowHandle, WindowsMessageHelpers::specialId, 0, (LPARAM) message) != 0;
}

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
                                    WindowsMessageHelpers::specialCallbackId,
                                    (WPARAM) callback,
                                    (LPARAM) userData);
    }
}

void MessageManager::broadcastMessage (const String& value)
{
    Array<HWND> windows;
    EnumWindows (&WindowsMessageHelpers::broadcastEnumWindowProc, (LPARAM) &windows);

    const String localCopy (value);

    COPYDATASTRUCT data;
    data.dwData = WindowsMessageHelpers::broadcastId;
    data.cbData = (localCopy.length() + 1) * sizeof (CharPointer_UTF32::CharType);
    data.lpData = (void*) localCopy.toUTF32().getAddress();

    for (int i = windows.size(); --i >= 0;)
    {
        HWND hwnd = windows.getUnchecked(i);

        TCHAR windowName [64]; // no need to read longer strings than this
        GetWindowText (hwnd, windowName, 64);
        windowName [63] = 0;

        if (String (windowName) == WindowsMessageHelpers::messageWindowName)
        {
            DWORD_PTR result;
            SendMessageTimeout (hwnd, WM_COPYDATA,
                                (WPARAM) juce_messageWindowHandle,
                                (LPARAM) &data,
                                SMTO_BLOCK | SMTO_ABORTIFHUNG, 8000, &result);
        }
    }
}

//==============================================================================
void MessageManager::doPlatformSpecificInitialisation()
{
    OleInitialize (0);

    using namespace WindowsMessageHelpers;
    messageWindow = new HiddenMessageWindow (messageWindowName, (WNDPROC) messageWndProc);
    juce_messageWindowHandle = messageWindow->getHWND();
}

void MessageManager::doPlatformSpecificShutdown()
{
    WindowsMessageHelpers::messageWindow = nullptr;

    OleUninitialize();
}

//==============================================================================
class DeviceChangeDetector   // (Used by various audio classes)
{
public:
    DeviceChangeDetector (const wchar_t* const name)
        : messageWindow (name, (WNDPROC) deviceChangeEventCallback)
    {
        SetWindowLongPtr (messageWindow.getHWND(), GWLP_USERDATA, (LONG_PTR) this);
    }

    virtual ~DeviceChangeDetector() {}

protected:
    virtual void systemDeviceChanged() = 0;

private:
    HiddenMessageWindow messageWindow;

    static LRESULT CALLBACK deviceChangeEventCallback (HWND h, const UINT message,
                                                       const WPARAM wParam, const LPARAM lParam)
    {
        if (message == WM_DEVICECHANGE
             && (wParam == 0x8000 /*DBT_DEVICEARRIVAL*/
                  || wParam == 0x8004 /*DBT_DEVICEREMOVECOMPLETE*/
                  || wParam == 0x0007 /*DBT_DEVNODES_CHANGED*/))
        {
            ((DeviceChangeDetector*) GetWindowLongPtr (h, GWLP_USERDATA))->systemDeviceChanged();
        }

        return DefWindowProc (h, message, wParam, lParam);
    }
};

#endif
