/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace VariantHelpers
{
    inline void clear (VARIANT* variant)
    {
        variant->vt = VT_EMPTY;
    }

    inline void setInt (int value, VARIANT* variant)
    {
        variant->vt   = VT_I4;
        variant->lVal = value;
    }

    inline void setBool (bool value, VARIANT* variant)
    {
        variant->vt      = VT_BOOL;
        variant->boolVal = value ? -1 : 0;
    }

    inline void setString (const String& value, VARIANT* variant)
    {
        variant->vt      = VT_BSTR;
        variant->bstrVal = SysAllocString ((const OLECHAR*) value.toWideCharPointer());
    }

    inline void setDouble (double value, VARIANT* variant)
    {
        variant->vt     = VT_R8;
        variant->dblVal = value;
    }
}

inline JUCE_COMRESULT addHandlersToArray (const std::vector<const AccessibilityHandler*>& handlers, SAFEARRAY** pRetVal)
{
    auto numHandlers = handlers.size();

    *pRetVal = SafeArrayCreateVector (VT_UNKNOWN, 0, (ULONG) numHandlers);

    if (pRetVal != nullptr)
    {
        for (LONG i = 0; i < (LONG) numHandlers; ++i)
        {
            auto* handler = handlers[(size_t) i];

            if (handler == nullptr)
                continue;

            ComSmartPtr<IRawElementProviderSimple> provider;
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            handler->getNativeImplementation()->QueryInterface (IID_PPV_ARGS (provider.resetAndGetPointerAddress()));
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            auto hr = SafeArrayPutElement (*pRetVal, &i, provider);

            if (FAILED (hr))
                return E_FAIL;
        }
    }

    return S_OK;
}

template <typename Value, typename Object, typename Callback>
inline JUCE_COMRESULT withCheckedComArgs (Value* pRetVal, Object& handle, Callback&& callback)
{
    if (pRetVal == nullptr)
        return E_INVALIDARG;

    *pRetVal = Value{};

    if (! handle.isElementValid())
        return (HRESULT) UIA_E_ELEMENTNOTAVAILABLE;

    return callback();
}

} // namespace juce
