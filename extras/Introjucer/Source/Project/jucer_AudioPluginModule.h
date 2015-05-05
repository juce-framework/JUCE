/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_AUDIOPLUGINMODULE_JUCEHEADER__
#define __JUCER_AUDIOPLUGINMODULE_JUCEHEADER__


//==============================================================================
namespace
{
    Value shouldBuildVST (Project& project)                       { return project.getProjectValue ("buildVST"); }
    Value shouldBuildVST3 (Project& project)                      { return project.getProjectValue ("buildVST3"); }
    Value shouldBuildAU (Project& project)                        { return project.getProjectValue ("buildAU"); }
    Value shouldBuildRTAS (Project& project)                      { return project.getProjectValue ("buildRTAS"); }
    Value shouldBuildAAX (Project& project)                       { return project.getProjectValue ("buildAAX"); }

    Value getPluginName (Project& project)                        { return project.getProjectValue ("pluginName"); }
    Value getPluginDesc (Project& project)                        { return project.getProjectValue ("pluginDesc"); }
    Value getPluginManufacturer (Project& project)                { return project.getProjectValue ("pluginManufacturer"); }
    Value getPluginManufacturerCode (Project& project)            { return project.getProjectValue ("pluginManufacturerCode"); }
    Value getPluginCode (Project& project)                        { return project.getProjectValue ("pluginCode"); }
    Value getPluginChannelConfigs (Project& project)              { return project.getProjectValue ("pluginChannelConfigs"); }
    Value getPluginIsSynth (Project& project)                     { return project.getProjectValue ("pluginIsSynth"); }
    Value getPluginWantsMidiInput (Project& project)              { return project.getProjectValue ("pluginWantsMidiIn"); }
    Value getPluginProducesMidiOut (Project& project)             { return project.getProjectValue ("pluginProducesMidiOut"); }
    Value getPluginSilenceInProducesSilenceOut (Project& project) { return project.getProjectValue ("pluginSilenceInIsSilenceOut"); }
    Value getPluginEditorNeedsKeyFocus (Project& project)         { return project.getProjectValue ("pluginEditorRequiresKeys"); }
    Value getPluginVSTCategory (Project& project)                 { return project.getProjectValue ("pluginVSTCategory"); }
    Value getPluginAUSDKLocation (Project& project)               { return project.getProjectValue ("pluginAUSDKLocation"); }
    Value getPluginAUExportPrefix (Project& project)              { return project.getProjectValue ("pluginAUExportPrefix"); }
    Value getPluginAUMainType (Project& project)                  { return project.getProjectValue ("pluginAUMainType"); }
    Value getPluginRTASCategory (Project& project)                { return project.getProjectValue ("pluginRTASCategory"); }
    Value getPluginRTASBypassDisabled (Project& project)          { return project.getProjectValue ("pluginRTASDisableBypass"); }
    Value getPluginRTASMultiMonoDisabled (Project& project)       { return project.getProjectValue ("pluginRTASDisableMultiMono"); }
    Value getPluginAAXCategory (Project& project)                 { return project.getProjectValue ("pluginAAXCategory"); }
    Value getPluginAAXBypassDisabled (Project& project)           { return project.getProjectValue ("pluginAAXDisableBypass"); }
    Value getPluginAAXMultiMonoDisabled (Project& project)        { return project.getProjectValue ("pluginAAXDisableMultiMono"); }

    String getPluginRTASCategoryCode (Project& project)
    {
        if (static_cast <bool> (getPluginIsSynth (project).getValue()))
            return "ePlugInCategory_SWGenerators";

        String s (getPluginRTASCategory (project).toString());
        if (s.isEmpty())
            s = "ePlugInCategory_None";

        return s;
    }

    String getAUMainTypeString (Project& project)
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

    String getAUMainTypeCode (Project& project)
    {
        String s (getPluginAUMainType (project).toString());

        if (s.isEmpty())
        {
            if (getPluginIsSynth (project).getValue())              s = "aumu";
            else if (getPluginWantsMidiInput (project).getValue())  s = "aumf";
            else                                                    s = "aufx";
        }

        return s;
    }

    String getPluginVSTCategoryString (Project& project)
    {
        String s (getPluginVSTCategory (project).toString().trim());

        if (s.isEmpty())
            s = static_cast<bool> (getPluginIsSynth (project).getValue()) ? "kPlugCategSynth"
                                                                          : "kPlugCategEffect";
        return s;
    }

    int countMaxPluginChannels (const String& configString, bool isInput)
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

    String valueToBool (const Value& v)
    {
        return static_cast<bool> (v.getValue()) ? "1" : "0";
    }

    String valueToStringLiteral (const var& v)
    {
        return CppTokeniserFunctions::addEscapeChars (v.toString()).quoted();
    }

