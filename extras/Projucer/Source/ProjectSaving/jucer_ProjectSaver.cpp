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

#include "../Application/jucer_Headers.h"
#include "jucer_ProjectSaver.h"

#include "jucer_ProjectExport_CLion.h"

//==============================================================================
namespace
{
    inline int countMaxPluginChannels (const String& configString, bool isInput)
    {
        auto configs = StringArray::fromTokens (configString, ", {}", {});
        configs.trim();
        configs.removeEmptyStrings();
        jassert ((configs.size() & 1) == 0);  // looks like a syntax error in the configs?

        int maxVal = 0;

        for (int i = (isInput ? 0 : 1); i < configs.size(); i += 2)
            maxVal = jmax (maxVal, configs[i].getIntValue());

        return maxVal;
    }

    inline String boolToString (bool b)
    {
        return b ? "1" : "0";
    }

    inline String toStringLiteral (const String& v)
    {
        return CppTokeniserFunctions::addEscapeChars (v).quoted();
    }

    inline String toCharLiteral (const String& v)
    {
        auto fourCharCode = v.substring (0, 4);
        uint32 hexRepresentation = 0;

        for (int i = 0; i < 4; ++i)
            hexRepresentation = (hexRepresentation << 8u)
                             |  (static_cast<unsigned int> (fourCharCode[i]) & 0xffu);

        return "0x" + String::toHexString (static_cast<int> (hexRepresentation))
                 + " // "
                 + CppTokeniserFunctions::addEscapeChars (fourCharCode).quoted ('\'');
    }
}

