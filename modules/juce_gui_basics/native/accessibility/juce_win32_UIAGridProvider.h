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

namespace juce
{

//==============================================================================
class UIAGridProvider  : public UIAProviderBase,
                         public ComBaseClassHelper<ComTypes::IGridProvider, ComTypes::ITableProvider>
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

            if (auto* cellHandler = tableInterface.getCellHandler (row, column))
            {
                cellHandler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
                return S_OK;
            }

            if (auto* rowHandler = tableInterface.getRowHandler (row))
            {
                rowHandler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (pRetVal));
                return S_OK;
            }

            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            return E_FAIL;
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

    JUCE_COMRESULT GetRowHeaders (SAFEARRAY**) override
    {
        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT GetColumnHeaders (SAFEARRAY** pRetVal) override
    {
        return withTableInterface (pRetVal, [&] (const AccessibilityTableInterface& tableInterface)
        {
            if (auto* header = tableInterface.getHeaderHandler())
            {
                const auto children = header->getChildren();

                *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, (ULONG) children.size());

                LONG index = 0;

                for (const auto& child : children)
                {
                    IRawElementProviderSimple* provider = nullptr;

                    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
                    if (child != nullptr)
                        child->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (&provider));
                    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

                    if (provider == nullptr)
                        return E_FAIL;

                    const auto hr = SafeArrayPutElement (*pRetVal, &index, provider);

                    if (FAILED (hr))
                        return E_FAIL;

                    ++index;
                }

                return S_OK;
            }

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    JUCE_COMRESULT get_RowOrColumnMajor (ComTypes::RowOrColumnMajor* pRetVal) override
    {
        *pRetVal = ComTypes::RowOrColumnMajor_RowMajor;
        return S_OK;
    }

private:
    template <typename Value, typename Callback>
    JUCE_COMRESULT withTableInterface (Value* pRetVal, Callback&& callback) const
    {
        return withCheckedComArgs (pRetVal, *this, [&]() -> HRESULT
        {
            if (auto* tableHandler = getEnclosingHandlerWithInterface (&getHandler(), &AccessibilityHandler::getTableInterface))
                if (auto* tableInterface = tableHandler->getTableInterface())
                    return callback (*tableInterface);

            return (HRESULT) UIA_E_NOTSUPPORTED;
        });
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIAGridProvider)
};

} // namespace juce
