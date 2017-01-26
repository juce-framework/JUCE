/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_WIN32_HIDDENMESSAGEWINDOW_H_INCLUDED
#define JUCE_WIN32_HIDDENMESSAGEWINDOW_H_INCLUDED

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

    LPCTSTR getClassNameFromAtom() noexcept  { return (LPCTSTR) (pointer_sized_uint) atom; }
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

    virtual void systemDeviceChanged() = 0;

    void triggerAsyncDeviceChangeCallback()
    {
        // We'll pause before sending a message, because on device removal, the OS hasn't always updated
        // its device lists correctly at this point. This also helps avoid repeated callbacks.
        startTimer (500);
    }

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
            ((DeviceChangeDetector*) GetWindowLongPtr (h, GWLP_USERDATA))
                ->triggerAsyncDeviceChangeCallback();
        }

        return DefWindowProc (h, message, wParam, lParam);
    }

    void timerCallback() override
    {
        stopTimer();
        systemDeviceChanged();
    }
};

#endif   // JUCE_WIN32_HIDDENMESSAGEWINDOW_H_INCLUDED
