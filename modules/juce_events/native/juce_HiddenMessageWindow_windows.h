/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class HiddenMessageWindow
{
public:
    HiddenMessageWindow (const TCHAR* const messageWindowName, WNDPROC wndProc)
    {
        String className ("JUCE_");
        className << String::toHexString (Time::getHighResolutionTicks());

        HMODULE moduleHandle = (HMODULE) Process::getCurrentModuleInstanceHandle();

        WNDCLASSEX wc = {};
        wc.cbSize         = sizeof (wc);
        wc.lpfnWndProc    = wndProc;
        wc.cbWndExtra     = 4;
        wc.hInstance      = moduleHandle;
        wc.lpszClassName  = className.toWideCharPointer();

        atom = RegisterClassEx (&wc);
        jassert (atom != 0);

        hwnd = CreateWindow (getClassNameFromAtom(), messageWindowName,
                             0, 0, 0, 0, 0,
                             nullptr, nullptr, moduleHandle, nullptr);
        jassert (hwnd != nullptr);
    }

    ~HiddenMessageWindow()
    {
        DestroyWindow (hwnd);
        UnregisterClass (getClassNameFromAtom(), nullptr);
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
        static auto number = (LONG_PTR) Random().nextInt64();
        return number;
    }
};

//==============================================================================
class DeviceChangeDetector  : private Timer
{
public:
    DeviceChangeDetector (const wchar_t* const name, std::function<void()> onChangeIn)
        : messageWindow (name, (WNDPROC) deviceChangeEventCallback),
          onChange (std::move (onChangeIn))
    {
        SetWindowLongPtr (messageWindow.getHWND(), GWLP_USERDATA, (LONG_PTR) this);
    }

    void triggerAsyncDeviceChangeCallback()
    {
        // We'll pause before sending a message, because on device removal, the OS hasn't always updated
        // its device lists correctly at this point. This also helps avoid repeated callbacks.
        startTimer (500);
    }

private:
    HiddenMessageWindow messageWindow;
    std::function<void()> onChange;

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
        NullCheckedInvocation::invoke (onChange);
    }
};

} // namespace juce
