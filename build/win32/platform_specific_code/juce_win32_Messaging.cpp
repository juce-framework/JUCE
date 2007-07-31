/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "win32_headers.h"
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_appframework/events/juce_MessageManager.h"
#include "../../../src/juce_appframework/application/juce_Application.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/basics/juce_Time.h"


//==============================================================================
static const unsigned int specialId             = WM_APP + 0x4400;
static const unsigned int broadcastId           = WM_APP + 0x4403;
static const unsigned int specialCallbackId     = WM_APP + 0x4402;

static const TCHAR* const messageWindowName     = _T("JUCEWindow");

HWND juce_messageWindowHandle = 0;

extern long improbableWindowNumber; // defined in windowing.cpp


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
                String* const messageString = (String*) lParam;
                MessageManager::getInstance()->deliverBroadcastMessage (*messageString);
                delete messageString;
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
        else
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
        return (*callback) (userData);
    else
        return (void*) SendMessage (juce_messageWindowHandle,
                                    specialCallbackId,
                                    (WPARAM) callback,
                                    (LPARAM) userData);
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

END_JUCE_NAMESPACE
