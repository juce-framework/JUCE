/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#if _MSC_VER || defined (__MINGW32__) || defined (__MINGW64__)
 #include <windows.h>
#endif

#include "../../juce_core/system/juce_TargetPlatform.h"
#include "../utility/juce_CheckSettingMacros.h"
#include "juce_IncludeModuleHeaders.h"

using namespace juce;

namespace juce
{

AudioProcessor::WrapperType PluginHostType::jucePlugInClientCurrentWrapperType = AudioProcessor::wrapperType_Undefined;

#ifndef JUCE_VST3_CAN_REPLACE_VST2
 #define JUCE_VST3_CAN_REPLACE_VST2 1
#endif

#if JucePlugin_Build_VST3 && (__APPLE_CPP__ || __APPLE_CC__ || _WIN32 || _WIN64) && JUCE_VST3_CAN_REPLACE_VST2
#define VST3_REPLACEMENT_AVAILABLE 1

// NB: Nasty old-fashioned code in here because it's copied from the Steinberg example code.
void JUCE_API getUUIDForVST2ID (bool forControllerUID, uint8 uuid[16])
{
    char uidString[33];

    const int vstfxid = (('V' << 16) | ('S' << 8) | (forControllerUID ? 'E' : 'T'));
    char vstfxidStr[7] = { 0 };
    sprintf (vstfxidStr, "%06X", vstfxid);

    strcpy (uidString, vstfxidStr);

    char uidStr[9] = { 0 };
    sprintf (uidStr, "%08X", JucePlugin_VSTUniqueID);
    strcat (uidString, uidStr);

    char nameidStr[3] = { 0 };
    const size_t len = strlen (JucePlugin_Name);

    for (size_t i = 0; i <= 8; ++i)
    {
        juce::uint8 c = i < len ? static_cast<juce::uint8> (JucePlugin_Name[i]) : 0;

        if (c >= 'A' && c <= 'Z')
            c += 'a' - 'A';

        sprintf (nameidStr, "%02X", c);
        strcat (uidString, nameidStr);
    }

    unsigned long p0;
    unsigned int p1, p2;
    unsigned int p3[8];

   #ifndef _MSC_VER
    sscanf
   #else
    sscanf_s
   #endif
    (uidString, "%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
     &p0, &p1, &p2, &p3[0], &p3[1], &p3[2], &p3[3], &p3[4], &p3[5], &p3[6], &p3[7]);

    union q0_u {
        uint32 word;
        uint8 bytes[4];
    } q0;

    union q1_u {
        uint16 half;
        uint8 bytes[2];
    } q1, q2;

    q0.word = static_cast<uint32> (p0);
    q1.half = static_cast<uint16> (p1);
    q2.half = static_cast<uint16> (p2);

    // VST3 doesn't use COM compatible UUIDs on non windows platforms
   #ifndef _WIN32
    q0.word = ByteOrder::swap (q0.word);
    q1.half = ByteOrder::swap (q1.half);
    q2.half = ByteOrder::swap (q2.half);
   #endif

    for (int i = 0; i < 4; ++i)
        uuid[i+0] = q0.bytes[i];

    for (int i = 0; i < 2; ++i)
        uuid[i+4] = q1.bytes[i];

    for (int i = 0; i < 2; ++i)
        uuid[i+6] = q2.bytes[i];

    for (int i = 0; i < 8; ++i)
        uuid[i+8] = static_cast<uint8> (p3[i]);
}
#else
#define VST3_REPLACEMENT_AVAILABLE 0
#endif

#if JucePlugin_Build_VST
bool JUCE_API handleManufacturerSpecificVST2Opcode (int32 index, pointer_sized_int value, void* ptr, float)
{
   #if VST3_REPLACEMENT_AVAILABLE
    if ((index == 'stCA' || index == 'stCa') && value == 'FUID' && ptr != nullptr)
    {
        uint8 fuid[16];
        getUUIDForVST2ID  (false, fuid);
        ::memcpy (ptr, fuid, 16);
        return true;
    }
   #else
    ignoreUnused (index, value, ptr);
   #endif
    return false;
}
#endif

} // namespace juce

//==============================================================================
/** Somewhere in the codebase of your plugin, you need to implement this function
    and make it return a new instance of the filter subclass that you're building.
*/
extern AudioProcessor* JUCE_CALLTYPE createPluginFilter();

#if JucePlugin_Enable_IAA && JucePlugin_Build_Standalone && JUCE_IOS && (! JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP)
extern bool JUCE_CALLTYPE juce_isInterAppAudioConnected();
extern void JUCE_CALLTYPE juce_switchToHostApplication();

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
extern Image JUCE_CALLTYPE juce_getIAAHostIcon (int);
#endif
#endif

AudioProcessor* JUCE_API JUCE_CALLTYPE createPluginFilterOfType (AudioProcessor::WrapperType type)
{
    AudioProcessor::setTypeOfNextNewPlugin (type);
    AudioProcessor* const pluginInstance = createPluginFilter();
    AudioProcessor::setTypeOfNextNewPlugin (AudioProcessor::wrapperType_Undefined);

    // your createPluginFilter() method must return an object!
    jassert (pluginInstance != nullptr && pluginInstance->wrapperType == type);

    return pluginInstance;
}

bool PluginHostType::isInterAppAudioConnected() const
{
   #if JucePlugin_Enable_IAA && JucePlugin_Build_Standalone && JUCE_IOS && (! JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP)
    if (getPluginLoadedAs() == AudioProcessor::wrapperType_Standalone)
        return juce_isInterAppAudioConnected();
   #endif

    return false;
}

void PluginHostType::switchToHostApplication() const
{
   #if JucePlugin_Enable_IAA && JucePlugin_Build_Standalone && JUCE_IOS && (! JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP)
    if (getPluginLoadedAs() == AudioProcessor::wrapperType_Standalone)
        juce_switchToHostApplication();
   #endif
}

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
namespace juce {

extern Image JUCE_API getIconFromApplication (const String&, const int);

Image PluginHostType::getHostIcon (int size) const
{
    ignoreUnused (size);

   #if JucePlugin_Enable_IAA && JucePlugin_Build_Standalone && JUCE_IOS && (! JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP)
    if (isInterAppAudioConnected())
        return juce_getIAAHostIcon (size);
   #endif

   #if JUCE_MAC
    String bundlePath (getHostPath().upToLastOccurrenceOf (".app", true, true));
    return getIconFromApplication (bundlePath, size);
   #endif

    return Image();
}

}
#endif
