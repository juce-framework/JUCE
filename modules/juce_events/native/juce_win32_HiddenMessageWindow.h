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

#ifndef __JUCE_WIN32_HIDDENMESSAGEWINDOW_JUCEHEADER__
#define __JUCE_WIN32_HIDDENMESSAGEWINDOW_JUCEHEADER__

//==============================================================================
class HiddenMessageWindow
{
public:
    HiddenMessageWindow (const TCHAR* const messageWindowName, WNDPROC wndProc)
    {
        String className ("JUCE_");
        className << String::toHexString (Time::getHighResolutionTicks());

        HMODULE moduleHandle = (HMODULE) Process::getCurrentModuleInstanceHandle();

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
class JuceWindowIdentifier
{
public:
    static bool isJUCEWindow (HWND hwnd) noexcept
    {
        return GetWindowLongPtr (hwnd, GWLP_USERDATA) == getImprobableWindowNumber();
    }

    static void setAsJUCEWindow (HWND hwnd, bool isJuceWindow) noexcept
    {
        SetWindowLongPtr (hwnd, GWLP_USERDATA, isJuceWindow ? getImprobableWindowNumber() : 0);
    }

private:
    static LONG_PTR getImprobableWindowNumber() noexcept
    {
        static LONG_PTR number = (LONG_PTR) Random::getSystemRandom().nextInt64();
        return number;
    }
};

//==============================================================================
class DeviceChangeDetector  : private Timer
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
            // We'll pause before sending a message, because on device removal, the OS hasn't always updated
            // its device lists correctly at this point. This also helps avoid repeated callbacks.
            ((DeviceChangeDetector*) GetWindowLongPtr (h, GWLP_USERDATA))->startTimer (500);
        }

        return DefWindowProc (h, message, wParam, lParam);
    }

    void timerCallback()
    {
        systemDeviceChanged();
    }
};

#endif   // __JUCE_WIN32_HIDDENMESSAGEWINDOW_JUCEHEADER__