//==============================================================================
void ProjectSaver::writePluginCharacteristicsFile()
{
    StringPairArray flags;
    flags.set ("JucePlugin_Build_VST",                   boolToString (project.shouldBuildVST()));
    flags.set ("JucePlugin_Build_VST3",                  boolToString (project.shouldBuildVST3()));
    flags.set ("JucePlugin_Build_AU",                    boolToString (project.shouldBuildAU()));
    flags.set ("JucePlugin_Build_AUv3",                  boolToString (project.shouldBuildAUv3()));
    flags.set ("JucePlugin_Build_RTAS",                  boolToString (project.shouldBuildRTAS()));
    flags.set ("JucePlugin_Build_AAX",                   boolToString (project.shouldBuildAAX()));
    flags.set ("JucePlugin_Build_Standalone",            boolToString (project.shouldBuildStandalonePlugin()));
    flags.set ("JucePlugin_Build_Unity",                 boolToString (project.shouldBuildUnityPlugin()));
    flags.set ("JucePlugin_Enable_IAA",                  boolToString (project.shouldEnableIAA()));
    flags.set ("JucePlugin_Name",                        toStringLiteral (project.getPluginNameString()));
    flags.set ("JucePlugin_Desc",                        toStringLiteral (project.getPluginDescriptionString()));
    flags.set ("JucePlugin_Manufacturer",                toStringLiteral (project.getPluginManufacturerString()));
    flags.set ("JucePlugin_ManufacturerWebsite",         toStringLiteral (project.getCompanyWebsiteString()));
    flags.set ("JucePlugin_ManufacturerEmail",           toStringLiteral (project.getCompanyEmailString()));
    flags.set ("JucePlugin_ManufacturerCode",            toCharLiteral (project.getPluginManufacturerCodeString()));
    flags.set ("JucePlugin_PluginCode",                  toCharLiteral (project.getPluginCodeString()));
    flags.set ("JucePlugin_IsSynth",                     boolToString (project.isPluginSynth()));
    flags.set ("JucePlugin_WantsMidiInput",              boolToString (project.pluginWantsMidiInput()));
    flags.set ("JucePlugin_ProducesMidiOutput",          boolToString (project.pluginProducesMidiOutput()));
    flags.set ("JucePlugin_IsMidiEffect",                boolToString (project.isPluginMidiEffect()));
    flags.set ("JucePlugin_EditorRequiresKeyboardFocus", boolToString (project.pluginEditorNeedsKeyFocus()));
    flags.set ("JucePlugin_Version",                     project.getVersionString());
    flags.set ("JucePlugin_VersionCode",                 project.getVersionAsHex());
    flags.set ("JucePlugin_VersionString",               toStringLiteral (project.getVersionString()));
    flags.set ("JucePlugin_VSTUniqueID",                 "JucePlugin_PluginCode");
    flags.set ("JucePlugin_VSTCategory",                 project.getVSTCategoryString());
    flags.set ("JucePlugin_Vst3Category",                toStringLiteral (project.getVST3CategoryString()));
    flags.set ("JucePlugin_AUMainType",                  project.getAUMainTypeString());
    flags.set ("JucePlugin_AUSubType",                   "JucePlugin_PluginCode");
    flags.set ("JucePlugin_AUExportPrefix",              project.getPluginAUExportPrefixString());
    flags.set ("JucePlugin_AUExportPrefixQuoted",        toStringLiteral (project.getPluginAUExportPrefixString()));
    flags.set ("JucePlugin_AUManufacturerCode",          "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_CFBundleIdentifier",          project.getBundleIdentifierString());
    flags.set ("JucePlugin_RTASCategory",                String (project.getRTASCategory()));
    flags.set ("JucePlugin_RTASManufacturerCode",        "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_RTASProductId",               "JucePlugin_PluginCode");
    flags.set ("JucePlugin_RTASDisableBypass",           boolToString (project.isPluginRTASBypassDisabled()));
    flags.set ("JucePlugin_RTASDisableMultiMono",        boolToString (project.isPluginRTASMultiMonoDisabled()));
    flags.set ("JucePlugin_AAXIdentifier",               project.getAAXIdentifierString());
    flags.set ("JucePlugin_AAXManufacturerCode",         "JucePlugin_ManufacturerCode");
    flags.set ("JucePlugin_AAXProductId",                "JucePlugin_PluginCode");
    flags.set ("JucePlugin_AAXCategory",                 String (project.getAAXCategory()));
    flags.set ("JucePlugin_AAXDisableBypass",            boolToString (project.isPluginAAXBypassDisabled()));
    flags.set ("JucePlugin_AAXDisableMultiMono",         boolToString (project.isPluginAAXMultiMonoDisabled()));
    flags.set ("JucePlugin_IAAType",                     toCharLiteral (project.getIAATypeCode()));
    flags.set ("JucePlugin_IAASubType",                  "JucePlugin_PluginCode");
    flags.set ("JucePlugin_IAAName",                     project.getIAAPluginName().quoted());
    flags.set ("JucePlugin_VSTNumMidiInputs",            project.getVSTNumMIDIInputsString());
    flags.set ("JucePlugin_VSTNumMidiOutputs",           project.getVSTNumMIDIOutputsString());

    {
        String plugInChannelConfig = project.getPluginChannelConfigsString();

        if (plugInChannelConfig.isNotEmpty())
        {
            flags.set ("JucePlugin_MaxNumInputChannels",            String (countMaxPluginChannels (plugInChannelConfig, true)));
            flags.set ("JucePlugin_MaxNumOutputChannels",           String (countMaxPluginChannels (plugInChannelConfig, false)));
            flags.set ("JucePlugin_PreferredChannelConfigurations", plugInChannelConfig);
        }
    }

    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

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

void ProjectSaver::writeProjects (const OwnedArray<LibraryModule>& modules, const String& specifiedExporterToSave, bool isCommandLineApp)
{
    ThreadPool threadPool;

    // keep a copy of the basic generated files group, as each exporter may modify it.
    auto originalGeneratedGroup = generatedFilesGroup.state.createCopy();

    CLionProjectExporter* clionExporter = nullptr;
    OwnedArray<ProjectExporter> exporters;

    try
    {
        for (Project::ExporterIterator exp (project); exp.next();)
        {
            if (specifiedExporterToSave.isNotEmpty() && exp->getName() != specifiedExporterToSave)
                continue;

            auto* exporter = exporters.add (exp.exporter.release());

            exporter->initialiseDependencyPathValues();

            if (exporter->getTargetFolder().createDirectory())
            {
                if (exporter->isCLion())
                {
                    clionExporter = dynamic_cast<CLionProjectExporter*> (exporter);
                }
                else
                {
                    exporter->copyMainGroupFromProject();
                    exporter->settings = exporter->settings.createCopy();

                    exporter->addToExtraSearchPaths (RelativePath ("JuceLibraryCode", RelativePath::projectFolder));

                    generatedFilesGroup.state = originalGeneratedGroup.createCopy();
                    exporter->addSettingsForProjectType (project.getProjectType());

                    for (auto& module: modules)
                        module->addSettingsForModuleToExporter (*exporter, *this);

                    generatedFilesGroup.sortAlphabetically (true, true);
                    exporter->getAllGroups().add (generatedFilesGroup);
                }

                if (isCommandLineApp)
                    saveExporter (exporter, modules);
                else
                    threadPool.addJob (new ExporterJob (*this, exporter, modules), true);
            }
            else
            {
                addError ("Can't create folder: " + exporter->getTargetFolder().getFullPathName());
            }
        }
    }
    catch (ProjectExporter::SaveError& saveError)
    {
        addError (saveError.message);
    }

    if (! isCommandLineApp)
        while (threadPool.getNumJobs() > 0)
            Thread::sleep (10);

    if (clionExporter != nullptr)
    {
        for (auto* exporter : exporters)
            clionExporter->writeCMakeListsExporterSection (exporter);

        std::cout << "Finished saving: " << clionExporter->getName() << std::endl;
    }
}
