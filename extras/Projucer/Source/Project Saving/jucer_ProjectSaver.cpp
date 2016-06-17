/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_ProjectExporter.h"
#include "jucer_ProjectSaver.h"

//==============================================================================
namespace
{
    inline int countMaxPluginChannels (const String& configString, bool isInput)
    {
        StringArray configs;
        configs.addTokens (configString, ", {}", StringRef());
        configs.trim();
        configs.removeEmptyStrings();
        jassert ((configs.size() & 1) == 0);  // looks like a syntax error in the configs?

        int maxVal = 0;
        for (int i = (isInput ? 0 : 1); i < configs.size(); i += 2)
            maxVal = jmax (maxVal, configs[i].getIntValue());

        return maxVal;
    }

    inline String valueToBool (const Value& v)
    {
        return static_cast<bool> (v.getValue()) ? "1" : "0";
    }

    inline String valueToStringLiteral (const var& v)
    {
        return CppTokeniserFunctions::addEscapeChars (v.toString()).quoted();
    }

    inline String valueToCharLiteral (const var& v)
    {
        String fourCharCode = v.toString().trim().substring (0, 4);
        uint32 hexRepresentation = 0;

        for (int i = 0; i < 4; ++i)
            hexRepresentation = (hexRepresentation << 8U)
                             |  (static_cast<unsigned int> (fourCharCode[i]) & 0xffU);

        return String ("0x") + String::toHexString (static_cast<int> (hexRepresentation))
                             + String (" // ")
                             + CppTokeniserFunctions::addEscapeChars (fourCharCode).quoted ('\'');
    }
}

//==============================================================================
void ProjectSaver::writePluginCharacteristicsFile()
{
    StringPairArray flags;
    flags.set ("JucePlugin_Build_VST",                   valueToBool (project.shouldBuildVST()));
    flags.set ("JucePlugin_Build_VST3",                  valueToBool (project.shouldBuildVST3()));
    flags.set ("JucePlugin_Build_AU",                    valueToBool (project.shouldBuildAU()));
    flags.set ("JucePlugin_Build_AUv3",                  valueToBool (project.shouldBuildAUv3()));
    flags.set ("JucePlugin_Build_RTAS",                  valueToBool (project.shouldBuildRTAS()));
    flags.set ("JucePlugin_Build_AAX",                   valueToBool (project.shouldBuildAAX()));
    flags.set ("JucePlugin_Build_STANDALONE",            valueToBool (project.shouldBuildStandalone()));
    flags.set ("JucePlugin_Name",                        valueToStringLiteral (project.getPluginName()));
    flags.set ("JucePlugin_Desc",                        valueToStringLiteral (project.getPluginDesc()));
    flags.set ("JucePlugin_Manufacturer",                valueToStringLiteral (project.getPluginManufacturer()));
    flags.set ("JucePlugin_ManufacturerWebsite",         valueToStringLiteral (project.getCompanyWebsite()));
    flags.set ("JucePlugin_ManufacturerEmail",           valueToStringLiteral (project.getCompanyEmail()));
    flags.set ("JucePlugin_ManufacturerCode",            valueToCharLiteral (project.getPluginManufacturerCode()));
    flags.set ("JucePlugin_PluginCode",                  valueToCharLiteral (project.getPluginCode()));
    flags.set ("JucePlugin_IsSynth",                     valueToBool (project.getPluginIsSynth()));
    flags.set ("JucePlugin_WantsMidiInput",              valueToBool (project.getPluginWantsMidiInput()));
    flags.set ("JucePlugin_ProducesMidiOutput",          valueToBool (project.getPluginProducesMidiOut()));
    flags.set ("JucePlugin_IsMidiEffect",                valueToBool (project.getPluginIsMidiEffectPlugin()));
    flags.set ("JucePlugin_EditorRequiresKeyboardFocus", valueToBool (project.getPluginEditorNeedsKeyFocus()));
    flags.set ("JucePlugin_Version",                     project.getVersionString());
    flags.set ("JucePlugin_VersionCode",                 project.getVersionAsHex());
    flags.set ("JucePlugin_VersionString",               valueToStringLiteral (project.getVersionString()));
    flags.set ("JucePlugin_VSTUniqueID",                 "JucePlugin_PluginCode");
    flags.set ("JucePlugin_VSTCategory",                 project.getPluginVSTCategoryString());
    flags.set ("JucePlugin_AUMainType",                  project.getAUMainTypeString());
    flags.set ("JucePlugin_AUSubType",                   "JucePlugin_PluginCode");
    flags.set ("JucePlugin_AUExportPrefix",              project.getPluginAUExportPrefix().toString());
    flags.set ("JucePlugin_AUExportPrefixQuoted",        valueToStringLiteral (project.getPluginAUExportPrefix()));
    flags.set ("JucePlugin_AUManufacturerCode",          "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_CFBundleIdentifier",          project.getBundleIdentifier().toString());
    flags.set ("JucePlugin_RTASCategory",                project.getPluginRTASCategoryCode());
    flags.set ("JucePlugin_RTASManufacturerCode",        "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_RTASProductId",               "JucePlugin_PluginCode");
    flags.set ("JucePlugin_RTASDisableBypass",           valueToBool (project.getPluginRTASBypassDisabled()));
    flags.set ("JucePlugin_RTASDisableMultiMono",        valueToBool (project.getPluginRTASMultiMonoDisabled()));
    flags.set ("JucePlugin_AAXIdentifier",               project.getAAXIdentifier().toString());
    flags.set ("JucePlugin_AAXManufacturerCode",         "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_AAXProductId",                "JucePlugin_PluginCode");
    flags.set ("JucePlugin_AAXCategory",                 project.getPluginAAXCategory().toString());
    flags.set ("JucePlugin_AAXDisableBypass",            valueToBool (project.getPluginAAXBypassDisabled()));
    flags.set ("JucePlugin_AAXDisableMultiMono",         valueToBool (project.getPluginAAXMultiMonoDisabled()));

    {
        String plugInChannelConfig = project.getPluginChannelConfigs().toString();

        if (plugInChannelConfig.isNotEmpty())
        {
            flags.set ("JucePlugin_MaxNumInputChannels",         String (countMaxPluginChannels (plugInChannelConfig, true)));
            flags.set ("JucePlugin_MaxNumOutputChannels",        String (countMaxPluginChannels (plugInChannelConfig, false)));
            flags.set ("JucePlugin_PreferredChannelConfigurations", plugInChannelConfig);
        }
    }

    MemoryOutputStream mem;

    mem << "//==============================================================================" << newLine
        << "// Audio plugin settings.." << newLine
        << newLine;

    for (int i = 0; i < flags.size(); ++i)
    {
        mem << "#ifndef  " << flags.getAllKeys()[i] << newLine
            << " #define " << flags.getAllKeys()[i].paddedRight (' ', 32) << "  "
                           << flags.getAllValues()[i] << newLine
            << "#endif" << newLine;
    }

    setExtraAppConfigFileContent (mem.toString());
}
