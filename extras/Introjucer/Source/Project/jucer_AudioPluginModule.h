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

#ifndef JUCER_AUDIOPLUGINMODULE_H_INCLUDED
#define JUCER_AUDIOPLUGINMODULE_H_INCLUDED

#include "../Application/jucer_GlobalPreferences.h"

//==============================================================================
namespace
{
    inline Value shouldBuildVST (Project& project)                       { return project.getProjectValue ("buildVST"); }
    inline Value shouldBuildVST3 (Project& project)                      { return project.getProjectValue ("buildVST3"); }
    inline Value shouldBuildAU (Project& project)                        { return project.getProjectValue ("buildAU"); }
    inline Value shouldBuildRTAS (Project& project)                      { return project.getProjectValue ("buildRTAS"); }
    inline Value shouldBuildAAX (Project& project)                       { return project.getProjectValue ("buildAAX"); }

    inline Value getPluginName (Project& project)                        { return project.getProjectValue ("pluginName"); }
    inline Value getPluginDesc (Project& project)                        { return project.getProjectValue ("pluginDesc"); }
    inline Value getPluginManufacturer (Project& project)                { return project.getProjectValue ("pluginManufacturer"); }
    inline Value getPluginManufacturerCode (Project& project)            { return project.getProjectValue ("pluginManufacturerCode"); }
    inline Value getPluginCode (Project& project)                        { return project.getProjectValue ("pluginCode"); }
    inline Value getPluginChannelConfigs (Project& project)              { return project.getProjectValue ("pluginChannelConfigs"); }
    inline Value getPluginIsSynth (Project& project)                     { return project.getProjectValue ("pluginIsSynth"); }
    inline Value getPluginWantsMidiInput (Project& project)              { return project.getProjectValue ("pluginWantsMidiIn"); }
    inline Value getPluginProducesMidiOut (Project& project)             { return project.getProjectValue ("pluginProducesMidiOut"); }
    inline Value getPluginIsMidiEffectPlugin (Project& project)          { return project.getProjectValue ("pluginIsMidiEffectPlugin"); }
    inline Value getPluginEditorNeedsKeyFocus (Project& project)         { return project.getProjectValue ("pluginEditorRequiresKeys"); }
    inline Value getPluginVSTCategory (Project& project)                 { return project.getProjectValue ("pluginVSTCategory"); }
    inline Value getPluginAUExportPrefix (Project& project)              { return project.getProjectValue ("pluginAUExportPrefix"); }
    inline Value getPluginAUMainType (Project& project)                  { return project.getProjectValue ("pluginAUMainType"); }
    inline Value getPluginRTASCategory (Project& project)                { return project.getProjectValue ("pluginRTASCategory"); }
    inline Value getPluginRTASBypassDisabled (Project& project)          { return project.getProjectValue ("pluginRTASDisableBypass"); }
    inline Value getPluginRTASMultiMonoDisabled (Project& project)       { return project.getProjectValue ("pluginRTASDisableMultiMono"); }
    inline Value getPluginAAXCategory (Project& project)                 { return project.getProjectValue ("pluginAAXCategory"); }
    inline Value getPluginAAXBypassDisabled (Project& project)           { return project.getProjectValue ("pluginAAXDisableBypass"); }
    inline Value getPluginAAXMultiMonoDisabled (Project& project)        { return project.getProjectValue ("pluginAAXDisableMultiMono"); }

    inline String getPluginRTASCategoryCode (Project& project)
    {
        if (static_cast<bool> (getPluginIsSynth (project).getValue()))
            return "ePlugInCategory_SWGenerators";

        String s (getPluginRTASCategory (project).toString());
        if (s.isEmpty())
            s = "ePlugInCategory_None";

        return s;
    }

    inline String getAUMainTypeString (Project& project)
    {
        String s (getPluginAUMainType (project).toString());

        if (s.isEmpty())
        {
            if (getPluginIsSynth (project).getValue())              s = "kAudioUnitType_MusicDevice";
            else if (getPluginWantsMidiInput (project).getValue())  s = "kAudioUnitType_MusicEffect";
            else                                                    s = "kAudioUnitType_Effect";
        }

        return s;
    }

