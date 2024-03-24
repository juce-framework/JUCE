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
            {
                if (auto* target = static_cast<HWND> (comp->getWindowHandle()))
                {
                    const ScopedThreadDPIAwarenessSetter scope { target };
                    return PostMessage (target, WM_MOUSEWHEEL,
                                        hs.mouseData & 0xffff0000, MAKELPARAM (hs.pt.x, hs.pt.y));
                }
            }
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
