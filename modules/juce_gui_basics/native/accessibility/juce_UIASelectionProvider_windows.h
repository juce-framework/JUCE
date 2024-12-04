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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

//==============================================================================
class UIASelectionItemProvider  : public UIAProviderBase,
                                  public ComBaseClassHelper<ISelectionItemProvider>
{
public:
    explicit UIASelectionItemProvider (AccessibilityNativeHandle* handle)
        : UIAProviderBase (handle),
          isRadioButton (getHandler().getRole() == AccessibilityRole::radioButton)
    {
    }

    //==============================================================================
    JUCE_COMRESULT AddToSelection() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        const auto& handler = getHandler();

        if (isRadioButton)
        {
            handler.getActions().invoke (AccessibilityActionType::press);
            sendAccessibilityAutomationEvent (handler, UIA_SelectionItem_ElementSelectedEventId);

            return S_OK;
        }

        handler.getActions().invoke (AccessibilityActionType::toggle);
        handler.getActions().invoke (AccessibilityActionType::press);

        return S_OK;
    }

    JUCE_COMRESULT get_IsSelected (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            const auto state = getHandler().getCurrentState();
            *pRetVal = isRadioButton ? state.isChecked() : state.isSelected();
            return S_OK;
        });
    }

    JUCE_COMRESULT get_SelectionContainer (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            if (! isRadioButton)
                if (auto* parent = getHandler().getParent())
                    parent->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    JUCE_COMRESULT RemoveFromSelection() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        if (! isRadioButton)
        {
            const auto& handler = getHandler();

            if (handler.getCurrentState().isSelected())
                getHandler().getActions().invoke (AccessibilityActionType::toggle);
        }

        return S_OK;
    }

    JUCE_COMRESULT Select() override
    {
        if (! isElementValid())
            return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

        AddToSelection();

        if (isElementValid() && ! isRadioButton)
        {
            const auto& handler = getHandler();

            if (auto* parent = handler.getParent())
                for (auto* child : parent->getChildren())
                    if (child != &handler && child->getCurrentState().isSelected())
                        child->getActions().invoke (AccessibilityActionType::toggle);
        }

        return S_OK;
    }

private:
    const bool isRadioButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIASelectionItemProvider)
};

//==============================================================================
class UIASelectionProvider  : public UIAProviderBase,
                              public ComBaseClassHelper<ISelectionProvider2>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT QueryInterface (REFIID iid, void** result) override
    {
        if (iid == __uuidof (IUnknown) || iid == __uuidof (ISelectionProvider))
            return castToType<ISelectionProvider> (result);

        if (iid == __uuidof (ISelectionProvider2))
            return castToType<ISelectionProvider2> (result);

        *result = nullptr;
        return E_NOINTERFACE;
    }

    //==============================================================================
    JUCE_COMRESULT get_CanSelectMultiple (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = isMultiSelectable();
            return S_OK;
        });
    }

    JUCE_COMRESULT get_IsSelectionRequired (BOOL* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = getSelectedChildren().size() > 0 && ! isMultiSelectable();
            return S_OK;
        });
    }

    JUCE_COMRESULT GetSelection (SAFEARRAY** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            return addHandlersToArray (getSelectedChildren(), pRetVal);
        });
    }

    //==============================================================================
    JUCE_COMRESULT get_FirstSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto selectedChildren = getSelectedChildren();

            if (! selectedChildren.empty())
                selectedChildren.front()->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    JUCE_COMRESULT get_LastSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            auto selectedChildren = getSelectedChildren();

            if (! selectedChildren.empty())
                selectedChildren.back()->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            return S_OK;
        });
    }

    JUCE_COMRESULT get_CurrentSelectedItem (IRawElementProviderSimple** pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            get_FirstSelectedItem (pRetVal);
            return S_OK;
        });
    }

    JUCE_COMRESULT get_ItemCount (int* pRetVal) override
    {
        return withCheckedComArgs (pRetVal, *this, [&]
        {
            *pRetVal = (int) getSelectedChildren().size();
            return S_OK;
        });
    }

private:
    bool isMultiSelectable() const noexcept
    {
         return getHandler().getCurrentState().isMultiSelectable();
    }

    std::vector<const AccessibilityHandler*> getSelectedChildren() const
    {
        std::vector<const AccessibilityHandler*> selectedHandlers;

        for (auto* child : getHandler().getComponent().getChildren())
            if (auto* handler = child->getAccessibilityHandler())
                if (handler->getCurrentState().isSelected())
                    selectedHandlers.push_back (handler);

        return selectedHandlers;
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIASelectionProvider)
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