    inline String getAUMainTypeCode (Project& project)
    {
        String s (getPluginAUMainType (project).toString());

        if (s.isEmpty())
        {
            if      (getPluginIsMidiEffectPlugin (project).getValue()) s = "aumi";
            else if (getPluginIsSynth (project).getValue())            s = "aumu";
            else if (getPluginWantsMidiInput (project).getValue())     s = "aumf";
            else                                                       s = "aufx";
        }

        return s;
    }

    inline String getPluginVSTCategoryString (Project& project)
    {
        String s (getPluginVSTCategory (project).toString().trim());

        if (s.isEmpty())
            s = static_cast<bool> (getPluginIsSynth (project).getValue()) ? "kPlugCategSynth"
                                                                          : "kPlugCategEffect";
        return s;
    }

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
        return CppTokeniserFunctions::addEscapeChars (v.toString().trim().substring (0, 4)).quoted ('\'');
    }

    inline void writePluginCharacteristicsFile (ProjectSaver& projectSaver)
    {
        Project& project = projectSaver.project;

        StringPairArray flags;
        flags.set ("JucePlugin_Build_VST",                   valueToBool (shouldBuildVST  (project)));
        flags.set ("JucePlugin_Build_VST3",                  valueToBool (shouldBuildVST3 (project)));
        flags.set ("JucePlugin_Build_AU",                    valueToBool (shouldBuildAU   (project)));
        flags.set ("JucePlugin_Build_RTAS",                  valueToBool (shouldBuildRTAS (project)));
        flags.set ("JucePlugin_Build_AAX",                   valueToBool (shouldBuildAAX  (project)));
        flags.set ("JucePlugin_Name",                        valueToStringLiteral (getPluginName (project)));
        flags.set ("JucePlugin_Desc",                        valueToStringLiteral (getPluginDesc (project)));
        flags.set ("JucePlugin_Manufacturer",                valueToStringLiteral (getPluginManufacturer (project)));
        flags.set ("JucePlugin_ManufacturerWebsite",         valueToStringLiteral (project.getCompanyWebsite()));
        flags.set ("JucePlugin_ManufacturerEmail",           valueToStringLiteral (project.getCompanyEmail()));
        flags.set ("JucePlugin_ManufacturerCode",            valueToCharLiteral (getPluginManufacturerCode (project)));
        flags.set ("JucePlugin_PluginCode",                  valueToCharLiteral (getPluginCode (project)));
        flags.set ("JucePlugin_IsSynth",                     valueToBool (getPluginIsSynth (project)));
        flags.set ("JucePlugin_WantsMidiInput",              valueToBool (getPluginWantsMidiInput (project)));
        flags.set ("JucePlugin_ProducesMidiOutput",          valueToBool (getPluginProducesMidiOut (project)));
        flags.set ("JucePlugin_IsMidiEffect",                valueToBool (getPluginIsMidiEffectPlugin (project)));
        flags.set ("JucePlugin_EditorRequiresKeyboardFocus", valueToBool (getPluginEditorNeedsKeyFocus (project)));
        flags.set ("JucePlugin_Version",                     project.getVersionString());
        flags.set ("JucePlugin_VersionCode",                 project.getVersionAsHex());
        flags.set ("JucePlugin_VersionString",               valueToStringLiteral (project.getVersionString()));
        flags.set ("JucePlugin_VSTUniqueID",                 "JucePlugin_PluginCode");
        flags.set ("JucePlugin_VSTCategory",                 getPluginVSTCategoryString (project));
        flags.set ("JucePlugin_AUMainType",                  getAUMainTypeString (project));
        flags.set ("JucePlugin_AUSubType",                   "JucePlugin_PluginCode");
        flags.set ("JucePlugin_AUExportPrefix",              getPluginAUExportPrefix (project).toString());
        flags.set ("JucePlugin_AUExportPrefixQuoted",        valueToStringLiteral (getPluginAUExportPrefix (project)));
        flags.set ("JucePlugin_AUManufacturerCode",          "JucePlugin_ManufacturerCode");
        flags.set ("JucePlugin_CFBundleIdentifier",          project.getBundleIdentifier().toString());
        flags.set ("JucePlugin_RTASCategory",                getPluginRTASCategoryCode (project));
        flags.set ("JucePlugin_RTASManufacturerCode",        "JucePlugin_ManufacturerCode");
        flags.set ("JucePlugin_RTASProductId",               "JucePlugin_PluginCode");
        flags.set ("JucePlugin_RTASDisableBypass",           valueToBool (getPluginRTASBypassDisabled (project)));
        flags.set ("JucePlugin_RTASDisableMultiMono",        valueToBool (getPluginRTASMultiMonoDisabled (project)));
        flags.set ("JucePlugin_AAXIdentifier",               project.getAAXIdentifier().toString());
        flags.set ("JucePlugin_AAXManufacturerCode",         "JucePlugin_ManufacturerCode");
        flags.set ("JucePlugin_AAXProductId",                "JucePlugin_PluginCode");
        flags.set ("JucePlugin_AAXCategory",                 getPluginAAXCategory (project).toString());
        flags.set ("JucePlugin_AAXDisableBypass",            valueToBool (getPluginAAXBypassDisabled (project)));
        flags.set ("JucePlugin_AAXDisableMultiMono",         valueToBool (getPluginAAXMultiMonoDisabled (project)));

        {
            String plugInChannelConfig = getPluginChannelConfigs (project).toString();

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

        projectSaver.setExtraAppConfigFileContent (mem.toString());
    }

    inline static void fixMissingXcodePostBuildScript (ProjectExporter& exporter)
    {
        if (exporter.isXcode() && exporter.settings [Ids::postbuildCommand].toString().isEmpty())
            exporter.getSetting (Ids::postbuildCommand) = String::fromUTF8 (BinaryData::AudioPluginXCodeScript_txt,
                                                                            BinaryData::AudioPluginXCodeScript_txtSize);
    }

    inline String createEscapedStringForVersion (ProjectExporter& exporter, const String& text)
    {
        // (VS10 automatically adds escape characters to the quotes for this definition)
        return exporter.getVisualStudioVersion() < 10 ? CppTokeniserFunctions::addEscapeChars (text.quoted())
                                                      : CppTokeniserFunctions::addEscapeChars (text).quoted();
    }

    inline String createRebasedPath (ProjectExporter& exporter, const RelativePath& path)
    {
        return createEscapedStringForVersion (exporter,
                                              exporter.rebaseFromProjectFolderToBuildTarget (path)
                                                      .toWindowsStyle());
    }
}