    String valueToCharLiteral (const var& v)
    {
        return CppTokeniserFunctions::addEscapeChars (v.toString().trim().substring (0, 4)).quoted ('\'');
    }

    void writePluginCharacteristicsFile (ProjectSaver& projectSaver)
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
        flags.set ("JucePlugin_MaxNumInputChannels",         String (countMaxPluginChannels (getPluginChannelConfigs (project).toString(), true)));
        flags.set ("JucePlugin_MaxNumOutputChannels",        String (countMaxPluginChannels (getPluginChannelConfigs (project).toString(), false)));
        flags.set ("JucePlugin_PreferredChannelConfigurations", getPluginChannelConfigs (project).toString());
        flags.set ("JucePlugin_IsSynth",                     valueToBool (getPluginIsSynth (project)));
        flags.set ("JucePlugin_WantsMidiInput",              valueToBool (getPluginWantsMidiInput (project)));
        flags.set ("JucePlugin_ProducesMidiOutput",          valueToBool (getPluginProducesMidiOut (project)));
        flags.set ("JucePlugin_SilenceInProducesSilenceOut", valueToBool (getPluginSilenceInProducesSilenceOut (project)));
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

    static void fixMissingXcodePostBuildScript (ProjectExporter& exporter)
    {
        if (exporter.isXcode() && exporter.settings [Ids::postbuildCommand].toString().isEmpty())
            exporter.getSetting (Ids::postbuildCommand) = String::fromUTF8 (BinaryData::AudioPluginXCodeScript_txt,
                                                                            BinaryData::AudioPluginXCodeScript_txtSize);
    }

    String createEscapedStringForVersion (ProjectExporter& exporter, const String& text)
    {
        // (VS10 automatically adds escape characters to the quotes for this definition)
        return exporter.getVisualStudioVersion() < 10 ? CppTokeniserFunctions::addEscapeChars (text.quoted())
                                                      : CppTokeniserFunctions::addEscapeChars (text).quoted();
    }

    String createRebasedPath (ProjectExporter& exporter, const RelativePath& path)
    {
        return createEscapedStringForVersion (exporter,
                                              exporter.rebaseFromProjectFolderToBuildTarget (path)
                                                      .toWindowsStyle());
    }
}

//==============================================================================
namespace VSTHelpers
{
    static Value getVSTFolder (ProjectExporter& exporter, bool isVST3)
    {
        return exporter.getSetting (isVST3 ? Ids::vst3Folder
                                           : Ids::vstFolder);
    }

    static void addVSTFolderToPath (ProjectExporter& exporter, bool isVST3)
    {
        const String vstFolder (getVSTFolder (exporter, isVST3).toString());

        if (vstFolder.isNotEmpty())
        {
            RelativePath path (exporter.rebaseFromProjectFolderToBuildTarget (RelativePath (vstFolder, RelativePath::projectFolder)));

            if (exporter.isVisualStudio())
                exporter.extraSearchPaths.add (path.toWindowsStyle());
            else if (exporter.isLinuxMakefile() || exporter.isCodeBlocksLinux() || exporter.isXcode())
                exporter.extraSearchPaths.insert (0, path.toUnixStyle());
        }
    }

    static void createVSTPathEditor (ProjectExporter& exporter, PropertyListBuilder& props, bool isVST3)
    {
        const String vstFormat (isVST3 ? "VST3" : "VST");

        props.add (new TextPropertyComponent (getVSTFolder (exporter, isVST3), vstFormat + " Folder", 1024, false),
                   "If you're building a " + vstFormat + ", this must be the folder containing the " + vstFormat + " SDK. This should be an absolute path.");
    }

    static void fixMissingVSTValues (ProjectExporter& exporter, bool isVST3)
    {
        if (getVSTFolder (exporter, isVST3).toString().isEmpty())
            getVSTFolder (exporter, isVST3) = exporter.isWindows() ? (isVST3 ? "c:\\SDKs\\VST3 SDK" : "c:\\SDKs\\vstsdk2.4")
                                                                   : (isVST3 ? "~/SDKs/VST3 SDK"    : "~/SDKs/vstsdk2.4");

        fixMissingXcodePostBuildScript (exporter);
    }

