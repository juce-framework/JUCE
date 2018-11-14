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

#pragma once


//==============================================================================
class MSVCProjectExporterBase   : public ProjectExporter
{
public:
    MSVCProjectExporterBase (Project& p, const ValueTree& t, String folderName)
        : ProjectExporter (p, t),
          IPPLibraryValue       (settings, Ids::IPPLibrary,                   getUndoManager()),
          platformToolsetValue  (settings, Ids::toolset,                      getUndoManager()),
          targetPlatformVersion (settings, Ids::windowsTargetPlatformVersion, getUndoManager()),
          manifestFileValue     (settings, Ids::msvcManifestFile,             getUndoManager())
    {
        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + folderName);

        updateOldSettings();
    }

    virtual int getVisualStudioVersion() const = 0;

    virtual String getSolutionComment() const = 0;
    virtual String getToolsVersion() const = 0;
    virtual String getDefaultToolset() const = 0;
    virtual String getDefaultWindowsTargetPlatformVersion() const = 0;

    //==============================================================================
    String getIPPLibrary() const                      { return IPPLibraryValue.get(); }
    String getPlatformToolset() const                 { return platformToolsetValue.get(); }
    String getWindowsTargetPlatformVersion() const    { return targetPlatformVersion.get(); }

    //==============================================================================
    void addToolsetProperty (PropertyListBuilder& props, const char** names, const var* values, int num)
    {
        props.add (new ChoicePropertyComponent (platformToolsetValue, "Platform Toolset",
                                                StringArray (names, num), { values, num }),
                   "Specifies the version of the platform toolset that will be used when building this project.");
    }

    void addIPPLibraryProperty (PropertyListBuilder& props)
    {
        props.add (new ChoicePropertyComponent (IPPLibraryValue, "Use IPP Library",
                                                { "No",  "Yes (Default Linking)",  "Multi-Threaded Static Library", "Single-Threaded Static Library", "Multi-Threaded DLL", "Single-Threaded DLL" },
                                                { var(), "true",                   "Parallel_Static",               "Sequential",                     "Parallel_Dynamic",   "Sequential_Dynamic" }),
                   "Enable this to use Intel's Integrated Performance Primitives library.");
    }

    void addWindowsTargetPlatformProperties (PropertyListBuilder& props)
    {
        auto isWindows10SDK = getVisualStudioVersion() > 14;

        props.add (new TextPropertyComponent (targetPlatformVersion, "Windows Target Platform", 20, false),
                   String ("Specifies the version of the Windows SDK that will be used when building this project. ")
                   + (isWindows10SDK ? "Leave this field empty to use the latest Windows 10 SDK installed on the build machine."
                                     : "The default value for this exporter is " + getDefaultWindowsTargetPlatformVersion()));
    }

    void create (const OwnedArray<LibraryModule>&) const override
    {
        createResourcesAndIcon();

        for (int i = 0; i < targets.size(); ++i)
            if (auto* target = targets[i])
                target->writeProjectFile();

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, "11.00", getSolutionComment());

            overwriteFileIfDifferentOrThrow (getSLNFile(), mo);
        }
    }

    //==============================================================================
    void initialiseDependencyPathValues() override
    {
        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                        getAppSettings().getStoredPath (Ids::vstLegacyPath, TargetOS::windows), TargetOS::windows);

        vst3PathValueWrapper.init ({ settings, Ids::vst3Folder, nullptr },
                                   getAppSettings().getStoredPath (Ids::vst3Path, TargetOS::windows), TargetOS::windows);

        aaxPathValueWrapper.init ({ settings, Ids::aaxFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::aaxPath,  TargetOS::windows), TargetOS::windows);

        rtasPathValueWrapper.init ({ settings, Ids::rtasFolder, nullptr },
                                   getAppSettings().getStoredPath (Ids::rtasPath, TargetOS::windows), TargetOS::windows);
    }

    //==============================================================================
    class MSVCBuildConfiguration  : public BuildConfiguration,
                                    private Value::Listener
    {
    public:
        MSVCBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e),
              warningLevelValue              (config, Ids::winWarningLevel,            getUndoManager(), 4),
              warningsAreErrorsValue         (config, Ids::warningsAreErrors,          getUndoManager(), false),
              prebuildCommandValue           (config, Ids::prebuildCommand,            getUndoManager()),
              postbuildCommandValue          (config, Ids::postbuildCommand,           getUndoManager()),
              generateDebugSymbolsValue      (config, Ids::alwaysGenerateDebugSymbols, getUndoManager(), false),
              generateManifestValue          (config, Ids::generateManifest,           getUndoManager(), true),
              enableIncrementalLinkingValue  (config, Ids::enableIncrementalLinking,   getUndoManager(), false),
              useRuntimeLibDLLValue          (config, Ids::useRuntimeLibDLL,           getUndoManager(), true),
              multiProcessorCompilationValue (config, Ids::multiProcessorCompilation,  getUndoManager(), true),
              intermediatesPathValue         (config, Ids::intermediatesPath,          getUndoManager()),
              characterSetValue              (config, Ids::characterSet,               getUndoManager()),
              architectureTypeValue          (config, Ids::winArchitecture,            getUndoManager(), get64BitArchName()),
              fastMathValue                  (config, Ids::fastMath,                   getUndoManager()),
              debugInformationFormatValue    (config, Ids::debugInformationFormat,     getUndoManager(), isDebug() ? "ProgramDatabase" : "None"),
              pluginBinaryCopyStepValue      (config, Ids::enablePluginBinaryCopyStep, getUndoManager(), false),
              vstBinaryLocation              (config, Ids::vstBinaryLocation,          getUndoManager()),
              vst3BinaryLocation             (config, Ids::vst3BinaryLocation,         getUndoManager()),
              rtasBinaryLocation             (config, Ids::rtasBinaryLocation,         getUndoManager()),
              aaxBinaryLocation              (config, Ids::aaxBinaryLocation,          getUndoManager()),
              unityPluginBinaryLocation      (config, Ids::unityPluginBinaryLocation,  getUndoManager(), {})
        {
            if (! isDebug())
                updateOldLTOSetting();

            setPluginBinaryCopyLocationDefaults();
            optimisationLevelValue.setDefault (isDebug() ? optimisationOff : optimiseFull);

            architectureValueToListenTo = architectureTypeValue.getPropertyAsValue();
            architectureValueToListenTo.addListener (this);
        }

        //==============================================================================
        int getWarningLevel() const                       { return warningLevelValue.get(); }
        bool areWarningsTreatedAsErrors() const           { return warningsAreErrorsValue.get(); }

        String getPrebuildCommandString() const           { return prebuildCommandValue.get(); }
        String getPostbuildCommandString() const          { return postbuildCommandValue.get(); }
        String getVSTBinaryLocationString() const         { return vstBinaryLocation.get(); }
        String getVST3BinaryLocationString() const        { return vst3BinaryLocation.get(); }
        String getRTASBinaryLocationString() const        { return rtasBinaryLocation.get();}
        String getAAXBinaryLocationString() const         { return aaxBinaryLocation.get();}
        String getUnityPluginBinaryLocationString() const { return unityPluginBinaryLocation.get(); }
        String getIntermediatesPathString() const         { return intermediatesPathValue.get(); }
        String getCharacterSetString() const              { return characterSetValue.get(); }
        String get64BitArchName() const                   { return "x64"; }
        String get32BitArchName() const                   { return "Win32"; }
        String getArchitectureString() const              { return architectureTypeValue.get(); }
        String getDebugInformationFormatString() const    { return debugInformationFormatValue.get(); }

        bool shouldGenerateDebugSymbols() const           { return generateDebugSymbolsValue.get(); }
        bool shouldGenerateManifest() const               { return generateManifestValue.get(); }
        bool shouldLinkIncremental() const                { return enableIncrementalLinkingValue.get(); }
        bool isUsingRuntimeLibDLL() const                 { return useRuntimeLibDLLValue.get(); }
        bool shouldUseMultiProcessorCompilation() const   { return multiProcessorCompilationValue.get(); }
        bool is64Bit() const                              { return getArchitectureString() == get64BitArchName(); }
        bool isFastMathEnabled() const                    { return fastMathValue.get(); }
        bool isPluginBinaryCopyStepEnabled() const        { return pluginBinaryCopyStepValue.get(); }

        //==============================================================================
        String createMSVCConfigName() const
        {
            return getName() + "|" + (is64Bit() ? "x64" : "Win32");
        }

        String getOutputFilename (const String& suffix, bool forceSuffix, bool forceUnityPrefix) const
        {
            auto target = File::createLegalFileName (getTargetBinaryNameString (forceUnityPrefix).trim());

            if (forceSuffix || ! target.containsChar ('.'))
                return target.upToLastOccurrenceOf (".", false, false) + suffix;

            return target;
        }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addVisualStudioPluginInstallPathProperties (props);

            props.add (new ChoicePropertyComponent (architectureTypeValue, "Architecture",
                                                    { get32BitArchName(), get64BitArchName() },
                                                    { get32BitArchName(), get64BitArchName() }),
                       "Whether to use a 32-bit or 64-bit architecture.");


            props.add (new ChoicePropertyComponentWithEnablement (debugInformationFormatValue,
                                                                  isDebug() ? isDebugValue : generateDebugSymbolsValue,
                                                                  "Debug Information Format",
                                                                  { "None", "C7 Compatible (/Z7)", "Program Database (/Zi)", "Program Database for Edit And Continue (/ZI)" },
                                                                  { "None", "OldStyle",            "ProgramDatabase",        "EditAndContinue" }),
                       "The type of debugging information created for your program for this configuration."
                       " This will always be used in a debug configuration and will be used in a release configuration"
                       " with forced generation of debug symbols.");

            props.add (new ChoicePropertyComponent (fastMathValue, "Relax IEEE Compliance"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new ChoicePropertyComponent (optimisationLevelValue, "Optimisation",
                                                    { "Disabled (/Od)", "Minimise size (/O1)", "Maximise speed (/O2)", "Full optimisation (/Ox)" },
                                                    { optimisationOff,  optimiseMinSize,       optimiseMaxSpeed,       optimiseFull }),
                       "The optimisation level for this configuration");

            props.add (new TextPropertyComponent (intermediatesPathValue, "Intermediates Path", 2048, false),
                       "An optional path to a folder to use for the intermediate build files. Note that Visual Studio allows "
                       "you to use macros in this path, e.g. \"$(TEMP)\\MyAppBuildFiles\\$(Configuration)\", which is a handy way to "
                       "send them to the user's temp folder.");

            props.add (new ChoicePropertyComponent (warningLevelValue, "Warning Level",
                                                    { "Low", "Medium", "High" },
                                                    { 2,     3,        4 }),
                       "The compilation warning level to use.");

            props.add (new ChoicePropertyComponent (warningsAreErrorsValue, "Treat Warnings as Errors"),
                       "Enable this to treat compilation warnings as errors.");

            props.add (new ChoicePropertyComponent (useRuntimeLibDLLValue, "Runtime Library",
                                                    { "Use static runtime", "Use DLL runtime" },
                                                    { false,                true }),
                       "If the static runtime is selected then your app/plug-in will not be dependent upon users having Microsoft's redistributable "
                       "C++ runtime installed. However, if you are linking libraries from different sources you must select the same type of runtime "
                       "used by the libraries.");

            props.add (new ChoicePropertyComponent (multiProcessorCompilationValue, "Multi-Processor Compilation",
                                                    { "Enabled", "Disabled" },
                                                    { true,      false }),
                       "Allows the compiler to use of all the available processors, which can reduce compilation time. "
                       "This is enabled by default and should only be disabled if you know what you are doing.");

            props.add (new ChoicePropertyComponent (enableIncrementalLinkingValue, "Incremental Linking"),
                       "Enable to avoid linking from scratch for every new build. "
                       "Disable to ensure that your final release build does not contain padding or thunks.");

            if (! isDebug())
            {
                props.add (new ChoicePropertyComponent (generateDebugSymbolsValue, "Force Generation of Debug Symbols"),
                           "Enable this to force generation of debug symbols in a release configuration.");
            }

            props.add (new TextPropertyComponent (prebuildCommandValue,  "Pre-build Command",  2048, true),
                       "Some command that will be run before a build starts.");

            props.add (new TextPropertyComponent (postbuildCommandValue, "Post-build Command", 2048, true),
                       "Some command that will be run after a build starts.");

            props.add (new ChoicePropertyComponent (generateManifestValue, "Generate Manifest"),
                       "Enable this to generate a Manifest file.");

            props.add (new ChoicePropertyComponent (characterSetValue, "Character Set",
                                                    { "MultiByte", "Unicode" },
                                                    { "MultiByte", "Unicode" }),
                       "Specifies the character set used when building.");
        }

        String getModuleLibraryArchName() const override
        {
            String result ("$(Platform)\\");
            result += isUsingRuntimeLibDLL() ? "MD" : "MT";

            if (isDebug())
                result += "d";

            return result;
        }

    private:
        ValueWithDefault warningLevelValue, warningsAreErrorsValue, prebuildCommandValue, postbuildCommandValue, generateDebugSymbolsValue,
                         generateManifestValue, enableIncrementalLinkingValue, useRuntimeLibDLLValue, multiProcessorCompilationValue,
                         intermediatesPathValue, characterSetValue, architectureTypeValue, fastMathValue, debugInformationFormatValue,
                         pluginBinaryCopyStepValue;

        ValueWithDefault vstBinaryLocation, vst3BinaryLocation, rtasBinaryLocation, aaxBinaryLocation, unityPluginBinaryLocation;

        Value architectureValueToListenTo;

        //==============================================================================
        void updateOldLTOSetting()
        {
            if (config.getPropertyAsValue ("wholeProgramOptimisation", nullptr) != Value())
                linkTimeOptimisationValue = (static_cast<int> (config ["wholeProgramOptimisation"]) == 0);
        }

        void addVisualStudioPluginInstallPathProperties (PropertyListBuilder& props)
        {
            auto isBuildingAnyPlugins = (project.shouldBuildVST()  || project.shouldBuildVST3() || project.shouldBuildRTAS()
                                          || project.shouldBuildAAX() || project.shouldBuildUnityPlugin());

            if (isBuildingAnyPlugins)
                props.add (new ChoicePropertyComponent (pluginBinaryCopyStepValue, "Enable Plugin Copy Step"),
                           "Enable this to copy plugin binaries to a specified folder after building.");

            if (project.shouldBuildVST3())
                props.add (new TextPropertyComponentWithEnablement (vst3BinaryLocation, pluginBinaryCopyStepValue, "VST3 Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled VST3 binary should be placed.");

            if (project.shouldBuildRTAS())
                props.add (new TextPropertyComponentWithEnablement (rtasBinaryLocation, pluginBinaryCopyStepValue, "RTAS Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled RTAS binary should be placed.");

            if (project.shouldBuildAAX())
                props.add (new TextPropertyComponentWithEnablement (aaxBinaryLocation, pluginBinaryCopyStepValue, "AAX Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled AAX binary should be placed.");

            if (project.shouldBuildUnityPlugin())
                props.add (new TextPropertyComponentWithEnablement (unityPluginBinaryLocation, pluginBinaryCopyStepValue, "Unity Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled Unity plugin binary and associated C# GUI script should be placed.");

            if (project.shouldBuildVST())
                props.add (new TextPropertyComponentWithEnablement (vstBinaryLocation, pluginBinaryCopyStepValue, "VST (Legacy) Binary Location",
                                                                    1024, false),
                           "The folder in which the compiled lehacy VST binary should be placed.");

        }

        void setPluginBinaryCopyLocationDefaults()
        {
            vstBinaryLocation.setDefault  ((is64Bit() ? "%ProgramW6432%" : "%programfiles(x86)%") + String ("\\Steinberg\\Vstplugins"));

            auto prefix = is64Bit() ? "%CommonProgramW6432%"
                                    : "%CommonProgramFiles(x86)%";

            vst3BinaryLocation.setDefault (prefix + String ("\\VST3"));
            rtasBinaryLocation.setDefault (prefix + String ("\\Digidesign\\DAE\\Plug-Ins"));
            aaxBinaryLocation.setDefault  (prefix + String ("\\Avid\\Audio\\Plug-Ins"));
        }

        void valueChanged (Value&) override
        {
            setPluginBinaryCopyLocationDefaults();
        }
    };

    //==============================================================================
    class MSVCTargetBase : public ProjectType::Target
    {
    public:
        MSVCTargetBase (ProjectType::Target::Type targetType, const MSVCProjectExporterBase& exporter)
            : ProjectType::Target (targetType), owner (exporter)
        {
            projectGuid = createGUID (owner.getProject().getProjectUIDString() + getName());
        }

        virtual ~MSVCTargetBase() {}

        String getProjectVersionString() const     { return "10.00"; }
        String getProjectFileSuffix() const        { return ".vcxproj"; }
        String getFiltersFileSuffix() const        { return ".vcxproj.filters"; }
        String getTopLevelXmlEntity() const        { return "Project"; }

        //==============================================================================
        void fillInProjectXml (XmlElement& projectXml) const
        {
            projectXml.setAttribute ("DefaultTargets", "Build");
            projectXml.setAttribute ("ToolsVersion", getOwner().getToolsVersion());
            projectXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

            {
                auto* configsGroup = projectXml.createNewChildElement ("ItemGroup");
                configsGroup->setAttribute ("Label", "ProjectConfigurations");

                for (ConstConfigIterator i (owner); i.next();)
                {
                    auto& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);
                    auto* e = configsGroup->createNewChildElement ("ProjectConfiguration");
                    e->setAttribute ("Include", config.createMSVCConfigName());
                    e->createNewChildElement ("Configuration")->addTextElement (config.getName());
                    e->createNewChildElement ("Platform")->addTextElement (config.is64Bit() ? config.get64BitArchName()
                                                                                            : config.get32BitArchName());
                }
            }

            {
                auto* globals = projectXml.createNewChildElement ("PropertyGroup");
                globals->setAttribute ("Label", "Globals");
                globals->createNewChildElement ("ProjectGuid")->addTextElement (getProjectGuid());
            }

            {
                auto* imports = projectXml.createNewChildElement ("Import");
                imports->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
            }

            for (ConstConfigIterator i (owner); i.next();)
            {
                auto& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);

                auto* e = projectXml.createNewChildElement ("PropertyGroup");
                setConditionAttribute (*e, config);
                e->setAttribute ("Label", "Configuration");
                e->createNewChildElement ("ConfigurationType")->addTextElement (getProjectType());
                e->createNewChildElement ("UseOfMfc")->addTextElement ("false");
                e->createNewChildElement ("WholeProgramOptimization")->addTextElement (config.isLinkTimeOptimisationEnabled() ? "true"
                                                                                                                              : "false");

                auto charSet = config.getCharacterSetString();

                if (charSet.isNotEmpty())
                    e->createNewChildElement ("CharacterSet")->addTextElement (charSet);

                if (config.shouldLinkIncremental())
                    e->createNewChildElement ("LinkIncremental")->addTextElement ("true");

                e->createNewChildElement ("PlatformToolset")->addTextElement (owner.getPlatformToolset());

                addWindowsTargetPlatformToConfig (*e);

                auto ippLibrary = owner.getIPPLibrary();

                if (ippLibrary.isNotEmpty())
                    e->createNewChildElement ("UseIntelIPP")->addTextElement (ippLibrary);
            }

            {
                auto* e = projectXml.createNewChildElement ("Import");
                e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");
            }

            {
                auto* e = projectXml.createNewChildElement ("ImportGroup");
                e->setAttribute ("Label", "ExtensionSettings");
            }

            {
                auto* e = projectXml.createNewChildElement ("ImportGroup");
                e->setAttribute ("Label", "PropertySheets");
                auto* p = e->createNewChildElement ("Import");
                p->setAttribute ("Project", "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props");
                p->setAttribute ("Condition", "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')");
                p->setAttribute ("Label", "LocalAppDataPlatform");
            }

            {
                auto* props = projectXml.createNewChildElement ("PropertyGroup");
                props->createNewChildElement ("_ProjectFileVersion")->addTextElement ("10.0.30319.1");
                props->createNewChildElement ("TargetExt")->addTextElement (getTargetSuffix());

                for (ConstConfigIterator i (owner); i.next();)
                {
                    auto& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);

                    if (getConfigTargetPath (config).isNotEmpty())
                    {
                        auto* outdir = props->createNewChildElement ("OutDir");
                        setConditionAttribute (*outdir, config);
                        outdir->addTextElement (FileHelpers::windowsStylePath (getConfigTargetPath (config)) + "\\");
                    }

                    {
                        auto* intdir = props->createNewChildElement("IntDir");
                        setConditionAttribute (*intdir, config);

                        auto intermediatesPath = getIntermediatesPath (config);
                        if (! intermediatesPath.endsWithChar (L'\\'))
                            intermediatesPath += L'\\';

                        intdir->addTextElement (FileHelpers::windowsStylePath (intermediatesPath));
                    }


                    {
                        auto* targetName = props->createNewChildElement ("TargetName");
                        setConditionAttribute (*targetName, config);
                        targetName->addTextElement (config.getOutputFilename ("", false, type == UnityPlugIn));
                    }

                    {
                        auto* manifest = props->createNewChildElement ("GenerateManifest");
                        setConditionAttribute (*manifest, config);
                        manifest->addTextElement (config.shouldGenerateManifest() ? "true" : "false");
                    }

                    if (type != SharedCodeTarget)
                    {
                        auto librarySearchPaths = getLibrarySearchPaths (config);
                        if (librarySearchPaths.size() > 0)
                        {
                            auto* libPath = props->createNewChildElement ("LibraryPath");
                            setConditionAttribute (*libPath, config);
                            libPath->addTextElement ("$(LibraryPath);" + librarySearchPaths.joinIntoString (";"));
                        }
                    }
                }
            }

            for (ConstConfigIterator i (owner); i.next();)
            {
                auto& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);

                bool isDebug = config.isDebug();

                auto* group = projectXml.createNewChildElement ("ItemDefinitionGroup");
                setConditionAttribute (*group, config);

                {
                    auto* midl = group->createNewChildElement ("Midl");
                    midl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                     : "NDEBUG;%(PreprocessorDefinitions)");
                    midl->createNewChildElement ("MkTypLibCompatible")->addTextElement ("true");
                    midl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                    midl->createNewChildElement ("TargetEnvironment")->addTextElement ("Win32");
                    midl->createNewChildElement ("HeaderFileName");
                }

                bool isUsingEditAndContinue = false;

                {
                    auto* cl = group->createNewChildElement ("ClCompile");

                    cl->createNewChildElement ("Optimization")->addTextElement (getOptimisationLevelString (config.getOptimisationLevelInt()));

                    if (isDebug || config.shouldGenerateDebugSymbols())
                    {
                        cl->createNewChildElement ("DebugInformationFormat")
                            ->addTextElement (config.getDebugInformationFormatString());
                    }

                    auto includePaths = getOwner().getHeaderSearchPaths (config);
                    includePaths.addArray (getExtraSearchPaths());
                    includePaths.add ("%(AdditionalIncludeDirectories)");

                    cl->createNewChildElement ("AdditionalIncludeDirectories")->addTextElement (includePaths.joinIntoString (";"));
                    cl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (getPreprocessorDefs (config, ";") + ";%(PreprocessorDefinitions)");

                    cl->createNewChildElement ("RuntimeLibrary")->addTextElement (config.isUsingRuntimeLibDLL() ? (isDebug ? "MultiThreadedDebugDLL" : "MultiThreadedDLL")
                                                                                                                : (isDebug ? "MultiThreadedDebug"    : "MultiThreaded"));
                    cl->createNewChildElement ("RuntimeTypeInfo")->addTextElement ("true");
                    cl->createNewChildElement ("PrecompiledHeader");
                    cl->createNewChildElement ("AssemblerListingLocation")->addTextElement ("$(IntDir)\\");
                    cl->createNewChildElement ("ObjectFileName")->addTextElement ("$(IntDir)\\");
                    cl->createNewChildElement ("ProgramDataBaseFileName")->addTextElement ("$(IntDir)\\");
                    cl->createNewChildElement ("WarningLevel")->addTextElement ("Level" + String (config.getWarningLevel()));
                    cl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                    cl->createNewChildElement ("MultiProcessorCompilation")->addTextElement (config.shouldUseMultiProcessorCompilation() ? "true" : "false");

                    if (config.isFastMathEnabled())
                        cl->createNewChildElement ("FloatingPointModel")->addTextElement ("Fast");

                    auto extraFlags = getOwner().replacePreprocessorTokens (config, getOwner().getExtraCompilerFlagsString()).trim();
                    if (extraFlags.isNotEmpty())
                        cl->createNewChildElement ("AdditionalOptions")->addTextElement (extraFlags + " %(AdditionalOptions)");

                    if (config.areWarningsTreatedAsErrors())
                        cl->createNewChildElement ("TreatWarningAsError")->addTextElement ("true");

                    auto cppStandard = owner.project.getCppStandardString();

                    if (cppStandard == "11") // VS doesn't support the C++11 flag so we have to bump it to C++14
                        cppStandard = "14";
                    else if (cppStandard == "17") // nor does it support the C++17 flag, so we'll just use latest for now until it's added
                        cppStandard = "latest";

                    cl->createNewChildElement ("LanguageStandard")->addTextElement ("stdcpp" + cppStandard);
                }

                {
                    auto* res = group->createNewChildElement ("ResourceCompile");
                    res->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                    : "NDEBUG;%(PreprocessorDefinitions)");
                }

                auto externalLibraries = getExternalLibraries (config, getOwner().getExternalLibrariesString());
                auto additionalDependencies = type != SharedCodeTarget && externalLibraries.isNotEmpty()
                                                        ? getOwner().replacePreprocessorTokens (config, externalLibraries).trim() + ";%(AdditionalDependencies)"
                                                        : String();

                auto librarySearchPaths = config.getLibrarySearchPaths();
                auto additionalLibraryDirs = type != SharedCodeTarget && librarySearchPaths.size() > 0
                                                       ? getOwner().replacePreprocessorTokens (config, librarySearchPaths.joinIntoString (";")) + ";%(AdditionalLibraryDirectories)"
                                                       : String();

                {
                    auto* link = group->createNewChildElement ("Link");
                    link->createNewChildElement ("OutputFile")->addTextElement (getOutputFilePath (config, type == UnityPlugIn));
                    link->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                    link->createNewChildElement ("IgnoreSpecificDefaultLibraries")->addTextElement (isDebug ? "libcmt.lib; msvcrt.lib;;%(IgnoreSpecificDefaultLibraries)"
                                                                                                            : "%(IgnoreSpecificDefaultLibraries)");
                    link->createNewChildElement ("GenerateDebugInformation")->addTextElement ((isDebug || config.shouldGenerateDebugSymbols()) ? "true" : "false");
                    link->createNewChildElement ("ProgramDatabaseFile")->addTextElement (getOwner().getIntDirFile (config, config.getOutputFilename (".pdb", true, type == UnityPlugIn)));
                    link->createNewChildElement ("SubSystem")->addTextElement (type == ConsoleApp ? "Console" : "Windows");

                    if (! config.is64Bit())
                        link->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");

                    if (isUsingEditAndContinue)
                        link->createNewChildElement ("ImageHasSafeExceptionHandlers")->addTextElement ("false");

                    if (! isDebug)
                    {
                        link->createNewChildElement ("OptimizeReferences")->addTextElement ("true");
                        link->createNewChildElement ("EnableCOMDATFolding")->addTextElement ("true");
                    }

                    if (additionalLibraryDirs.isNotEmpty())
                        link->createNewChildElement ("AdditionalLibraryDirectories")->addTextElement (additionalLibraryDirs);

                    link->createNewChildElement ("LargeAddressAware")->addTextElement ("true");

                    if (additionalDependencies.isNotEmpty())
                        link->createNewChildElement ("AdditionalDependencies")->addTextElement (additionalDependencies);

                    auto extraLinkerOptions = getOwner().getExtraLinkerFlagsString();
                    if (extraLinkerOptions.isNotEmpty())
                        link->createNewChildElement ("AdditionalOptions")->addTextElement (getOwner().replacePreprocessorTokens (config, extraLinkerOptions).trim()
                                                                                           + " %(AdditionalOptions)");

                    auto delayLoadedDLLs = getDelayLoadedDLLs();
                    if (delayLoadedDLLs.isNotEmpty())
                        link->createNewChildElement ("DelayLoadDLLs")->addTextElement (delayLoadedDLLs);

                    auto moduleDefinitionsFile = getModuleDefinitions (config);
                    if (moduleDefinitionsFile.isNotEmpty())
                        link->createNewChildElement ("ModuleDefinitionFile")
                            ->addTextElement (moduleDefinitionsFile);
                }

                {
                    auto* bsc = group->createNewChildElement ("Bscmake");
                    bsc->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                    bsc->createNewChildElement ("OutputFile")->addTextElement (getOwner().getIntDirFile (config, config.getOutputFilename (".bsc", true, type == UnityPlugIn)));
                }

                if (type != SharedCodeTarget)
                {
                    auto* lib = group->createNewChildElement ("Lib");

                    if (additionalDependencies.isNotEmpty())
                        lib->createNewChildElement ("AdditionalDependencies")->addTextElement (additionalDependencies);

                    if (additionalLibraryDirs.isNotEmpty())
                        lib->createNewChildElement ("AdditionalLibraryDirectories")->addTextElement (additionalLibraryDirs);
                }

                auto manifestFile = getOwner().getManifestPath();
                if (manifestFile.getRoot() != RelativePath::unknown)
                {
                    auto* bsc = group->createNewChildElement ("Manifest");
                    bsc->createNewChildElement ("AdditionalManifestFiles")
                       ->addTextElement (manifestFile.rebased (getOwner().getProject().getFile().getParentDirectory(),
                                                               getOwner().getTargetFolder(),
                                                               RelativePath::buildTargetFolder).toWindowsStyle());
                }

                if (getTargetFileType() == staticLibrary && ! config.is64Bit())
                {
                    auto* lib = group->createNewChildElement ("Lib");
                    lib->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");
                }

                auto preBuild = getPreBuildSteps (config);
                if (preBuild.isNotEmpty())
                    group->createNewChildElement ("PreBuildEvent")
                         ->createNewChildElement ("Command")
                         ->addTextElement (preBuild);

                auto postBuild = getPostBuildSteps (config);
                if (postBuild.isNotEmpty())
                    group->createNewChildElement ("PostBuildEvent")
                         ->createNewChildElement ("Command")
                         ->addTextElement (postBuild);
            }

            std::unique_ptr<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

            {
                auto* cppFiles    = projectXml.createNewChildElement ("ItemGroup");
                auto* headerFiles = projectXml.createNewChildElement ("ItemGroup");

                for (int i = 0; i < getOwner().getAllGroups().size(); ++i)
                {
                    auto& group = getOwner().getAllGroups().getReference (i);

                    if (group.getNumChildren() > 0)
                        addFilesToCompile (group, *cppFiles, *headerFiles, *otherFilesGroup);
                }
            }

            if (getOwner().iconFile != File())
            {
                auto* e = otherFilesGroup->createNewChildElement ("None");
                e->setAttribute ("Include", prependDot (getOwner().iconFile.getFileName()));
            }

            if (otherFilesGroup->getFirstChildElement() != nullptr)
                projectXml.addChildElement (otherFilesGroup.release());

            if (type != SharedCodeTarget && getOwner().hasResourceFile())
            {
                auto* rcGroup = projectXml.createNewChildElement ("ItemGroup");
                auto* e = rcGroup->createNewChildElement ("ResourceCompile");
                e->setAttribute ("Include", prependDot (getOwner().rcFile.getFileName()));
            }

            {
                auto* e = projectXml.createNewChildElement ("Import");
                e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");
            }

            {
                auto* e = projectXml.createNewChildElement ("ImportGroup");
                e->setAttribute ("Label", "ExtensionTargets");
            }
        }

        String getProjectType() const
        {
            switch (getTargetFileType())
            {
                case executable:
                    return "Application";
                case staticLibrary:
                    return "StaticLibrary";
                default:
                    break;
            }

            return "DynamicLibrary";
        }

        //==============================================================================
        void addFilesToCompile (const Project::Item& projectItem, XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
        {
            auto targetType = (getOwner().getProject().getProjectType().isAudioPlugin() ? type : SharedCodeTarget);

            if (projectItem.isGroup())
            {
                for (int i = 0; i < projectItem.getNumChildren(); ++i)
                    addFilesToCompile (projectItem.getChild (i), cpps, headers, otherFiles);
            }
            else if (projectItem.shouldBeAddedToTargetProject()
                     && getOwner().getProject().getTargetTypeFromFilePath (projectItem.getFile(), true) == targetType)
            {
                RelativePath path (projectItem.getFile(), getOwner().getTargetFolder(), RelativePath::buildTargetFolder);

                jassert (path.getRoot() == RelativePath::buildTargetFolder);

                if (path.hasFileExtension (cOrCppFileExtensions) || path.hasFileExtension (asmFileExtensions))
                {
                    if (targetType == SharedCodeTarget || projectItem.shouldBeCompiled())
                    {
                        auto* e = cpps.createNewChildElement ("ClCompile");
                        e->setAttribute ("Include", path.toWindowsStyle());

                        if (shouldUseStdCall (path))
                            e->createNewChildElement ("CallingConvention")->addTextElement ("StdCall");

                        if (! projectItem.shouldBeCompiled())
                            e->createNewChildElement ("ExcludedFromBuild")->addTextElement ("true");
                    }
                }
                else if (path.hasFileExtension (headerFileExtensions))
                {
                    headers.createNewChildElement ("ClInclude")->setAttribute ("Include", path.toWindowsStyle());
                }
                else if (! path.hasFileExtension (objCFileExtensions))
                {
                    otherFiles.createNewChildElement ("None")->setAttribute ("Include", path.toWindowsStyle());
                }
            }
        }

        void setConditionAttribute (XmlElement& xml, const BuildConfiguration& config) const
        {
            auto& msvcConfig = dynamic_cast<const MSVCBuildConfiguration&> (config);
            xml.setAttribute ("Condition", "'$(Configuration)|$(Platform)'=='" + msvcConfig.createMSVCConfigName() + "'");
        }

        //==============================================================================
        void addFilterGroup (XmlElement& groups, const String& path) const
        {
            auto* e = groups.createNewChildElement ("Filter");
            e->setAttribute ("Include", path);
            e->createNewChildElement ("UniqueIdentifier")->addTextElement (createGUID (path + "_guidpathsaltxhsdf"));
        }

        void addFileToFilter (const RelativePath& file, const String& groupPath,
                              XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
        {
            XmlElement* e = nullptr;

            if (file.hasFileExtension (headerFileExtensions))
                e = headers.createNewChildElement ("ClInclude");
            else if (file.hasFileExtension (sourceFileExtensions))
                e = cpps.createNewChildElement ("ClCompile");
            else
                e = otherFiles.createNewChildElement ("None");

            jassert (file.getRoot() == RelativePath::buildTargetFolder);
            e->setAttribute ("Include", file.toWindowsStyle());
            e->createNewChildElement ("Filter")->addTextElement (groupPath);
        }

        bool addFilesToFilter (const Project::Item& projectItem, const String& path,
                               XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups) const
        {
            auto targetType = (getOwner().getProject().getProjectType().isAudioPlugin() ? type : SharedCodeTarget);

            if (projectItem.isGroup())
            {
                bool filesWereAdded = false;

                for (int i = 0; i < projectItem.getNumChildren(); ++i)
                    if (addFilesToFilter (projectItem.getChild(i),
                                          (path.isEmpty() ? String() : (path + "\\")) + projectItem.getChild(i).getName(),
                                          cpps, headers, otherFiles, groups))
                        filesWereAdded = true;

                if (filesWereAdded)
                    addFilterGroup (groups, path);

                return filesWereAdded;
            }
            else if (projectItem.shouldBeAddedToTargetProject())
            {
                RelativePath relativePath (projectItem.getFile(), getOwner().getTargetFolder(), RelativePath::buildTargetFolder);

                jassert (relativePath.getRoot() == RelativePath::buildTargetFolder);

                if (getOwner().getProject().getTargetTypeFromFilePath (projectItem.getFile(), true) == targetType
                    && (targetType == SharedCodeTarget || projectItem.shouldBeCompiled()))
                {
                    addFileToFilter (relativePath, path.upToLastOccurrenceOf ("\\", false, false), cpps, headers, otherFiles);
                    return true;
                }
            }

            return false;
        }

        bool addFilesToFilter (const Array<RelativePath>& files, const String& path,
                               XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups)
        {
            if (files.size() > 0)
            {
                addFilterGroup (groups, path);

                for (int i = 0; i < files.size(); ++i)
                    addFileToFilter (files.getReference(i), path, cpps, headers, otherFiles);

                return true;
            }

            return false;
        }

        void fillInFiltersXml (XmlElement& filterXml) const
        {
            filterXml.setAttribute ("ToolsVersion", getOwner().getToolsVersion());
            filterXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

            auto* groupsXml  = filterXml.createNewChildElement ("ItemGroup");
            auto* cpps       = filterXml.createNewChildElement ("ItemGroup");
            auto* headers    = filterXml.createNewChildElement ("ItemGroup");
            std::unique_ptr<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

            for (int i = 0; i < getOwner().getAllGroups().size(); ++i)
            {
                auto& group = getOwner().getAllGroups().getReference(i);

                if (group.getNumChildren() > 0)
                    addFilesToFilter (group, group.getName(), *cpps, *headers, *otherFilesGroup, *groupsXml);
            }

            if (getOwner().iconFile.exists())
            {
                auto* e = otherFilesGroup->createNewChildElement ("None");
                e->setAttribute ("Include", prependDot (getOwner().iconFile.getFileName()));
                e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getJuceCodeGroupName());
            }

            if (otherFilesGroup->getFirstChildElement() != nullptr)
                filterXml.addChildElement (otherFilesGroup.release());

            if (type != SharedCodeTarget && getOwner().hasResourceFile())
            {
                auto* rcGroup = filterXml.createNewChildElement ("ItemGroup");
                auto* e = rcGroup->createNewChildElement ("ResourceCompile");
                e->setAttribute ("Include", prependDot (getOwner().rcFile.getFileName()));
                e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getJuceCodeGroupName());
            }
        }

        const MSVCProjectExporterBase& getOwner() const { return owner; }
        const String& getProjectGuid() const            { return projectGuid; }

        //==============================================================================
        void writeProjectFile()
        {
            {
                XmlElement projectXml (getTopLevelXmlEntity());
                fillInProjectXml (projectXml);
                writeXmlOrThrow (projectXml, getVCProjFile(), "UTF-8", 10);
            }

            {
                XmlElement filtersXml (getTopLevelXmlEntity());
                fillInFiltersXml (filtersXml);
                writeXmlOrThrow (filtersXml, getVCProjFiltersFile(), "UTF-8", 100);
            }
        }

        String getSolutionTargetPath (const BuildConfiguration& config) const
        {
            auto binaryPath = config.getTargetBinaryRelativePathString().trim();
            if (binaryPath.isEmpty())
                return "$(SolutionDir)$(Platform)\\$(Configuration)";

            RelativePath binaryRelPath (binaryPath, RelativePath::projectFolder);

            if (binaryRelPath.isAbsolute())
                return binaryRelPath.toWindowsStyle();

            return prependDot (binaryRelPath.rebased (getOwner().projectFolder, getOwner().getTargetFolder(), RelativePath::buildTargetFolder)
                               .toWindowsStyle());
        }

        String getConfigTargetPath (const BuildConfiguration& config) const
        {
            auto solutionTargetFolder = getSolutionTargetPath (config);
            return solutionTargetFolder + "\\" + getName();
        }

        String getIntermediatesPath (const MSVCBuildConfiguration& config) const
        {
            auto intDir = (config.getIntermediatesPathString().isNotEmpty() ? config.getIntermediatesPathString()
                                                                            : "$(Platform)\\$(Configuration)");

            if (! intDir.endsWithChar (L'\\'))
                intDir += L'\\';

            return intDir + getName();
        }

        static const char* getOptimisationLevelString (int level)
        {
            switch (level)
            {
                case optimiseMinSize:   return "MinSpace";
                case optimiseMaxSpeed:  return "MaxSpeed";
                case optimiseFull:      return "Full";
                default:                return "Disabled";
            }
        }

        String getTargetSuffix() const
        {
            auto fileType = getTargetFileType();

            switch (fileType)
            {
                case executable:            return ".exe";
                case staticLibrary:         return ".lib";
                case sharedLibraryOrDLL:    return ".dll";

                case pluginBundle:
                    switch (type)
                    {
                        case VST3PlugIn:    return ".vst3";
                        case AAXPlugIn:     return ".aaxdll";
                        case RTASPlugIn:    return ".dpm";
                        default:            break;
                    }

                    return ".dll";

                default:
                    break;
            }

            return {};
        }

        XmlElement* createToolElement (XmlElement& parent, const String& toolName) const
        {
            auto* e = parent.createNewChildElement ("Tool");
            e->setAttribute ("Name", toolName);
            return e;
        }

        String getPreprocessorDefs (const BuildConfiguration& config, const String& joinString) const
        {
            auto defines = getOwner().msvcExtraPreprocessorDefs;
            defines.set ("WIN32", "");
            defines.set ("_WINDOWS", "");

            if (config.isDebug())
            {
                defines.set ("DEBUG", "");
                defines.set ("_DEBUG", "");
            }
            else
            {
                defines.set ("NDEBUG", "");
            }

            defines = mergePreprocessorDefs (defines, getOwner().getAllPreprocessorDefs (config, type));
            addExtraPreprocessorDefines (defines);

            if (getTargetFileType() == staticLibrary || getTargetFileType() == sharedLibraryOrDLL)
                defines.set("_LIB", "");

            StringArray result;

            for (int i = 0; i < defines.size(); ++i)
            {
                auto def = defines.getAllKeys()[i];
                auto value = defines.getAllValues()[i];
                if (value.isNotEmpty())
                    def << "=" << value;

                result.add (def);
            }

            return result.joinIntoString (joinString);
        }

        //==============================================================================
        RelativePath getAAXIconFile() const
        {
            RelativePath aaxSDK (owner.getAAXPathString(), RelativePath::projectFolder);
            RelativePath projectIcon ("icon.ico", RelativePath::buildTargetFolder);

            if (getOwner().getTargetFolder().getChildFile ("icon.ico").existsAsFile())
                return projectIcon.rebased (getOwner().getTargetFolder(),
                                            getOwner().getProject().getProjectFolder(),
                                            RelativePath::projectFolder);

            return aaxSDK.getChildFile ("Utilities").getChildFile ("PlugIn.ico");
        }

        String getExtraPostBuildSteps (const MSVCBuildConfiguration& config) const
        {
            if (type == AAXPlugIn)
            {
                RelativePath aaxSDK (owner.getAAXPathString(), RelativePath::projectFolder);
                RelativePath aaxLibsFolder = aaxSDK.getChildFile ("Libs");
                RelativePath bundleScript  = aaxSDK.getChildFile ("Utilities").getChildFile ("CreatePackage.bat");
                RelativePath iconFilePath  = getAAXIconFile();

                auto outputFilename = config.getOutputFilename (".aaxplugin", true, false);
                auto bundleDir      = getOwner().getOutDirFile (config, outputFilename);
                auto bundleContents = bundleDir + "\\Contents";
                auto archDir        = bundleContents + String ("\\") + (config.is64Bit() ? "x64" : "Win32");
                auto executable     = archDir + String ("\\") + outputFilename;

                auto pkgScript = String ("copy /Y ") + getOutputFilePath (config, false).quoted() + String (" ") + executable.quoted() + String ("\r\ncall ")
                                     + createRebasedPath (bundleScript) + String (" ") + archDir.quoted() + String (" ") + createRebasedPath (iconFilePath);

                if (config.isPluginBinaryCopyStepEnabled())
                    return pkgScript + "\r\n" + "xcopy " + bundleDir.quoted() + " "
                               + String (config.getAAXBinaryLocationString() + "\\" + outputFilename + "\\").quoted() + " /E /H /K /R /Y";

                return pkgScript;
            }
            else if (type == UnityPlugIn)
            {
                RelativePath scriptPath (config.project.getGeneratedCodeFolder().getChildFile (config.project.getUnityScriptName()),
                                         getOwner().getTargetFolder(),
                                         RelativePath::projectFolder);

                auto pkgScript = String ("copy /Y ") + scriptPath.toWindowsStyle().quoted() + " \"$(OutDir)\"";

                if (config.isPluginBinaryCopyStepEnabled())
                {
                    auto copyLocation = config.getUnityPluginBinaryLocationString();

                    pkgScript += "\r\ncopy /Y \"$(OutDir)$(TargetFileName)\" " +  String (copyLocation + "\\$(TargetFileName)").quoted();
                    pkgScript += "\r\ncopy /Y " + String ("$(OutDir)" + config.project.getUnityScriptName()).quoted() + " " + String (copyLocation + "\\" + config.project.getUnityScriptName()).quoted();
                }

                return pkgScript;
            }
            else if (config.isPluginBinaryCopyStepEnabled())
            {
                auto copyScript = String ("copy /Y \"$(OutDir)$(TargetFileName)\"") + String (" \"$COPYDIR$\\$(TargetFileName)\"");

                if (type == VSTPlugIn)     return copyScript.replace ("$COPYDIR$", config.getVSTBinaryLocationString());
                if (type == VST3PlugIn)    return copyScript.replace ("$COPYDIR$", config.getVST3BinaryLocationString());
                if (type == RTASPlugIn)    return copyScript.replace ("$COPYDIR$", config.getRTASBinaryLocationString());
            }

            return {};
        }

        String getExtraPreBuildSteps (const MSVCBuildConfiguration& config) const
        {
            if (type == AAXPlugIn)
            {
                String script;

                auto bundleDir      = getOwner().getOutDirFile (config, config.getOutputFilename (".aaxplugin", false, false));
                auto bundleContents = bundleDir + "\\Contents";
                auto archDir        = bundleContents + String ("\\") + (config.is64Bit() ? "x64" : "Win32");

                for (auto& folder : StringArray { bundleDir, bundleContents, archDir })
                    script += String ("if not exist \"") + folder + String ("\" mkdir \"") + folder + String ("\"\r\n");

                return script;
            }

            return {};
        }

        String getPostBuildSteps (const MSVCBuildConfiguration& config) const
        {
            auto postBuild = config.getPostbuildCommandString();
            auto extraPostBuild = getExtraPostBuildSteps (config);

            return postBuild + String (postBuild.isNotEmpty() && extraPostBuild.isNotEmpty() ? "\r\n" : "") + extraPostBuild;
        }

        String getPreBuildSteps (const MSVCBuildConfiguration& config) const
        {
            auto preBuild = config.getPrebuildCommandString();
            auto extraPreBuild = getExtraPreBuildSteps (config);

            return preBuild + String (preBuild.isNotEmpty() && extraPreBuild.isNotEmpty() ? "\r\n" : "") + extraPreBuild;
        }

        void addExtraPreprocessorDefines (StringPairArray& defines) const
        {
            switch (type)
            {
            case AAXPlugIn:
                {
                    auto aaxLibsFolder = RelativePath (owner.getAAXPathString(), RelativePath::projectFolder).getChildFile ("Libs");
                    defines.set ("JucePlugin_AAXLibs_path", createRebasedPath (aaxLibsFolder));
                }
                break;
            case RTASPlugIn:
                {
                    RelativePath rtasFolder (owner.getRTASPathString(), RelativePath::projectFolder);
                    defines.set ("JucePlugin_WinBag_path", createRebasedPath (rtasFolder.getChildFile ("WinBag")));
                }
                break;
            default:
                break;
            }
        }

        String getExtraLinkerFlags() const
        {
            if (type == RTASPlugIn)
                return "/FORCE:multiple";

            return {};
        }

        StringArray getExtraSearchPaths() const
        {
            StringArray searchPaths;
            if (type == RTASPlugIn)
            {
                RelativePath rtasFolder (owner.getRTASPathString(), RelativePath::projectFolder);

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

                for (auto* path : p)
                    searchPaths.add (createRebasedPath (rtasFolder.getChildFile (path)));
            }

            return searchPaths;
        }

        String getBinaryNameWithSuffix (const MSVCBuildConfiguration& config, bool forceUnityPrefix) const
        {
            return config.getOutputFilename (getTargetSuffix(), true, forceUnityPrefix);
        }

        String getOutputFilePath (const MSVCBuildConfiguration& config, bool forceUnityPrefix) const
        {
            return getOwner().getOutDirFile (config, getBinaryNameWithSuffix (config, forceUnityPrefix));
        }

        StringArray getLibrarySearchPaths (const BuildConfiguration& config) const
        {
            auto librarySearchPaths = config.getLibrarySearchPaths();

            if (type != SharedCodeTarget)
                if (auto* shared = getOwner().getSharedCodeTarget())
                    librarySearchPaths.add (shared->getConfigTargetPath (config));

            return librarySearchPaths;
        }

        String getExternalLibraries (const MSVCBuildConfiguration& config, const String& otherLibs) const
        {
            StringArray libraries;

            if (otherLibs.isNotEmpty())
                libraries.add (otherLibs);

            auto moduleLibs = getOwner().getModuleLibs();
            if (! moduleLibs.isEmpty())
                libraries.addArray (moduleLibs);

            if (type != SharedCodeTarget)
                if (auto* shared = getOwner().getSharedCodeTarget())
                    libraries.add (shared->getBinaryNameWithSuffix (config, false));

            return libraries.joinIntoString (";");
        }

        String getDelayLoadedDLLs() const
        {
            auto delayLoadedDLLs = getOwner().msvcDelayLoadedDLLs;

            if (type == RTASPlugIn)
                delayLoadedDLLs += "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; "
                    "DSPManager.dll; DSPManager.dll; DSPManagerClientLib.dll; RTASClientLib.dll";

            return delayLoadedDLLs;
        }

        String getModuleDefinitions (const MSVCBuildConfiguration& config) const
        {
            auto moduleDefinitions = config.config [Ids::msvcModuleDefinitionFile].toString();

            if (moduleDefinitions.isNotEmpty())
                return moduleDefinitions;

            if (type == RTASPlugIn)
            {
                auto& exp = getOwner();

                auto moduleDefPath
                    = RelativePath (exp.getPathForModuleString ("juce_audio_plugin_client"), RelativePath::projectFolder)
                         .getChildFile ("juce_audio_plugin_client").getChildFile ("RTAS").getChildFile ("juce_RTAS_WinExports.def");

                return prependDot (moduleDefPath.rebased (exp.getProject().getProjectFolder(),
                                                            exp.getTargetFolder(),
                                                            RelativePath::buildTargetFolder).toWindowsStyle());
            }

            return {};
        }

        File getVCProjFile() const                                   { return getOwner().getProjectFile (getProjectFileSuffix(), getName()); }
        File getVCProjFiltersFile() const                            { return getOwner().getProjectFile (getFiltersFileSuffix(), getName()); }

        String createRebasedPath (const RelativePath& path) const    {  return getOwner().createRebasedPath (path); }

        void addWindowsTargetPlatformToConfig (XmlElement& e) const
        {
            auto target = owner.getWindowsTargetPlatformVersion();

            if (target == "Latest")
            {
                auto* child = e.createNewChildElement ("WindowsTargetPlatformVersion");

                child->setAttribute ("Condition", "'$(WindowsTargetPlatformVersion)' == ''");
                child->addTextElement ("$([Microsoft.Build.Utilities.ToolLocationHelper]::GetLatestSDKTargetPlatformVersion('Windows', '10.0'))");
            }
            else
            {
                e.createNewChildElement ("WindowsTargetPlatformVersion")->addTextElement (target);
            }
        }

    protected:
        const MSVCProjectExporterBase& owner;
        String projectGuid;
    };

    //==============================================================================
    bool usesMMFiles() const override            { return false; }
    bool canCopeWithDuplicateFiles() override    { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                { return false; }
    bool isVisualStudio() const override         { return true; }
    bool isCodeBlocks() const override           { return false; }
    bool isMakefile() const override             { return false; }
    bool isAndroidStudio() const override        { return false; }
    bool isCLion() const override                { return false; }

    bool isAndroid() const override              { return false; }
    bool isWindows() const override              { return true; }
    bool isLinux() const override                { return false; }
    bool isOSX() const override                  { return false; }
    bool isiOS() const override                  { return false; }

    bool supportsTargetType (ProjectType::Target::Type type) const override
    {
        switch (type)
        {
        case ProjectType::Target::StandalonePlugIn:
        case ProjectType::Target::GUIApp:
        case ProjectType::Target::ConsoleApp:
        case ProjectType::Target::StaticLibrary:
        case ProjectType::Target::SharedCodeTarget:
        case ProjectType::Target::AggregateTarget:
        case ProjectType::Target::VSTPlugIn:
        case ProjectType::Target::VST3PlugIn:
        case ProjectType::Target::AAXPlugIn:
        case ProjectType::Target::RTASPlugIn:
        case ProjectType::Target::UnityPlugIn:
        case ProjectType::Target::DynamicLibrary:
            return true;
        default:
            break;
        }

        return false;
    }

    //==============================================================================
    RelativePath getManifestPath() const
    {
        auto path = manifestFileValue.get().toString();

        return path.isEmpty() ? RelativePath()
                              : RelativePath (path, RelativePath::projectFolder);
    }

    //==============================================================================
    bool launchProject() override
    {
       #if JUCE_WINDOWS
        return getSLNFile().startAsProcess();
       #else
        return false;
       #endif
    }

    bool canLaunchProject() override
    {
       #if JUCE_WINDOWS
        return true;
       #else
        return false;
       #endif
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        props.add (new TextPropertyComponent (manifestFileValue, "Manifest file", 8192, false),
            "Path to a manifest input file which should be linked into your binary (path is relative to jucer file).");
    }

    enum OptimisationLevel
    {
        optimisationOff = 1,
        optimiseMinSize = 2,
        optimiseFull = 3,
        optimiseMaxSpeed = 4
    };

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType& type) override
    {
        msvcExtraPreprocessorDefs.set ("_CRT_SECURE_NO_WARNINGS", "");

        if (type.isCommandLineApp())
            msvcExtraPreprocessorDefs.set("_CONSOLE", "");

        callForAllSupportedTargets ([this] (ProjectType::Target::Type targetType)
                                    {
                                        if (MSVCTargetBase* target = new MSVCTargetBase (targetType, *this))
                                        {
                                            if (targetType != ProjectType::Target::AggregateTarget)
                                                targets.add (target);
                                        }
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

    const MSVCTargetBase* getSharedCodeTarget() const
    {
        for (auto target : targets)
            if (target->type == ProjectType::Target::SharedCodeTarget)
                return target;

        return nullptr;
    }

    bool hasTarget (ProjectType::Target::Type type) const
    {
        for (auto target : targets)
            if (target->type == type)
                return true;

        return false;
    }

private:
    //==============================================================================
    String createRebasedPath (const RelativePath& path) const
    {
        auto rebasedPath = rebaseFromProjectFolderToBuildTarget (path).toWindowsStyle();

        return getVisualStudioVersion() < 10  // (VS10 automatically adds escape characters to the quotes for this definition)
                                          ? CppTokeniserFunctions::addEscapeChars (rebasedPath.quoted())
                                          : CppTokeniserFunctions::addEscapeChars (rebasedPath).quoted();
    }

protected:
    //==============================================================================
    mutable File rcFile, iconFile;
    OwnedArray<MSVCTargetBase> targets;

    ValueWithDefault IPPLibraryValue, platformToolsetValue, targetPlatformVersion, manifestFileValue;

    File getProjectFile (const String& extension, const String& target) const
    {
        auto filename = project.getProjectFilenameRootString();

        if (target.isNotEmpty())
            filename += String ("_") + target.removeCharacters (" ");

        return getTargetFolder().getChildFile (filename).withFileExtension (extension);
    }

    File getSLNFile() const     { return getProjectFile (".sln", String()); }

    static String prependIfNotAbsolute (const String& file, const char* prefix)
    {
        if (File::isAbsolutePath (file) || file.startsWithChar ('$'))
            prefix = "";

        return prefix + FileHelpers::windowsStylePath (file);
    }

    String getIntDirFile (const BuildConfiguration& config, const String& file) const  { return prependIfNotAbsolute (replacePreprocessorTokens (config, file), "$(IntDir)\\"); }
    String getOutDirFile (const BuildConfiguration& config, const String& file) const  { return prependIfNotAbsolute (replacePreprocessorTokens (config, file), "$(OutDir)\\"); }

    void updateOldSettings()
    {
        {
            auto oldStylePrebuildCommand = getSettingString (Ids::prebuildCommand);
            settings.removeProperty (Ids::prebuildCommand, nullptr);

            if (oldStylePrebuildCommand.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    dynamic_cast<MSVCBuildConfiguration&> (*config).getValue (Ids::prebuildCommand) = oldStylePrebuildCommand;
        }

        {
            auto oldStyleLibName = getSettingString ("libraryName_Debug");
            settings.removeProperty ("libraryName_Debug", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (config->isDebug())
                        config->getValue (Ids::targetName) = oldStyleLibName;
        }

        {
            auto oldStyleLibName = getSettingString ("libraryName_Release");
            settings.removeProperty ("libraryName_Release", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (! config->isDebug())
                        config->getValue (Ids::targetName) = oldStyleLibName;
        }
    }

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return *new MSVCBuildConfiguration (project, v, *this);
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        auto searchPaths = extraSearchPaths;
        searchPaths.addArray (config.getHeaderSearchPaths());
        return getCleanedStringArray (searchPaths);
    }

    String getSharedCodeGuid() const
    {
        String sharedCodeGuid;

        for (int i = 0; i < targets.size(); ++i)
            if (auto* target = targets[i])
                if (target->type == ProjectType::Target::SharedCodeTarget)
                    return target->getProjectGuid();

        return {};
    }

    //==============================================================================
    void writeProjectDependencies (OutputStream& out) const
    {
        auto sharedCodeGuid = getSharedCodeGuid();

        for (int addingOtherTargets = 0; addingOtherTargets < (sharedCodeGuid.isNotEmpty() ? 2 : 1); ++addingOtherTargets)
        {
            for (int i = 0; i < targets.size(); ++i)
            {
                if (auto* target = targets[i])
                {
                    if (sharedCodeGuid.isEmpty() || (addingOtherTargets != 0) == (target->type != ProjectType::Target::StandalonePlugIn))
                    {
                        out << "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"" << projectName << " - "
                            << target->getName() << "\", \""
                            << target->getVCProjFile().getFileName() << "\", \"" << target->getProjectGuid() << '"' << newLine;

                        if (sharedCodeGuid.isNotEmpty() && target->type != ProjectType::Target::SharedCodeTarget)
                            out << "\tProjectSection(ProjectDependencies) = postProject" << newLine
                                << "\t\t" << sharedCodeGuid << " = " << sharedCodeGuid << newLine
                                << "\tEndProjectSection" << newLine;

                        out << "EndProject" << newLine;
                    }
                }
            }
        }
    }

    void writeSolutionFile (OutputStream& out, const String& versionString, String commentString) const
    {
        if (commentString.isNotEmpty())
            commentString += newLine;

        out << "Microsoft Visual Studio Solution File, Format Version " << versionString << newLine
            << commentString << newLine;

        writeProjectDependencies (out);

        out << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        for (ConstConfigIterator i (*this); i.next();)
        {
            auto& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);
            auto configName = config.createMSVCConfigName();
            out << "\t\t" << configName << " = " << configName << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (auto& target : targets)
            for (ConstConfigIterator i (*this); i.next();)
            {
                auto& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);
                auto configName = config.createMSVCConfigName();

                for (auto& suffix : { "ActiveCfg", "Build.0" })
                    out << "\t\t" << target->getProjectGuid() << "." << configName << "." << suffix << " = " << configName << newLine;
            }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(SolutionProperties) = preSolution" << newLine
            << "\t\tHideSolutionNode = FALSE" << newLine
            << "\tEndGlobalSection" << newLine;

        out << "EndGlobal" << newLine;
    }

    //==============================================================================
    static void writeBMPImage (const Image& image, const int w, const int h, MemoryOutputStream& out)
    {
        int maskStride = (w / 8 + 3) & ~3;

        out.writeInt (40); // bitmapinfoheader size
        out.writeInt (w);
        out.writeInt (h * 2);
        out.writeShort (1); // planes
        out.writeShort (32); // bits
        out.writeInt (0); // compression
        out.writeInt ((h * w * 4) + (h * maskStride)); // size image
        out.writeInt (0); // x pixels per meter
        out.writeInt (0); // y pixels per meter
        out.writeInt (0); // clr used
        out.writeInt (0); // clr important

        Image::BitmapData bitmap (image, Image::BitmapData::readOnly);
        int alphaThreshold = 5;

        int y;
        for (y = h; --y >= 0;)
        {
            for (int x = 0; x < w; ++x)
            {
                auto pixel = bitmap.getPixelColour (x, y);

                if (pixel.getAlpha() <= alphaThreshold)
                {
                    out.writeInt (0);
                }
                else
                {
                    out.writeByte ((char) pixel.getBlue());
                    out.writeByte ((char) pixel.getGreen());
                    out.writeByte ((char) pixel.getRed());
                    out.writeByte ((char) pixel.getAlpha());
                }
            }
        }

        for (y = h; --y >= 0;)
        {
            int mask = 0, count = 0;

            for (int x = 0; x < w; ++x)
            {
                auto pixel = bitmap.getPixelColour (x, y);

                mask <<= 1;
                if (pixel.getAlpha() <= alphaThreshold)
                    mask |= 1;

                if (++count == 8)
                {
                    out.writeByte ((char) mask);
                    count = 0;
                    mask = 0;
                }
            }

            if (mask != 0)
                out.writeByte ((char) mask);

            for (int i = maskStride - w / 8; --i >= 0;)
                out.writeByte (0);
        }
    }

    static void writeIconFile (const Array<Image>& images, MemoryOutputStream& out)
    {
        out.writeShort (0); // reserved
        out.writeShort (1); // .ico tag
        out.writeShort ((short) images.size());

        MemoryOutputStream dataBlock;

        int imageDirEntrySize = 16;
        int dataBlockStart = 6 + images.size() * imageDirEntrySize;

        for (int i = 0; i < images.size(); ++i)
        {
            auto oldDataSize = dataBlock.getDataSize();

            auto& image = images.getReference (i);
            auto w = image.getWidth();
            auto h = image.getHeight();

            if (w >= 256 || h >= 256)
            {
                PNGImageFormat pngFormat;
                pngFormat.writeImageToStream (image, dataBlock);
            }
            else
            {
                writeBMPImage (image, w, h, dataBlock);
            }

            out.writeByte ((char) w);
            out.writeByte ((char) h);
            out.writeByte (0);
            out.writeByte (0);
            out.writeShort (1); // colour planes
            out.writeShort (32); // bits per pixel
            out.writeInt ((int) (dataBlock.getDataSize() - oldDataSize));
            out.writeInt (dataBlockStart + (int) oldDataSize);
        }

        jassert (out.getPosition() == dataBlockStart);
        out << dataBlock;
    }

    bool hasResourceFile() const
    {
        return ! projectType.isStaticLibrary();
    }

    void createResourcesAndIcon() const
    {
        if (hasResourceFile())
        {
            Array<Image> images;
            int sizes[] = { 16, 32, 48, 256 };

            for (int i = 0; i < numElementsInArray (sizes); ++i)
            {
                auto im = getBestIconForSize (sizes[i], true);
                if (im.isValid())
                    images.add (im);
            }

            if (images.size() > 0)
            {
                iconFile = getTargetFolder().getChildFile ("icon.ico");

                MemoryOutputStream mo;
                writeIconFile (images, mo);
                overwriteFileIfDifferentOrThrow (iconFile, mo);
            }

            createRCFile();
        }
    }

    void createRCFile() const
    {
        rcFile = getTargetFolder().getChildFile ("resources.rc");

        auto version = project.getVersionString();

        MemoryOutputStream mo;

        mo << "#ifdef JUCE_USER_DEFINED_RC_FILE" << newLine
           << " #include JUCE_USER_DEFINED_RC_FILE" << newLine
           << "#else" << newLine
           << newLine
           << "#undef  WIN32_LEAN_AND_MEAN" << newLine
           << "#define WIN32_LEAN_AND_MEAN" << newLine
           << "#include <windows.h>" << newLine
           << newLine
           << "VS_VERSION_INFO VERSIONINFO" << newLine
           << "FILEVERSION  " << getCommaSeparatedVersionNumber (version) << newLine
           << "BEGIN" << newLine
           << "  BLOCK \"StringFileInfo\"" << newLine
           << "  BEGIN" << newLine
           << "    BLOCK \"040904E4\"" << newLine
           << "    BEGIN" << newLine;

        writeRCValue (mo, "CompanyName", project.getCompanyNameString());
        writeRCValue (mo, "LegalCopyright", project.getCompanyCopyrightString());
        writeRCValue (mo, "FileDescription", project.getProjectNameString());
        writeRCValue (mo, "FileVersion", version);
        writeRCValue (mo, "ProductName", project.getProjectNameString());
        writeRCValue (mo, "ProductVersion", version);

        mo << "    END" << newLine
           << "  END" << newLine
           << newLine
           << "  BLOCK \"VarFileInfo\"" << newLine
           << "  BEGIN" << newLine
           << "    VALUE \"Translation\", 0x409, 1252" << newLine
           << "  END" << newLine
           << "END" << newLine
           << newLine
           << "#endif" << newLine;

        if (iconFile != File())
            mo << newLine
               << "IDI_ICON1 ICON DISCARDABLE " << iconFile.getFileName().quoted()
               << newLine
               << "IDI_ICON2 ICON DISCARDABLE " << iconFile.getFileName().quoted();

        overwriteFileIfDifferentOrThrow (rcFile, mo);
    }

    static void writeRCValue (MemoryOutputStream& mo, const String& name, const String& value)
    {
        if (value.isNotEmpty())
            mo << "      VALUE \"" << name << "\",  \""
               << CppTokeniserFunctions::addEscapeChars (value) << "\\0\"" << newLine;
    }

    static String getCommaSeparatedVersionNumber (const String& version)
    {
        auto versionParts = StringArray::fromTokens (version, ",.", "");
        versionParts.trim();
        versionParts.removeEmptyStrings();
        while (versionParts.size() < 4)
            versionParts.add ("0");

        return versionParts.joinIntoString (",");
    }

    static String prependDot (const String& filename)
    {
        return FileHelpers::isAbsolutePath (filename) ? filename
                                                      : (".\\" + filename);
    }

    static bool shouldUseStdCall (const RelativePath& path)
    {
        return path.getFileNameWithoutExtension().startsWithIgnoreCase ("include_juce_audio_plugin_client_RTAS_");
    }

    StringArray getModuleLibs() const
    {
        StringArray result;

        for (auto& lib : windowsLibs)
            result.add (lib + ".lib");

        return result;
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterBase)
};

//==============================================================================
class MSVCProjectExporterVC2013  : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2013 (Project& p, const ValueTree& t)
        : MSVCProjectExporterBase (p, t, getTargetFolderForExporter (getValueTreeTypeName()))
    {
        name = getName();

        targetPlatformVersion.setDefault (getDefaultWindowsTargetPlatformVersion());
        platformToolsetValue.setDefault (getDefaultToolset());
    }

    static const char* getName()                                     { return "Visual Studio 2013"; }
    static const char* getValueTreeTypeName()                        { return "VS2013"; }
    int getVisualStudioVersion() const override                      { return 12; }
    String getSolutionComment() const override                       { return "# Visual Studio 2013"; }
    String getToolsVersion() const override                          { return "12.0"; }
    String getDefaultToolset() const override                        { return "v120"; }
    String getDefaultWindowsTargetPlatformVersion() const override   { return "8.1"; }


    static MSVCProjectExporterVC2013* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2013 (project, settings);

        return nullptr;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        MSVCProjectExporterBase::createExporterProperties (props);

        static const char* toolsetNames[] = { "v120", "v120_xp", "Windows7.1SDK", "CTP_Nov2013" };
        const var toolsets[]              = { "v120", "v120_xp", "Windows7.1SDK", "CTP_Nov2013" };
        addToolsetProperty (props, toolsetNames, toolsets, numElementsInArray (toolsets));

        addIPPLibraryProperty (props);

        addWindowsTargetPlatformProperties (props);
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2013)
};

//==============================================================================
class MSVCProjectExporterVC2015  : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2015 (Project& p, const ValueTree& t)
        : MSVCProjectExporterBase (p, t, getTargetFolderForExporter (getValueTreeTypeName()))
    {
        name = getName();

        targetPlatformVersion.setDefault (getDefaultWindowsTargetPlatformVersion());
        platformToolsetValue.setDefault (getDefaultToolset());
    }

    static const char* getName()                                     { return "Visual Studio 2015"; }
    static const char* getValueTreeTypeName()                        { return "VS2015"; }
    int getVisualStudioVersion() const override                      { return 14; }
    String getSolutionComment() const override                       { return "# Visual Studio 2015"; }
    String getToolsVersion() const override                          { return "14.0"; }
    String getDefaultToolset() const override                        { return "v140"; }
    String getDefaultWindowsTargetPlatformVersion() const override   { return "8.1"; }

    static MSVCProjectExporterVC2015* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2015 (project, settings);

        return nullptr;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        MSVCProjectExporterBase::createExporterProperties (props);

        static const char* toolsetNames[] = { "v140", "v140_xp", "CTP_Nov2013" };
        const var toolsets[]              = { "v140", "v140_xp", "CTP_Nov2013" };
        addToolsetProperty (props, toolsetNames, toolsets, numElementsInArray (toolsets));

        addIPPLibraryProperty (props);

        addWindowsTargetPlatformProperties (props);
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2015)
};