//==============================================================================
namespace VSTHelpers
{
    inline bool isExporterSupported (ProjectExporter& exporter)
    {
        return ! exporter.isAndroid();
    }

    inline void addVSTFolderToPath (ProjectExporter& exporter, bool isVST3)
    {
        const String vstFolder (exporter.getVSTPathValue (isVST3).toString());

        if (vstFolder.isNotEmpty())
            exporter.addToExtraSearchPaths (RelativePath (vstFolder, RelativePath::projectFolder), 0);
    }

    inline void createVSTPathEditor (ProjectExporter& exporter, PropertyListBuilder& props, bool isVST3)
    {
        const String vstFormat (isVST3 ? "VST3" : "VST");

        props.add (new DependencyPathPropertyComponent (exporter.getVSTPathValue (isVST3),
                                                        vstFormat + " Folder"),
                   "If you're building a " + vstFormat + ", this must be the folder containing the " + vstFormat + " SDK. This should be an absolute path.");
    }

    inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver, bool isVST3)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingXcodePostBuildScript (exporter);
            writePluginCharacteristicsFile (projectSaver);

            exporter.makefileTargetSuffix = ".so";

            Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(),
                                                             "Juce VST Wrapper", "__jucevstfiles"));

            addVSTFolderToPath (exporter, isVST3);

            if (exporter.isVisualStudio())
            {
                RelativePath modulePath (exporter.rebaseFromProjectFolderToBuildTarget (RelativePath (exporter.getPathForModuleString ("juce_audio_plugin_client"),
                                                                                                      RelativePath::projectFolder)
                                                                                          .getChildFile ("juce_audio_plugin_client")
                                                                                          .getChildFile ("VST3")));

                for (ProjectExporter::ConfigIterator config (exporter); config.next();)
                {
                    if (config->getValue (Ids::useRuntimeLibDLL).getValue().isVoid())
                        config->getValue (Ids::useRuntimeLibDLL) = true;

                    if (isVST3)
                        if (config->getValue (Ids::postbuildCommand).toString().isEmpty())
                            config->getValue (Ids::postbuildCommand) = "copy /Y \"$(OutDir)\\$(TargetFileName)\" \"$(OutDir)\\$(TargetName).vst3\"";
                }
            }

            if (exporter.isLinux())
                exporter.makefileExtraLinkerFlags.add ("-Wl,--no-undefined");
        }
    }

    inline void createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props, bool isVST3)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingXcodePostBuildScript (exporter);
            createVSTPathEditor (exporter, props, isVST3);
        }
    }
}

