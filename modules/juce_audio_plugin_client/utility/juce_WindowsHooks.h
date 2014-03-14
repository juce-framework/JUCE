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

#if JUCE_WINDOWS

namespace
{
    static HHOOK mouseWheelHook = 0;
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

               #if 0 // XXX temporary for testing
                keyboardHook = SetWindowsHookEx (WH_KEYBOARD, keyboardHookCallback,
                                                 (HINSTANCE) Process::getCurrentModuleInstanceHandle(),
                                                 GetCurrentThreadId());
               #endif
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

               #if 0 // XXX temporary for testing
                if (keyboardHook != 0)
                {
                    UnhookWindowsHookEx (keyboardHook);
                    keyboardHook = 0;
                }
               #endif
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

       #if 0 // XXX temporary for testing
        static LRESULT CALLBACK keyboardHookCallback (int nCode, WPARAM wParam, LPARAM lParam)
        {
            if (nCode >= 0 && nCode == HC_ACTION)
                if (passKeyUpDownToPeer (GetFocus(), wParam, (lParam & (1 << 31)) == 0))
                    return 1;

            return CallNextHookEx (keyboardHook, nCode, wParam, lParam);
        }
       #endif
    };
}

#endif
