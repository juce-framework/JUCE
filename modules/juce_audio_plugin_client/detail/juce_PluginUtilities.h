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
            const auto uidString = VST3ClientExtensions::convertVST2PluginId (JucePlugin_VSTUniqueID, JucePlugin_Name, VST3ClientExtensions::InterfaceType::component);
            MemoryBlock uidValue;
            uidValue.loadFromHexString (uidString);
            uidValue.copyTo (ptr, 0, uidValue.getSize());
            return true;
        }
       #endif
        return false;
    }
   #endif
};

} // namespace juce::detail