//==============================================================================
namespace RTASHelpers
{
    inline RelativePath getRTASRelativeFolderPath (ProjectExporter& exporter)
    {
        return RelativePath (exporter.getRTASPathValue().toString(), RelativePath::projectFolder);
    }

    inline bool isExporterSupported (ProjectExporter& exporter)
    {
        return exporter.isVisualStudio() || exporter.isXcode();
    }

    inline void addExtraSearchPaths (ProjectExporter& exporter)
    {
        RelativePath rtasFolder (getRTASRelativeFolderPath (exporter));

        if (exporter.isVisualStudio())
        {
            RelativePath juceWrapperFolder (exporter.getProject().getGeneratedCodeFolder(),
                                            exporter.getTargetFolder(), RelativePath::buildTargetFolder);

            exporter.extraSearchPaths.add (juceWrapperFolder.toWindowsStyle());

            static const char* p[] = { "AlturaPorts/TDMPlugins/PluginLibrary/EffectClasses",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/ProcessClasses/Interfaces",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/Utilities",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/RTASP_Adapt",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/CoreClasses",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/Controls",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/Meters",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/ViewClasses",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/DSPClasses",
                                       "AlturaPorts/TDMPlugins/PluginLibrary/Interfaces",
                                       "AlturaPorts/TDMPlugins/common",
                                       "AlturaPorts/TDMPlugins/common/Platform",
                                       "AlturaPorts/TDMPlugins/common/Macros",
                                       "AlturaPorts/TDMPlugins/SignalProcessing/Public",
                                       "AlturaPorts/TDMPlugIns/DSPManager/Interfaces",
                                       "AlturaPorts/SADriver/Interfaces",
                                       "AlturaPorts/DigiPublic/Interfaces",
                                       "AlturaPorts/DigiPublic",
                                       "AlturaPorts/Fic/Interfaces/DAEClient",
                                       "AlturaPorts/NewFileLibs/Cmn",
                                       "AlturaPorts/NewFileLibs/DOA",
                                       "AlturaPorts/AlturaSource/PPC_H",
                                       "AlturaPorts/AlturaSource/AppSupport",
                                       "AvidCode/AVX2sdk/AVX/avx2/avx2sdk/inc",
                                       "xplat/AVX/avx2/avx2sdk/inc" };

            for (int i = 0; i < numElementsInArray (p); ++i)
                exporter.addToExtraSearchPaths (rtasFolder.getChildFile (p[i]));
        }
        else if (exporter.isXcode())
        {
            exporter.extraSearchPaths.add ("$(DEVELOPER_DIR)/Headers/FlatCarbon");
            exporter.extraSearchPaths.add ("$(SDKROOT)/Developer/Headers/FlatCarbon");

            static const char* p[] = { "AlturaPorts/TDMPlugIns/PlugInLibrary/Controls",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/CoreClasses",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/DSPClasses",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/MacBuild",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/Meters",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses/Interfaces",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/RTASP_Adapt",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/Utilities",
                                       "AlturaPorts/TDMPlugIns/PlugInLibrary/ViewClasses",
                                       "AlturaPorts/TDMPlugIns/DSPManager/**",
                                       "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/Encryption",
                                       "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/GraphicsExtensions",
                                       "AlturaPorts/TDMPlugIns/common/**",
                                       "AlturaPorts/TDMPlugIns/common/PI_LibInterface",
                                       "AlturaPorts/TDMPlugIns/PACEProtection/**",
                                       "AlturaPorts/TDMPlugIns/SignalProcessing/**",
                                       "AlturaPorts/OMS/Headers",
                                       "AlturaPorts/Fic/Interfaces/**",
                                       "AlturaPorts/Fic/Source/SignalNets",
                                       "AlturaPorts/DSIPublicInterface/PublicHeaders",
                                       "DAEWin/Include",
                                       "AlturaPorts/DigiPublic/Interfaces",
                                       "AlturaPorts/DigiPublic",
                                       "AlturaPorts/NewFileLibs/DOA",
                                       "AlturaPorts/NewFileLibs/Cmn",
                                       "xplat/AVX/avx2/avx2sdk/inc",
                                       "xplat/AVX/avx2/avx2sdk/utils" };

            for (int i = 0; i < numElementsInArray (p); ++i)
                exporter.addToExtraSearchPaths (rtasFolder.getChildFile (p[i]));
        }
    }

    inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingXcodePostBuildScript (exporter);

            const RelativePath rtasFolder (getRTASRelativeFolderPath (exporter));

            if (exporter.isVisualStudio())
            {
                exporter.msvcTargetSuffix = ".dpm";

                exporter.msvcExtraPreprocessorDefs.set ("JucePlugin_WinBag_path",
                                                        createRebasedPath (exporter,
                                                                           rtasFolder.getChildFile ("WinBag")));

                exporter.msvcDelayLoadedDLLs = "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; "
                                               "DSPManager.dll; DSPManager.dll; DSPManagerClientLib.dll; RTASClientLib.dll";

                if (! exporter.getExtraLinkerFlagsString().contains ("/FORCE:multiple"))
                    exporter.getExtraLinkerFlags() = exporter.getExtraLinkerFlags().toString() + " /FORCE:multiple";

                RelativePath modulePath (exporter.rebaseFromProjectFolderToBuildTarget (RelativePath (exporter.getPathForModuleString ("juce_audio_plugin_client"),
                                                                                                      RelativePath::projectFolder)
                                                                                           .getChildFile ("juce_audio_plugin_client")
                                                                                           .getChildFile ("RTAS")));

                for (ProjectExporter::ConfigIterator config (exporter); config.next();)
                {
                    config->getValue (Ids::msvcModuleDefinitionFile) = modulePath.getChildFile ("juce_RTAS_WinExports.def").toWindowsStyle();

                    if (config->getValue (Ids::useRuntimeLibDLL).getValue().isVoid())
                        config->getValue (Ids::useRuntimeLibDLL) = true;

                    if (config->getValue (Ids::postbuildCommand).toString().isEmpty())
                        config->getValue (Ids::postbuildCommand)
                            = "copy /Y "
                                + modulePath.getChildFile ("juce_RTAS_WinResources.rsr").toWindowsStyle().quoted()
                                + " \"$(TargetPath)\".rsr";
                }
            }
            else
            {
                exporter.xcodeCanUseDwarf = false;

                exporter.xcodeExtraLibrariesDebug.add   (rtasFolder.getChildFile ("MacBag/Libs/Debug/libPluginLibrary.a"));
                exporter.xcodeExtraLibrariesRelease.add (rtasFolder.getChildFile ("MacBag/Libs/Release/libPluginLibrary.a"));
            }

