/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#if JUCE_WINDOWS

namespace juce
{
    // This function is in juce_win32_Windowing.cpp
    extern bool offerKeyMessageToJUCEWindow (MSG&);
}

namespace
{
    static HHOOK mouseWheelHook = 0, keyboardHook = 0;
    static int numHookUsers = 0;

    struct WindowsHooks
    {
        WindowsHooks()
        {
            if (numHookUsers++ == 0)
            {
                mouseWheelHook = SetWindowsHookEx (WH_MOUSE, mouseWheelHookCallback,
                                                   (HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                                   GetCurrentThreadId());

                keyboardHook = SetWindowsHookEx (WH_GETMESSAGE, keyboardHookCallback,
                                                 (HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                                 GetCurrentThreadId());
            }
        }

        ~WindowsHooks()
        {
            if (--numHookUsers == 0)
            {
                if (mouseWheelHook != 0)
                {
                    UnhookWindowsHookEx (mouseWheelHook);
                    mouseWheelHook = 0;
                }

                if (keyboardHook != 0)
                {
                    UnhookWindowsHookEx (keyboardHook);
                    keyboardHook = 0;
                }
            }
        }

        static LRESULT CALLBACK mouseWheelHookCallback (int nCode, WPARAM wParam, LPARAM lParam)
        {
            if (nCode >= 0 && wParam == WM_MOUSEWHEEL)
            {
                // using a local copy of this struct to support old mingw libraries
                struct MOUSEHOOKSTRUCTEX_  : public MOUSEHOOKSTRUCT  { DWORD mouseData; };

                const MOUSEHOOKSTRUCTEX_& hs = *(MOUSEHOOKSTRUCTEX_*) lParam;

                if (Component* const comp = Desktop::getInstance().findComponentAt (Point<int> (hs.pt.x, hs.pt.y)))
                    if (comp->getWindowHandle() != 0)
                        return PostMessage ((HWND) comp->getWindowHandle(), WM_MOUSEWHEEL,
                                            hs.mouseData & 0xffff0000, (hs.pt.x & 0xffff) | (hs.pt.y << 16));
            }

            return CallNextHookEx (mouseWheelHook, nCode, wParam, lParam);
        }

        static LRESULT CALLBACK keyboardHookCallback (int nCode, WPARAM wParam, LPARAM lParam)
        {
            MSG& msg = *(MSG*) lParam;

            if (nCode == HC_ACTION && wParam == PM_REMOVE
                 && offerKeyMessageToJUCEWindow (msg))
            {
                zerostruct (msg);
                msg.message = WM_USER;
                return 1;
            }

            return CallNextHookEx (keyboardHook, nCode, wParam, lParam);
        }
    };
}

#endif
