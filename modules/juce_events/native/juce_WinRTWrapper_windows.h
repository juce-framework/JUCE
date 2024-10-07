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

class WinRTWrapper :   public DeletedAtShutdown
{
public:
    //==============================================================================
    ~WinRTWrapper();
    bool isInitialised() const noexcept  { return initialised; }

    JUCE_DECLARE_SINGLETON_INLINE (WinRTWrapper, false)

    //==============================================================================
    template <class ComClass>
    ComSmartPtr<ComClass> activateInstance (const wchar_t* runtimeClassID, REFCLSID classUUID)
    {
        ComSmartPtr<ComClass> result;

        if (isInitialised())
        {
            ComSmartPtr<IInspectable> inspectable;
            ScopedHString runtimeClass (runtimeClassID);
            auto hr = roActivateInstance (runtimeClass.get(), inspectable.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                inspectable->QueryInterface (classUUID, (void**) result.resetAndGetPointerAddress());
        }

        return result;
    }

    template <class ComClass>
    ComSmartPtr<ComClass> getWRLFactory (const wchar_t* runtimeClassID)
    {
        ComSmartPtr<ComClass> comPtr;

        if (isInitialised())
        {
            ScopedHString classID (runtimeClassID);

            if (classID.get() != nullptr)
                roGetActivationFactory (classID.get(), __uuidof (ComClass), (void**) comPtr.resetAndGetPointerAddress());
        }

        return comPtr;
    }

    //==============================================================================
    class ScopedHString
    {
    public:
        ScopedHString (String);
        ~ScopedHString();

        HSTRING get() const noexcept          { return hstr; }

    private:
        HSTRING hstr = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedHString)
    };

    String hStringToString (HSTRING);

private:
    WinRTWrapper();

    //==============================================================================
    HMODULE winRTHandle = nullptr;
    bool initialised = false;

    typedef HRESULT (WINAPI* RoInitializeFuncPtr) (int);
    typedef HRESULT (WINAPI* WindowsCreateStringFuncPtr) (LPCWSTR, UINT32, HSTRING*);
    typedef HRESULT (WINAPI* WindowsDeleteStringFuncPtr) (HSTRING);
    typedef PCWSTR  (WINAPI* WindowsGetStringRawBufferFuncPtr) (HSTRING, UINT32*);
    typedef HRESULT (WINAPI* RoActivateInstanceFuncPtr) (HSTRING, IInspectable**);
    typedef HRESULT (WINAPI* RoGetActivationFactoryFuncPtr) (HSTRING, REFIID, void**);

    RoInitializeFuncPtr roInitialize = nullptr;
    WindowsCreateStringFuncPtr createHString = nullptr;
    WindowsDeleteStringFuncPtr deleteHString = nullptr;
    WindowsGetStringRawBufferFuncPtr getHStringRawBuffer = nullptr;
    RoActivateInstanceFuncPtr roActivateInstance = nullptr;
    RoGetActivationFactoryFuncPtr roGetActivationFactory = nullptr;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WinRTWrapper)
};

} // namespace juce
