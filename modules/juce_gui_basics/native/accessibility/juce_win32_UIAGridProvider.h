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
class UIAGridProvider  : public UIAProviderBase,
                         public ComBaseClassHelper<ComTypes::IGridProvider>
{
public:
    using UIAProviderBase::UIAProviderBase;

    //==============================================================================
    JUCE_COMRESULT GetItem (int row, int column, IRawElementProviderSimple** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            if (! isPositiveAndBelow (row, tableInterface.getNumRows())
                || ! isPositiveAndBelow (column, tableInterface.getNumColumns()))
                return E_INVALIDARG;

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

            if (auto* handler = tableInterface.getCellHandler (row, column))
                handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            return S_OK;
        });
    }

    JUCE_COMRESULT get_RowCount (int* pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            *pRetVal = tableInterface.getNumRows();
            return S_OK;
        });
    }

    JUCE_COMRESULT get_ColumnCount (int* pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            *pRetVal = tableInterface.getNumColumns();
            return S_OK;
        });
    }

private:
    template <typename Value, typename Callback>
    JUCE_COMRESULT withTableInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* tableInterface = getHandler().getTableInterface())
                return callback (*tableInterface);

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridProvider)
};

} // namespace juce