    static inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver, bool isVST3)
    {
        fixMissingVSTValues (exporter, isVST3);
        writePluginCharacteristicsFile (projectSaver);

        exporter.makefileTargetSuffix = ".so";

        Project::Item group (Project::Item::createGroup (const_cast<ProjectExporter&> (exporter).getProject(),
                                                         "Juce VST Wrapper", "__jucevstfiles"));

        RelativePath juceWrapperFolder (exporter.getProject().getGeneratedCodeFolder(),
                                        exporter.getTargetFolder(), RelativePath::buildTargetFolder);

        addVSTFolderToPath (exporter, isVST3);

        if (exporter.isWindows())
            exporter.extraSearchPaths.add (juceWrapperFolder.toWindowsStyle());
        else if (exporter.isLinuxMakefile() || exporter.isCodeBlocksLinux() )
            exporter.extraSearchPaths.add (juceWrapperFolder.toUnixStyle());

        if (exporter.isVisualStudio())
        {
            if (! exporter.getExtraLinkerFlagsString().contains ("/FORCE:multiple"))
                exporter.getExtraLinkerFlags() = exporter.getExtraLinkerFlags().toString() + " /FORCE:multiple";

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
    }

    static inline void createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props, bool isVST3)
    {
        fixMissingVSTValues (exporter, isVST3);
        createVSTPathEditor (exporter, props, isVST3);
    }
}

//==============================================================================
namespace RTASHelpers
{
    static Value getRTASFolder (ProjectExporter& exporter)             { return exporter.getSetting (Ids::rtasFolder); }
    static RelativePath getRTASFolderPath (ProjectExporter& exporter)  { return RelativePath (exporter.getSettingString (Ids::rtasFolder),
                                                                                              RelativePath::projectFolder); }

    static bool isExporterSupported (ProjectExporter& exporter)   { return exporter.isVisualStudio() || exporter.isXcode(); }

    static void fixMissingRTASValues (ProjectExporter& exporter)
    {
        if (getRTASFolder (exporter).toString().isEmpty())
        {
            if (exporter.isVisualStudio())
                getRTASFolder (exporter) = "c:\\SDKs\\PT_80_SDK";
            else
                getRTASFolder (exporter) = "~/SDKs/PT_80_SDK";
        }

        fixMissingXcodePostBuildScript (exporter);
    }

    static void addExtraSearchPaths (ProjectExporter& exporter)
    {
        RelativePath rtasFolder (getRTASFolderPath (exporter));

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

    static inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingRTASValues (exporter);

            const RelativePath rtasFolder (getRTASFolderPath (exporter));

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

    static inline void createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingRTASValues (exporter);

            props.add (new TextPropertyComponent (getRTASFolder (exporter), "RTAS Folder", 1024, false),
                       "If you're building an RTAS, this must be the folder containing the RTAS SDK. This should be an absolute path.");
        }
    }
}

//==============================================================================
namespace AUHelpers
{
    static inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        writePluginCharacteristicsFile (projectSaver);

        if (exporter.isXcode())
        {
            String sdkLocation (getPluginAUSDKLocation (projectSaver.project).toString());

            if (sdkLocation.trim().isEmpty())
                sdkLocation = "$(DEVELOPER_DIR)/Extras/CoreAudio/";

            if (! sdkLocation.endsWithChar ('/'))
                sdkLocation << '/';

            {
                String relativeSDK (exporter.rebaseFromProjectFolderToBuildTarget (RelativePath (sdkLocation, RelativePath::projectFolder))
                                            .toUnixStyle());

                if (! relativeSDK.endsWithChar ('/'))
                    relativeSDK << '/';

                exporter.extraSearchPaths.add (relativeSDK + "PublicUtility");
                exporter.extraSearchPaths.add (relativeSDK + "AudioUnits/AUPublic/Utility");
                exporter.extraSearchPaths.add (relativeSDK + "AudioUnits/AUPublic/AUBase");
            }

            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);
            exporter.xcodeExcludedFiles64Bit = "\"*Carbon*.cpp\"";

            Project::Item subGroup (projectSaver.getGeneratedCodeGroup().addNewSubGroup ("Juce AU Wrapper", -1));
            subGroup.setID ("__juceappleaufiles");

