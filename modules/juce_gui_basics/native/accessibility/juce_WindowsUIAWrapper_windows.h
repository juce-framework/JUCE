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

class WindowsUIAWrapper  : public DeletedAtShutdown
{
public:
    bool isLoaded() const noexcept
    {
        return uiaReturnRawElementProvider            != nullptr
            && uiaHostProviderFromHwnd                != nullptr
            && uiaRaiseAutomationPropertyChangedEvent != nullptr
            && uiaRaiseAutomationEvent                != nullptr
            && uiaClientsAreListening                 != nullptr
            && uiaDisconnectProvider                  != nullptr
            && uiaDisconnectAllProviders              != nullptr;
    }

    //==============================================================================
    LRESULT returnRawElementProvider (HWND hwnd, WPARAM wParam, LPARAM lParam, IRawElementProviderSimple* provider)
    {
        return uiaReturnRawElementProvider != nullptr ? uiaReturnRawElementProvider (hwnd, wParam, lParam, provider)
                                                      : (LRESULT) nullptr;
    }

    JUCE_COMRESULT hostProviderFromHwnd (HWND hwnd, IRawElementProviderSimple** provider)
    {
        return uiaHostProviderFromHwnd != nullptr ? uiaHostProviderFromHwnd (hwnd, provider)
                                                  : (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT raiseAutomationPropertyChangedEvent (IRawElementProviderSimple* provider, PROPERTYID propID, VARIANT oldValue, VARIANT newValue)
    {
        return uiaRaiseAutomationPropertyChangedEvent != nullptr ? uiaRaiseAutomationPropertyChangedEvent (provider, propID, oldValue, newValue)
                                                                 : (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT raiseAutomationEvent (IRawElementProviderSimple* provider, EVENTID eventID)
    {
        return uiaRaiseAutomationEvent != nullptr ? uiaRaiseAutomationEvent (provider, eventID)
                                                  : (HRESULT) UIA_E_NOTSUPPORTED;
    }

    BOOL clientsAreListening()
    {
        return uiaClientsAreListening != nullptr ? uiaClientsAreListening()
                                                 : false;
    }

    JUCE_COMRESULT disconnectProvider (IRawElementProviderSimple* provider)
    {
        if (uiaDisconnectProvider != nullptr)
        {
            const ScopedValueSetter<IRawElementProviderSimple*> disconnectingProviderSetter (disconnectingProvider, provider);
            return uiaDisconnectProvider (provider);
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    JUCE_COMRESULT disconnectAllProviders()
    {
        if (uiaDisconnectAllProviders != nullptr)
        {
            const ScopedValueSetter<bool> disconnectingAllProvidersSetter (disconnectingAllProviders, true);
            return uiaDisconnectAllProviders();
        }

        return (HRESULT) UIA_E_NOTSUPPORTED;
    }

    //==============================================================================
    bool isProviderDisconnecting (IRawElementProviderSimple* provider)
    {
        return disconnectingProvider == provider || disconnectingAllProviders;
    }

    //==============================================================================
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (WindowsUIAWrapper)

private:
    //==============================================================================
    WindowsUIAWrapper()
    {
        // force UIA COM library initialisation here to prevent an exception when calling methods from SendMessage()
        if (isLoaded())
            returnRawElementProvider (nullptr, 0, 0, nullptr);
        else
            jassertfalse; // UIAutomationCore could not be loaded!
    }

    ~WindowsUIAWrapper()
    {
        disconnectAllProviders();

        if (uiaHandle != nullptr)
            ::FreeLibrary (uiaHandle);

        clearSingletonInstance();
    }

    //==============================================================================
    template <typename FuncType>
    static FuncType getUiaFunction (HMODULE module, LPCSTR funcName)
    {
        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wcast-function-type")
        return (FuncType) GetProcAddress (module, funcName);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    //==============================================================================
    using UiaReturnRawElementProviderFunc            = LRESULT (WINAPI*) (HWND, WPARAM, LPARAM, IRawElementProviderSimple*);
    using UiaHostProviderFromHwndFunc                = HRESULT (WINAPI*) (HWND, IRawElementProviderSimple**);
    using UiaRaiseAutomationPropertyChangedEventFunc = HRESULT (WINAPI*) (IRawElementProviderSimple*, PROPERTYID, VARIANT, VARIANT);
    using UiaRaiseAutomationEventFunc                = HRESULT (WINAPI*) (IRawElementProviderSimple*, EVENTID);
    using UiaClientsAreListeningFunc                 = BOOL    (WINAPI*) ();
    using UiaDisconnectProviderFunc                  = HRESULT (WINAPI*) (IRawElementProviderSimple*);
    using UiaDisconnectAllProvidersFunc              = HRESULT (WINAPI*) ();

    HMODULE uiaHandle = ::LoadLibraryA ("UIAutomationCore.dll");
    UiaReturnRawElementProviderFunc            uiaReturnRawElementProvider            = getUiaFunction<UiaReturnRawElementProviderFunc>            (uiaHandle, "UiaReturnRawElementProvider");
    UiaHostProviderFromHwndFunc                uiaHostProviderFromHwnd                = getUiaFunction<UiaHostProviderFromHwndFunc>                (uiaHandle, "UiaHostProviderFromHwnd");
    UiaRaiseAutomationPropertyChangedEventFunc uiaRaiseAutomationPropertyChangedEvent = getUiaFunction<UiaRaiseAutomationPropertyChangedEventFunc> (uiaHandle, "UiaRaiseAutomationPropertyChangedEvent");
    UiaRaiseAutomationEventFunc                uiaRaiseAutomationEvent                = getUiaFunction<UiaRaiseAutomationEventFunc>                (uiaHandle, "UiaRaiseAutomationEvent");
    UiaClientsAreListeningFunc                 uiaClientsAreListening                 = getUiaFunction<UiaClientsAreListeningFunc>                 (uiaHandle, "UiaClientsAreListening");
    UiaDisconnectProviderFunc                  uiaDisconnectProvider                  = getUiaFunction<UiaDisconnectProviderFunc>                  (uiaHandle, "UiaDisconnectProvider");
    UiaDisconnectAllProvidersFunc              uiaDisconnectAllProviders              = getUiaFunction<UiaDisconnectAllProvidersFunc>              (uiaHandle, "UiaDisconnectAllProviders");

    IRawElementProviderSimple* disconnectingProvider = nullptr;
    bool disconnectingAllProviders = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WindowsUIAWrapper)
};

} // namespace juce
