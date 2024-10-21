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
class UIAWindowProvider : public UIAProviderBase,
                          public ComBaseClassHelper<IWindowProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT SetVisualState (WindowVisualState state) override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            switch (state)
            {
                case WindowVisualState_Maximized:
                    peer->setFullScreen (true);
                    break;

                case WindowVisualState_Minimized:
                    peer->setMinimised (true);
                    break;

                case WindowVisualState_Normal:
                    peer->setFullScreen (false);
                    peer->setMinimised (false);
                    break;

                default:
                    break;
            }

            return S_OK;
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT Close() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        if (auto* peer = getPeer())
        {
            peer->handleUserClosingWindow();
            return S_OK;
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT WaitForInputIdle (int, BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, []
        {
            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_CanMaximize (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = (peer->getStyleFlags() & ComponentPeer::windowHasMaximiseButton) != 0;
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_CanMinimize (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = (peer->getStyleFlags() & ComponentPeer::windowHasMinimiseButton) != 0;
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_IsModal (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = peer->getComponent().isCurrentlyModal();
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_WindowVisualState (WindowVisualState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                if (peer->isFullScreen())
                    *pRetVal = WindowVisualState_Maximized;
                else if (peer->isMinimised())
                    *pRetVal = WindowVisualState_Minimized;
                else
                    *pRetVal = WindowVisualState_Normal;

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_WindowInteractionState (WindowInteractionState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = peer->getComponent().isCurrentlyBlockedByAnotherModalComponent()
                    ? WindowInteractionState::WindowInteractionState_BlockedByModalWindow
                    : WindowInteractionState::WindowInteractionState_Running;

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_IsTopmost (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* peer = getPeer())
            {
                *pRetVal = peer->isFocused();
                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

private:
    ComponentPeer* getPeer() const
    {
        return getHandler().getComponent().getPeer();
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAWindowProvider)
};

} // namespace juce
