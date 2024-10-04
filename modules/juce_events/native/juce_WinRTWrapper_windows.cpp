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

WinRTWrapper::WinRTWrapper()
{
    winRTHandle = ::LoadLibraryA ("api-ms-win-core-winrt-l1-1-0");

    if (winRTHandle == nullptr)
        return;

    roInitialize           = (RoInitializeFuncPtr)              ::GetProcAddress (winRTHandle, "RoInitialize");
    createHString          = (WindowsCreateStringFuncPtr)       ::GetProcAddress (winRTHandle, "WindowsCreateString");
    deleteHString          = (WindowsDeleteStringFuncPtr)       ::GetProcAddress (winRTHandle, "WindowsDeleteString");
    getHStringRawBuffer    = (WindowsGetStringRawBufferFuncPtr) ::GetProcAddress (winRTHandle, "WindowsGetStringRawBuffer");
    roActivateInstance     = (RoActivateInstanceFuncPtr)        ::GetProcAddress (winRTHandle, "RoActivateInstance");
    roGetActivationFactory = (RoGetActivationFactoryFuncPtr)    ::GetProcAddress (winRTHandle, "RoGetActivationFactory");

    if (roInitialize == nullptr || createHString == nullptr || deleteHString == nullptr
        || getHStringRawBuffer == nullptr || roActivateInstance == nullptr || roGetActivationFactory == nullptr)
        return;

    HRESULT status = roInitialize (1);
    initialised = ! (status != S_OK && status != S_FALSE && status != (HRESULT) 0x80010106L);
}

WinRTWrapper::~WinRTWrapper()
{
    if (winRTHandle != nullptr)
        ::FreeLibrary (winRTHandle);

    clearSingletonInstance();
}

WinRTWrapper::ScopedHString::ScopedHString (String str)
{
    if (WinRTWrapper::getInstance()->isInitialised())
        WinRTWrapper::getInstance()->createHString (str.toWideCharPointer(),
                                                    static_cast<uint32_t> (str.length()),
                                                    &hstr);
}

WinRTWrapper::ScopedHString::~ScopedHString()
{
    if (WinRTWrapper::getInstance()->isInitialised() && hstr != nullptr)
        WinRTWrapper::getInstance()->deleteHString (hstr);
}

String WinRTWrapper::hStringToString (HSTRING hstr)
{
    if (isInitialised())
        if (const wchar_t* str = getHStringRawBuffer (hstr, nullptr))
            return String (str);

    return {};
}


JUCE_IMPLEMENT_SINGLETON (WinRTWrapper)

}
