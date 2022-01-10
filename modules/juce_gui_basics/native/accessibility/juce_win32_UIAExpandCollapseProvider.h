/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
class UIAExpandCollapseProvider  : public UIAProviderBase,
                                   public ComBaseClassHelper<ComTypes::IExpandCollapseProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT Expand() override
    {
        return invokeShowMenu();
    }

    JUCE_COMRESULT Collapse() override
    {
        return invokeShowMenu();
    }

    JUCE_COMRESULT get_ExpandCollapseState (ComTypes::ExpandCollapseState* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getHandler().getCurrentState().isExpanded()
                           ? ComTypes::ExpandCollapseState_Expanded
                           : ComTypes::ExpandCollapseState_Collapsed;

            return S_OK;
        });
    }

private:
    JUCE_COMRESULT invokeShowMenu()
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (handler.getActions().invoke (AccessibilityActionType::showMenu))
        {
            sendAccessibilityAutomationEvent (handler, handler.getCurrentState().isExpanded()
                                                           ? ComTypes::UIA_MenuOpenedEventId
                                                           : ComTypes::UIA_MenuClosedEventId);

            return S_OK;
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAExpandCollapseProvider)
};

} // namespace juce