            writePluginCharacteristicsFile (projectSaver);

            addExtraSearchPaths (exporter);
        }
    }

    inline void createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingXcodePostBuildScript (exporter);

            props.add (new DependencyPathPropertyComponent (exporter.getRTASPathValue(),
                                                            "RTAS Folder"),
                       "If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
        }
    }
}

//==============================================================================
namespace AUHelpers
{
    inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        writePluginCharacteristicsFile (projectSaver);

        if (exporter.isXcode())
        {
            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

            XmlElement plistKey ("key");
            plistKey.addTextElement ("AudioComponents");

            XmlElement plistEntry ("array");
            XmlElement* dict = plistEntry.createNewChildElement ("dict");

            Project& project = exporter.getProject();

            addPlistDictionaryKey (dict, "name", getPluginManufacturer (project).toString()
                                                   + ": " + getPluginName (project).toString());
            addPlistDictionaryKey (dict, "description", getPluginDesc (project).toString());
            addPlistDictionaryKey (dict, "factoryFunction", getPluginAUExportPrefix (project).toString() + "Factory");
            addPlistDictionaryKey (dict, "manufacturer", getPluginManufacturerCode (project).toString().trim().substring (0, 4));
            addPlistDictionaryKey (dict, "type", getAUMainTypeCode (project));
            addPlistDictionaryKey (dict, "subtype", getPluginCode (project).toString().trim().substring (0, 4));
            addPlistDictionaryKeyInt (dict, "version", project.getVersionAsHexInteger());

            exporter.xcodeExtraPListEntries.add (plistKey);
            exporter.xcodeExtraPListEntries.add (plistEntry);

            fixMissingXcodePostBuildScript (exporter);
        }
    }
}

//==============================================================================
namespace AAXHelpers
{
    inline RelativePath getAAXRelativeFolderPath (ProjectExporter& exporter)
    {
        return RelativePath (exporter.getAAXPathValue().toString(), RelativePath::projectFolder);
    }

    inline bool isExporterSupported (ProjectExporter& exporter)
    {
        return exporter.isVisualStudio() || exporter.isXcode();
    }

    inline void addExtraSearchPaths (ProjectExporter& exporter)
    {
        const RelativePath aaxFolder (getAAXRelativeFolderPath (exporter));

        exporter.addToExtraSearchPaths (aaxFolder);
        exporter.addToExtraSearchPaths (aaxFolder.getChildFile ("Interfaces"));
        exporter.addToExtraSearchPaths (aaxFolder.getChildFile ("Interfaces").getChildFile ("ACF"));
    }

    inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingXcodePostBuildScript (exporter);

            const RelativePath aaxLibsFolder (getAAXRelativeFolderPath (exporter).getChildFile ("Libs"));

            if (exporter.isVisualStudio())
            {
                for (ProjectExporter::ConfigIterator config (exporter); config.next();)
                    if (config->getValue (Ids::useRuntimeLibDLL).getValue().isVoid())
                        config->getValue (Ids::useRuntimeLibDLL) = true;

                exporter.msvcExtraPreprocessorDefs.set ("JucePlugin_AAXLibs_path",
                                                        createRebasedPath (exporter, aaxLibsFolder));
            }
            else
            {
                exporter.xcodeExtraLibrariesDebug.add   (aaxLibsFolder.getChildFile ("Debug/libAAXLibrary.a"));
                exporter.xcodeExtraLibrariesRelease.add (aaxLibsFolder.getChildFile ("Release/libAAXLibrary.a"));
            }

            writePluginCharacteristicsFile (projectSaver);

            addExtraSearchPaths (exporter);
        }
    }

    inline void createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingXcodePostBuildScript (exporter);

            props.add (new DependencyPathPropertyComponent (exporter.getAAXPathValue(),
                                                            "AAX SDK Folder"),
                       "If you're building an AAX, this must be the folder containing the AAX SDK. This should be an absolute path.");
        }
    }
}

#endif   // JUCER_AUDIOPLUGINMODULE_H_INCLUDED
