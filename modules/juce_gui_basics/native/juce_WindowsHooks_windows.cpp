/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if JUCE_WINDOWS

namespace juce::detail
{

class WindowsHooks::Hooks
{
public:
    Hooks() = default;

    ~Hooks()
    {
        if (mouseWheelHook != nullptr)
            UnhookWindowsHookEx (mouseWheelHook);

        if (keyboardHook != nullptr)
            UnhookWindowsHookEx (keyboardHook);
    }

    static inline std::weak_ptr<Hooks> weak;

private:
    static LRESULT CALLBACK mouseWheelHookCallback (int nCode, WPARAM wParam, LPARAM lParam)
    {
        if (nCode >= 0 && wParam == WM_MOUSEWHEEL)
        {
            // using a local copy of this struct to support old mingw libraries
            struct MOUSEHOOKSTRUCTEX_ final : public MOUSEHOOKSTRUCT  { DWORD mouseData; };

            auto& hs = *(MOUSEHOOKSTRUCTEX_*) lParam;

            if (auto* comp = Desktop::getInstance().findComponentAt ({ hs.pt.x, hs.pt.y }))
                if (comp->getWindowHandle() != nullptr)
                    return PostMessage ((HWND) comp->getWindowHandle(), WM_MOUSEWHEEL,
                                        hs.mouseData & 0xffff0000, (hs.pt.x & 0xffff) | (hs.pt.y << 16));
        }

        return CallNextHookEx (getSingleton()->mouseWheelHook, nCode, wParam, lParam);
    }

    static LRESULT CALLBACK keyboardHookCallback (int nCode, WPARAM wParam, LPARAM lParam)
    {
        auto& msg = *reinterpret_cast<MSG*> (lParam);

        if (nCode == HC_ACTION && wParam == PM_REMOVE && HWNDComponentPeer::offerKeyMessageToJUCEWindow (msg))
        {
            msg = {};
            msg.message = WM_USER;
            return 0;
        }

        return CallNextHookEx (getSingleton()->keyboardHook, nCode, wParam, lParam);
    }

    HHOOK mouseWheelHook = SetWindowsHookEx (WH_MOUSE,
                                             mouseWheelHookCallback,
                                             (HINSTANCE) juce::Process::getCurrentModuleInstanceHandle(),
                                             GetCurrentThreadId());
    HHOOK keyboardHook = SetWindowsHookEx (WH_GETMESSAGE,
                                           keyboardHookCallback,
                                           (HINSTANCE) juce::Process::getCurrentModuleInstanceHandle(),
                                           GetCurrentThreadId());
};

auto WindowsHooks::getSingleton() -> std::shared_ptr<Hooks>
{
    auto& weak = Hooks::weak;

    if (auto locked = weak.lock())
        return locked;

    auto strong = std::make_shared<Hooks>();
    weak = strong;
    return strong;
}

} // namespace juce::detail

#endif