            {
                static const char* appleAUFiles[] =
                {
                    "PublicUtility/CADebugMacros.h",
                    "PublicUtility/CAAUParameter.cpp",
                    "PublicUtility/CAAUParameter.h",
                    "PublicUtility/CAAudioChannelLayout.cpp",
                    "PublicUtility/CAAudioChannelLayout.h",
                    "PublicUtility/CAMutex.cpp",
                    "PublicUtility/CAMutex.h",
                    "PublicUtility/CAStreamBasicDescription.cpp",
                    "PublicUtility/CAStreamBasicDescription.h",
                    "PublicUtility/CAVectorUnitTypes.h",
                    "PublicUtility/CAVectorUnit.cpp",
                    "PublicUtility/CAVectorUnit.h",
                    "AudioUnits/AUPublic/AUViewBase/AUViewLocalizedStringKeys.h",
                    "AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewDispatch.cpp",
                    "AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewControl.cpp",
                    "AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewControl.h",
                    "AudioUnits/AUPublic/AUCarbonViewBase/CarbonEventHandler.cpp",
                    "AudioUnits/AUPublic/AUCarbonViewBase/CarbonEventHandler.h",
                    "AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewBase.cpp",
                    "AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewBase.h",
                    "AudioUnits/AUPublic/AUBase/AUBase.cpp",
                    "AudioUnits/AUPublic/AUBase/AUBase.h",
                    "AudioUnits/AUPublic/AUBase/AUDispatch.cpp",
                    "AudioUnits/AUPublic/AUBase/AUDispatch.h",
                    "AudioUnits/AUPublic/AUBase/AUInputElement.cpp",
                    "AudioUnits/AUPublic/AUBase/AUInputElement.h",
                    "AudioUnits/AUPublic/AUBase/AUOutputElement.cpp",
                    "AudioUnits/AUPublic/AUBase/AUOutputElement.h",
                    "AudioUnits/AUPublic/AUBase/AUResources.r",
                    "AudioUnits/AUPublic/AUBase/AUScopeElement.cpp",
                    "AudioUnits/AUPublic/AUBase/AUScopeElement.h",
                    "AudioUnits/AUPublic/AUBase/ComponentBase.cpp",
                    "AudioUnits/AUPublic/AUBase/ComponentBase.h",
                    "AudioUnits/AUPublic/OtherBases/AUMIDIBase.cpp",
                    "AudioUnits/AUPublic/OtherBases/AUMIDIBase.h",
                    "AudioUnits/AUPublic/OtherBases/AUMIDIEffectBase.cpp",
                    "AudioUnits/AUPublic/OtherBases/AUMIDIEffectBase.h",
                    "AudioUnits/AUPublic/OtherBases/AUOutputBase.cpp",
                    "AudioUnits/AUPublic/OtherBases/AUOutputBase.h",
                    "AudioUnits/AUPublic/OtherBases/MusicDeviceBase.cpp",
                    "AudioUnits/AUPublic/OtherBases/MusicDeviceBase.h",
                    "AudioUnits/AUPublic/OtherBases/AUEffectBase.cpp",
                    "AudioUnits/AUPublic/OtherBases/AUEffectBase.h",
                    "AudioUnits/AUPublic/Utility/AUBuffer.cpp",
                    "AudioUnits/AUPublic/Utility/AUBuffer.h",
                    "AudioUnits/AUPublic/Utility/AUInputFormatConverter.h",
                    "AudioUnits/AUPublic/Utility/AUSilentTimeout.h",
                    "AudioUnits/AUPublic/Utility/AUTimestampGenerator.h",
                    nullptr
                };

                // This converts things like $(DEVELOPER_DIR) to ${DEVELOPER_DIR}
                sdkLocation = sdkLocation.replaceCharacters ("()", "{}");

                for (const char** f = appleAUFiles; *f != nullptr; ++f)
                {
                    const RelativePath file (sdkLocation + *f, RelativePath::projectFolder);
                    subGroup.addRelativeFile (file, -1, file.hasFileExtension ("cpp;mm"));
                    subGroup.getChild (subGroup.getNumChildren() - 1).getShouldInhibitWarningsValue() = true;
                }
            }

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
    static Value getAAXFolder (ProjectExporter& exporter)             { return exporter.getSetting (Ids::aaxFolder); }
    static RelativePath getAAXFolderPath (ProjectExporter& exporter)  { return RelativePath (exporter.getSettingString (Ids::aaxFolder),
                                                                                             RelativePath::projectFolder); }

    static bool isExporterSupported (ProjectExporter& exporter)       { return exporter.isVisualStudio() || exporter.isXcode(); }

    static void fixMissingAAXValues (ProjectExporter& exporter)
    {
        if (getAAXFolder (exporter).toString().isEmpty())
        {
            if (exporter.isVisualStudio())
                getAAXFolder (exporter) = "c:\\SDKs\\AAX";
            else
                getAAXFolder (exporter) = "~/SDKs/AAX";
        }

        fixMissingXcodePostBuildScript (exporter);
    }

    static void addExtraSearchPaths (ProjectExporter& exporter)
    {
        const RelativePath aaxFolder (getAAXFolderPath (exporter));

        exporter.addToExtraSearchPaths (aaxFolder);
        exporter.addToExtraSearchPaths (aaxFolder.getChildFile ("Interfaces"));
        exporter.addToExtraSearchPaths (aaxFolder.getChildFile ("Interfaces").getChildFile ("ACF"));
    }

    static inline void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingAAXValues (exporter);

            const RelativePath aaxLibsFolder (getAAXFolderPath (exporter).getChildFile ("Libs"));

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

    static inline void createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props)
    {
        if (isExporterSupported (exporter))
        {
            fixMissingAAXValues (exporter);

            props.add (new TextPropertyComponent (getAAXFolder (exporter), "AAX SDK Folder", 1024, false),
                       "If you're building an AAX, this must be the folder containing the AAX SDK. This should be an absolute path.");
        }
    }
}

#endif   // __JUCER_AUDIOPLUGINMODULE_JUCEHEADER__
