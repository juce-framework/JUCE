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
    flags.set ("JucePlugin_Build_VST",                   valueToBool (project.getShouldBuildVSTAsValue()));
    flags.set ("JucePlugin_Build_VST3",                  valueToBool (project.getShouldBuildVST3AsValue()));
    flags.set ("JucePlugin_Build_AU",                    valueToBool (project.getShouldBuildAUAsValue()));
    flags.set ("JucePlugin_Build_AUv3",                  valueToBool (project.getShouldBuildAUv3AsValue()));
    flags.set ("JucePlugin_Build_RTAS",                  valueToBool (project.getShouldBuildRTASAsValue()));
    flags.set ("JucePlugin_Build_AAX",                   valueToBool (project.getShouldBuildAAXAsValue()));
    flags.set ("JucePlugin_Build_Standalone",            valueToBool (project.getShouldBuildStandalonePluginAsValue()));
    flags.set ("JucePlugin_Enable_IAA",                  valueToBool (project.getShouldEnableIAAAsValue()));
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
    flags.set ("JucePlugin_IAAType",                     valueToCharLiteral (project.getIAATypeCode()));
    flags.set ("JucePlugin_IAASubType",                  "JucePlugin_PluginCode");
    flags.set ("JucePlugin_IAAName",                     project.getIAAPluginName().quoted());

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

void ProjectSaver::writeProjects (const OwnedArray<LibraryModule>& modules, const String& specifiedExporterToSave, bool isCommandLineApp)
{
    ThreadPool threadPool;

    // keep a copy of the basic generated files group, as each exporter may modify it.
    const ValueTree originalGeneratedGroup (generatedFilesGroup.state.createCopy());

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

                    if (project.getProjectType().isAudioPlugin())
                        writePluginCharacteristicsFile();

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