//==============================================================================
class MSVCProjectExporterVC2017  : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2017 (Project& p, const ValueTree& t)
        : MSVCProjectExporterBase (p, t, getTargetFolderForExporter (getValueTreeTypeName()))
    {
        name = getName();

        targetPlatformVersion.setDefault (getDefaultWindowsTargetPlatformVersion());
        platformToolsetValue.setDefault (getDefaultToolset());
    }

    static const char* getName()                                     { return "Visual Studio 2017"; }
    static const char* getValueTreeTypeName()                        { return "VS2017"; }
    int getVisualStudioVersion() const override                      { return 15; }
    String getSolutionComment() const override                       { return "# Visual Studio 2017"; }
    String getToolsVersion() const override                          { return "15.0"; }
    String getDefaultToolset() const override                        { return "v141"; }
    String getDefaultWindowsTargetPlatformVersion() const override   { return "Latest"; }

    static MSVCProjectExporterVC2017* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2017 (project, settings);

        return nullptr;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        MSVCProjectExporterBase::createExporterProperties (props);

        static const char* toolsetNames[] = { "v140", "v140_xp", "v141", "v141_xp" };
        const var toolsets[]              = { "v140", "v140_xp", "v141", "v141_xp" };
        addToolsetProperty (props, toolsetNames, toolsets, numElementsInArray (toolsets));

        addIPPLibraryProperty (props);

        addWindowsTargetPlatformProperties (props);
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2017)
};
