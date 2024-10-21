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

#pragma once

#include <juce_audio_plugin_client/detail/juce_IncludeModuleHeaders.h>

namespace juce::detail
{

struct PluginUtilities
{
    PluginUtilities() = delete;

    static int getDesktopFlags (const AudioProcessorEditor& editor)
    {
        return editor.wantsLayerBackedView()
             ? 0
             : ComponentPeer::windowRequiresSynchronousCoreGraphicsRendering;
    }

    static int getDesktopFlags (const AudioProcessorEditor* editor)
    {
        return editor != nullptr ? getDesktopFlags (*editor) : 0;
    }

    static void addToDesktop (AudioProcessorEditor& editor, void* parent)
    {
        editor.addToDesktop (getDesktopFlags (editor), parent);
    }

    static const PluginHostType& getHostType()
    {
        static PluginHostType hostType;
        return hostType;
    }

    // NB: Nasty old-fashioned code in here because it's copied from the Steinberg example code.
    static void getUUIDForVST2ID (bool forControllerUID, uint8 uuid[16])
    {
        #if JUCE_WINDOWS
         const auto juce_sprintf = [] (auto&& head, auto&&... tail) { sprintf_s (head, (size_t) numElementsInArray (head), tail...); };
         const auto juce_strcpy  = [] (auto&& head, auto&&... tail) { strcpy_s  (head, (size_t) numElementsInArray (head), tail...); };
         const auto juce_strcat  = [] (auto&& head, auto&&... tail) { strcat_s  (head, (size_t) numElementsInArray (head), tail...); };
         const auto juce_sscanf  = [] (auto&&... args)              { sscanf_s  (args...); };
        #else
         const auto juce_sprintf = [] (auto&& head, auto&&... tail) { snprintf  (head, (size_t) numElementsInArray (head), tail...); };
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

   #if JucePlugin_Build_VST
     static bool handleManufacturerSpecificVST2Opcode ([[maybe_unused]] int32 index,
                                                       [[maybe_unused]] pointer_sized_int value,
                                                       [[maybe_unused]] void* ptr,
                                                       float)
    {
       #if JUCE_VST3_CAN_REPLACE_VST2
        if ((index == (int32) ByteOrder::bigEndianInt ("stCA") || index == (int32) ByteOrder::bigEndianInt ("stCa"))
            && value == (int32) ByteOrder::bigEndianInt ("FUID") && ptr != nullptr)
        {
            uint8 fuid[16];
            getUUIDForVST2ID  (false, fuid);
            ::memcpy (ptr, fuid, 16);
            return true;
        }
       #endif
        return false;
    }
   #endif
};

} // namespace juce::detail
