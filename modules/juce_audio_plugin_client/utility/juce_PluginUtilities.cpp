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

#if _MSC_VER || defined (__MINGW32__) || defined (__MINGW64__)
 #include <windows.h>
#endif

#include <juce_core/system/juce_TargetPlatform.h>
#include "../utility/juce_CheckSettingMacros.h"
#include "juce_IncludeModuleHeaders.h"

namespace juce
{

#if JucePlugin_Build_Unity
 bool juce_isRunningInUnity()    { return PluginHostType::getPluginLoadedAs() == AudioProcessor::wrapperType_Unity; }
#endif

#ifndef JUCE_VST3_CAN_REPLACE_VST2
 #define JUCE_VST3_CAN_REPLACE_VST2 1
#endif

#if JucePlugin_Build_VST3 && JUCE_VST3_CAN_REPLACE_VST2 && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD)
 #define VST3_REPLACEMENT_AVAILABLE 1

 // NB: Nasty old-fashioned code in here because it's copied from the Steinberg example code.
 void getUUIDForVST2ID (bool forControllerUID, uint8 uuid[16])
 {
     #if JUCE_MSVC
      const auto juce_sprintf = [] (auto&& head, auto&&... tail) { sprintf_s (head, numElementsInArray (head), tail...); };
      const auto juce_strcpy  = [] (auto&& head, auto&&... tail) { strcpy_s  (head, numElementsInArray (head), tail...); };
      const auto juce_strcat  = [] (auto&& head, auto&&... tail) { strcat_s  (head, numElementsInArray (head), tail...); };
      const auto juce_sscanf  = [] (auto&&... args)              { sscanf_s  (args...); };
     #else
      const auto juce_sprintf = [] (auto&&... args)              { sprintf   (args...); };
      const auto juce_strcpy  = [] (auto&&... args)              { strcpy    (args...); };
      const auto juce_strcat  = [] (auto&&... args)              { strcat    (args...); };
      const auto juce_sscanf  = [] (auto&&... args)              { sscanf    (args...); };
     #endif

     char uidString[33];

     const int vstfxid = (('V' << 16) | ('S' << 8) | (forControllerUID ? 'E' : 'T'));
     char vstfxidStr[7] = { 0 };
     juce_sprintf (vstfxidStr, "%06X", vstfxid);

     juce_strcpy (uidString, vstfxidStr);

     char uidStr[9] = { 0 };
     juce_sprintf (uidStr, "%08X", JucePlugin_VSTUniqueID);
     juce_strcat (uidString, uidStr);

     char nameidStr[3] = { 0 };
     const size_t len = strlen (JucePlugin_Name);

     for (size_t i = 0; i <= 8; ++i)
     {
         juce::uint8 c = i < len ? static_cast<juce::uint8> (JucePlugin_Name[i]) : 0;

         if (c >= 'A' && c <= 'Z')
             c += 'a' - 'A';

         juce_sprintf (nameidStr, "%02X", c);
         juce_strcat (uidString, nameidStr);
     }

     unsigned long p0;
     unsigned int p1, p2;
     unsigned int p3[8];

     juce_sscanf (uidString, "%08lX%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
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
    #if ! JUCE_WINDOWS
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
     if ((index == (int32) ByteOrder::bigEndianInt ("stCA") || index == (int32) ByteOrder::bigEndianInt ("stCa"))
         && value == (int32) ByteOrder::bigEndianInt ("FUID") && ptr != nullptr)
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
