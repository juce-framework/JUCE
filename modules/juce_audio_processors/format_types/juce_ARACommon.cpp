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

#if (JUCE_PLUGINHOST_ARA && (JUCE_PLUGINHOST_VST3 || JUCE_PLUGINHOST_AU) && (JUCE_MAC || JUCE_WINDOWS))

#include <ARA_Library/Debug/ARADebug.h>

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

static std::shared_ptr<const ARA::ARAFactory> getOrCreateARAFactory (const ARA::ARAFactory* ptr,
                                                                     std::function<void (const ARA::ARAFactory*)> onDelete)
{
    JUCE_ASSERT_MESSAGE_THREAD

    static std::unordered_map<const ARA::ARAFactory*, std::weak_ptr<const ARA::ARAFactory>> cache;

    auto& cachePtr = cache[ptr];

    if (const auto obj = cachePtr.lock())
        return obj;

    const auto interfaceConfig = createInterfaceConfig (ptr);
    ptr->initializeARAWithConfiguration (&interfaceConfig);
    const auto obj = std::shared_ptr<const ARA::ARAFactory> (ptr, [deleter = std::move (onDelete)] (const ARA::ARAFactory* factory)
                                                             {
                                                                 factory->uninitializeARA();
                                                                 deleter (factory);
                                                             });
    cachePtr = obj;
    return obj;
}

}

#endif
