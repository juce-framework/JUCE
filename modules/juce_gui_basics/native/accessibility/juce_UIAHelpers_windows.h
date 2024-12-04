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

namespace VariantHelpers
{
    namespace Detail
    {
        template <typename Fn, typename ValueType>
        inline VARIANT getWithValueGeneric (Fn&& setter, ValueType value)
        {
            VARIANT result{};
            setter (value, &result);
            return result;
        }
    }

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

    inline VARIANT getWithValue (double value)        { return Detail::getWithValueGeneric (&setDouble, value); }
    inline VARIANT getWithValue (const String& value) { return Detail::getWithValueGeneric (&setString, value); }
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
