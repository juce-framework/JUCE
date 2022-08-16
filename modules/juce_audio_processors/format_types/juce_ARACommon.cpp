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

#if (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU) && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX))

namespace juce
{

static void dummyARAInterfaceAssert (ARA::ARAAssertCategory, const void*, const char*)
{}

static ARA::ARAInterfaceConfiguration createInterfaceConfig (const ARA::ARAFactory* araFactory)
{
    static auto* assertFunction = &dummyARAInterfaceAssert;

   #if ARA_VALIDATE_API_CALLS
    assertFunction = &::ARA::ARAInterfaceAssert;
    static std::once_flag flag;
    std::call_once (flag, [] { ARA::ARASetExternalAssertReference (&assertFunction); });
   #endif

    return makeARASizedStruct (&ARA::ARAInterfaceConfiguration::assertFunctionAddress,
                               jmin (araFactory->highestSupportedApiGeneration, (ARA::ARAAPIGeneration) ARA::kARAAPIGeneration_2_X_Draft),
                               &assertFunction);
}

/*  If the provided ARAFactory is not yet in use it constructs a new shared_ptr that will call the
    provided onDelete function inside the custom deleter of this new shared_ptr instance.

    The onDelete function is responsible for releasing the resources that guarantee the validity of
    the wrapped ARAFactory*.

    If however the ARAFactory is already in use the function will just return a copy of the already
    existing shared_ptr and call the onDelete function immediately. This is to ensure that the
    ARAFactory is only uninitialised when no plugin instance can be using it.

    On both platforms the onDelete function is used to release resources that ensure that the module
    providing the ARAFactory* remains loaded.
*/
static std::shared_ptr<const ARA::ARAFactory> getOrCreateARAFactory (const ARA::ARAFactory* ptr,
                                                                     std::function<void()> onDelete)
{
    JUCE_ASSERT_MESSAGE_THREAD

    static std::unordered_map<const ARA::ARAFactory*, std::weak_ptr<const ARA::ARAFactory>> cache;

    auto& cachePtr = cache[ptr];

    if (const auto obj = cachePtr.lock())
    {
        onDelete();
        return obj;
    }

    const auto interfaceConfig = createInterfaceConfig (ptr);
    ptr->initializeARAWithConfiguration (&interfaceConfig);
    const auto obj = std::shared_ptr<const ARA::ARAFactory> (ptr, [deleter = std::move (onDelete)] (const ARA::ARAFactory* factory)
                                                             {
                                                                 factory->uninitializeARA();
                                                                 deleter();
                                                             });
    cachePtr = obj;
    return obj;
}

}

#endif
