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

class MSVCProjectExporterBase   : public ProjectExporter
{
public:
    MSVCProjectExporterBase (Project& p, const ValueTree& t, const char* const folderName)
        : ProjectExporter (p, t)
    {
        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + folderName;

        updateOldSettings();

        initialiseDependencyPathValues();
    }

    virtual int getVisualStudioVersion() const = 0;

    virtual String getSolutionComment() const = 0;
    virtual String getToolsVersion() const = 0;
    virtual String getDefaultToolset() const = 0;
    virtual String getDefaultWindowsTargetPlatformVersion() const = 0;

    //==============================================================================
    Value getIPPLibraryValue()                     { return getSetting (Ids::IPPLibrary); }
    String getIPPLibrary() const                   { return settings [Ids::IPPLibrary]; }

    Value getPlatformToolsetValue()                { return getSetting (Ids::toolset); }
    String getPlatformToolset() const
    {
        const String s (settings [Ids::toolset].toString());
        return s.isNotEmpty() ? s : getDefaultToolset();
    }

    Value getWindowsTargetPlatformVersionValue()   { return getSetting (Ids::windowsTargetPlatformVersion); }
    String getWindowsTargetPlatformVersion() const
    {
        String targetPlatform = settings [Ids::windowsTargetPlatformVersion];
        return (targetPlatform.isNotEmpty() ? targetPlatform : getDefaultWindowsTargetPlatformVersion());
    }

    Value getCppStandardValue()                    { return getSetting (Ids::cppLanguageStandard); }
    String getCppLanguageStandard() const          { return settings [Ids::cppLanguageStandard]; }

    //==============================================================================
    void addToolsetProperty (PropertyListBuilder& props, const char** names, const var* values, int num)
    {
        props.add (new ChoicePropertyComponent (getPlatformToolsetValue(), "Platform Toolset",
                                                StringArray (names, num), Array<var> (values, num)));
    }

    void addIPPLibraryProperty (PropertyListBuilder& props)
    {
        static const char* ippOptions[] = { "No",  "Yes (Default Mode)", "Multi-Threaded Static Library", "Single-Threaded Static Library", "Multi-Threaded DLL", "Single-Threaded DLL" };
        static const var ippValues[]    = { var(), "true",               "Parallel_Static",               "Sequential",                     "Parallel_Dynamic",   "Sequential_Dynamic" };

        props.add (new ChoicePropertyComponent (getIPPLibraryValue(), "Use IPP Library",
                                                StringArray (ippOptions, numElementsInArray (ippValues)),
                                                Array<var> (ippValues, numElementsInArray (ippValues))));
    }

    void addCppStandardProperty (PropertyListBuilder& props, const char** names, const var* values, int num)
    {
        props.add (new ChoicePropertyComponent (getCppStandardValue(), "C++ standard to use",
                                                StringArray (names, num), Array<var> (values, num)));
    }

    void addPlatformToolsetToPropertyGroup (XmlElement& p) const
    {
        forEachXmlChildElementWithTagName (p, e, "PropertyGroup")
        e->createNewChildElement ("PlatformToolset")->addTextElement (getPlatformToolset());
    }

    void addWindowsTargetPlatformVersionToPropertyGroup (XmlElement& p) const
    {
        const String& targetVersion = getWindowsTargetPlatformVersion();

        if (targetVersion.isNotEmpty())
            forEachXmlChildElementWithTagName (p, e, "PropertyGroup")
            e->createNewChildElement ("WindowsTargetPlatformVersion")->addTextElement (getWindowsTargetPlatformVersion());
    }

    void addWindowsTargetPlatformProperties (PropertyListBuilder& props)
    {
        static const char* targetPlatformNames[] = { "(default)", "8.1", "10.0.10240.0", "10.0.10586.0", "10.0.14393.0", "10.0.15063.0", nullptr };
        const var targetPlatforms[]              = { var(),       "8.1", "10.0.10240.0", "10.0.10586.0", "10.0.14393.0", "10.0.15063.0" };

        props.add (new ChoicePropertyComponent (getWindowsTargetPlatformVersionValue(), "Windows Target Platform",
                                                StringArray (targetPlatformNames), Array<var> (targetPlatforms, numElementsInArray (targetPlatforms))),
                   "Specifies the version of the Windows SDK that will be used when building this project. "
                   "The default value for this exporter is " + getDefaultWindowsTargetPlatformVersion());
    }

    void create (const OwnedArray<LibraryModule>&) const override
    {
        createResourcesAndIcon();

        for (int i = 0; i < targets.size(); ++i)
            if (MSVCTargetBase* target = targets[i])
                target->writeProjectFile();

            {
                MemoryOutputStream mo;
                writeSolutionFile (mo, "11.00", getSolutionComment());

                overwriteFileIfDifferentOrThrow (getSLNFile(), mo);
            }
    }

    //==============================================================================
    class MSVCBuildConfiguration  : public BuildConfiguration
    {
    public:
        MSVCBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
            if (getWarningLevel() == 0)
                getWarningLevelValue() = 4;

            setValueIfVoid (shouldGenerateManifestValue(), true);
        }

        Value getWarningLevelValue()                { return getValue (Ids::winWarningLevel); }
        int getWarningLevel() const                 { return config [Ids::winWarningLevel]; }

        Value getWarningsTreatedAsErrors()          { return getValue (Ids::warningsAreErrors); }
        bool areWarningsTreatedAsErrors() const     { return config [Ids::warningsAreErrors]; }

        Value getPrebuildCommand()                  { return getValue (Ids::prebuildCommand); }
        String getPrebuildCommandString() const     { return config [Ids::prebuildCommand]; }
        Value getPostbuildCommand()                 { return getValue (Ids::postbuildCommand); }
        String getPostbuildCommandString() const    { return config [Ids::postbuildCommand]; }

        Value shouldGenerateDebugSymbolsValue()     { return getValue (Ids::alwaysGenerateDebugSymbols); }
        bool shouldGenerateDebugSymbols() const     { return config [Ids::alwaysGenerateDebugSymbols]; }

        Value shouldGenerateManifestValue()         { return getValue (Ids::generateManifest); }
        bool shouldGenerateManifest() const         { return config [Ids::generateManifest]; }

        Value shouldLinkIncrementalValue()          { return getValue (Ids::enableIncrementalLinking); }
        bool shouldLinkIncremental() const          { return config [Ids::enableIncrementalLinking]; }

        Value getWholeProgramOptValue()             { return getValue (Ids::wholeProgramOptimisation); }
        bool shouldDisableWholeProgramOpt() const   { return static_cast<int> (config [Ids::wholeProgramOptimisation]) > 0; }

        Value getUsingRuntimeLibDLL()               { return getValue (Ids::useRuntimeLibDLL); }
        bool isUsingRuntimeLibDLL() const           { return config [Ids::useRuntimeLibDLL]; }

        String getIntermediatesPath() const         { return config [Ids::intermediatesPath].toString(); }
        Value getIntermediatesPathValue()           { return getValue (Ids::intermediatesPath); }

        String getCharacterSet() const              { return config [Ids::characterSet].toString(); }
        Value getCharacterSetValue()                { return getValue (Ids::characterSet); }

        String createMSVCConfigName() const
        {
            return getName() + "|" + (config [Ids::winArchitecture] == "x64" ? "x64" : "Win32");
        }

        String getOutputFilename (const String& suffix, bool forceSuffix) const
        {
            const String target (File::createLegalFileName (getTargetBinaryNameString().trim()));

            if (forceSuffix || ! target.containsChar ('.'))
                return target.upToLastOccurrenceOf (".", false, false) + suffix;

            return target;
        }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? optimisationOff : optimiseMaxSpeed)); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            static const char* optimisationLevels[] = { "No optimisation", "Minimise size", "Maximise speed", 0 };
            const int optimisationLevelValues[]     = { optimisationOff, optimiseMinSize, optimiseMaxSpeed, 0 };

            props.add (new ChoicePropertyComponent (getOptimisationLevel(), "Optimisation",
                                                    StringArray (optimisationLevels),
                                                    Array<var> (optimisationLevelValues)),
                       "The optimisation level for this configuration");

            props.add (new TextPropertyComponent (getIntermediatesPathValue(), "Intermediates path", 2048, false),
                       "An optional path to a folder to use for the intermediate build files. Note that Visual Studio allows "
                       "you to use macros in this path, e.g. \"$(TEMP)\\MyAppBuildFiles\\$(Configuration)\", which is a handy way to "
                       "send them to the user's temp folder.");

            static const char* warningLevelNames[] = { "Low", "Medium", "High", nullptr };
            const int warningLevels[] = { 2, 3, 4 };

            props.add (new ChoicePropertyComponent (getWarningLevelValue(), "Warning Level",
                                                    StringArray (warningLevelNames), Array<var> (warningLevels, numElementsInArray (warningLevels))));

            props.add (new BooleanPropertyComponent (getWarningsTreatedAsErrors(), "Warnings", "Treat warnings as errors"));

            {
                static const char* runtimeNames[] = { "(Default)", "Use static runtime", "Use DLL runtime", nullptr };
                const var runtimeValues[] = { var(), var (false), var (true) };

                props.add (new ChoicePropertyComponent (getUsingRuntimeLibDLL(), "Runtime Library",
                                                        StringArray (runtimeNames), Array<var> (runtimeValues, numElementsInArray (runtimeValues))),
                           "If the static runtime is selected then your app/plug-in will not be dependent upon users having Microsoft's redistributable "
                           "C++ runtime installed. However, if you are linking libraries from different sources you must select the same type of runtime "
                           "used by the libraries.");
            }

            {
                static const char* wpoNames[] = { "Enable link-time code generation when possible",
                                                  "Always disable link-time code generation", nullptr };
                const var wpoValues[] = { var(), var (1) };

                props.add (new ChoicePropertyComponent (getWholeProgramOptValue(), "Whole Program Optimisation",
                                                        StringArray (wpoNames), Array<var> (wpoValues, numElementsInArray (wpoValues))));
            }

            {
                props.add (new BooleanPropertyComponent (shouldLinkIncrementalValue(), "Incremental Linking", "Enable"),
                           "Enable to avoid linking from scratch for every new build. "
                           "Disable to ensure that your final release build does not contain padding or thunks.");
            }

            if (! isDebug())
                props.add (new BooleanPropertyComponent (shouldGenerateDebugSymbolsValue(), "Debug Symbols", "Force generation of debug symbols"));

            props.add (new TextPropertyComponent (getPrebuildCommand(),  "Pre-build Command",  2048, true));
            props.add (new TextPropertyComponent (getPostbuildCommand(), "Post-build Command", 2048, true));
            props.add (new BooleanPropertyComponent (shouldGenerateManifestValue(), "Manifest", "Generate Manifest"));

            {
                static const char* characterSetNames[] = { "Default", "MultiByte", "Unicode", nullptr };
                const var charSets[]                   = { var(),     "MultiByte", "Unicode", };

                props.add (new ChoicePropertyComponent (getCharacterSetValue(), "Character Set",
                                                        StringArray (characterSetNames), Array<var> (charSets, numElementsInArray (charSets))));
            }
        }

        String getModuleLibraryArchName() const override
        {
            String result ("$(Platform)\\");
            result += isUsingRuntimeLibDLL() ? "MD" : "MT";

            if (isDebug())
                result += "d";

            return result;
        }
    };

    //==============================================================================
    class MSVCTargetBase : public ProjectType::Target
    {
    public:
        MSVCTargetBase (ProjectType::Target::Type targetType, const MSVCProjectExporterBase& exporter)
            : ProjectType::Target (targetType), owner (exporter)
        {
            projectGuid = createGUID (owner.getProject().getProjectUID() + getName());
        }

        virtual ~MSVCTargetBase() {}

        const MSVCProjectExporterBase& getOwner() const { return owner; }
        virtual String getTopLevelXmlEntity() const = 0;
        const String& getProjectGuid() const { return projectGuid; }

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

        virtual void fillInProjectXml (XmlElement& projectXml) const = 0;
        virtual void fillInFiltersXml (XmlElement& filtersXml) const = 0;

        String getSolutionTargetPath (const BuildConfiguration& config) const
        {
            const String binaryPath (config.getTargetBinaryRelativePathString().trim());
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
            String solutionTargetFolder (getSolutionTargetPath (config));
            return solutionTargetFolder + String ("\\") + getName();
        }

        String getIntermediatesPath (const MSVCBuildConfiguration& config) const
        {
            String intDir = (config.getIntermediatesPath().isNotEmpty() ? config.getIntermediatesPath() : "$(Platform)\\$(Configuration)");
            if (! intDir.endsWithChar (L'\\'))
                intDir += L'\\';

            return intDir + getName();
        }

        static const char* getOptimisationLevelString (int level)
        {
            switch (level)
            {
            case optimiseMaxSpeed:  return "Full";
            case optimiseMinSize:   return "MinSpace";
            default:                return "Disabled";
            }
        }

        String getTargetSuffix() const
        {
            const ProjectType::Target::TargetFileType fileType = getTargetFileType();

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
            XmlElement* const e = parent.createNewChildElement ("Tool");
            e->setAttribute ("Name", toolName);
            return e;
        }

        String getPreprocessorDefs (const BuildConfiguration& config, const String& joinString) const
        {
            StringPairArray defines (getOwner().msvcExtraPreprocessorDefs);
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
                String def (defines.getAllKeys()[i]);
                const String value (defines.getAllValues()[i]);
                if (value.isNotEmpty())
                    def << "=" << value;

                result.add (def);
            }

            return result.joinIntoString (joinString);
        }

        //==============================================================================
        RelativePath getAAXIconFile() const
        {
            const RelativePath aaxSDK (getOwner().getAAXPathValue().toString(), RelativePath::projectFolder);
            const RelativePath projectIcon ("icon.ico", RelativePath::buildTargetFolder);

            if (getOwner().getTargetFolder().getChildFile ("icon.ico").existsAsFile())
                return projectIcon.rebased (getOwner().getTargetFolder(),
                                            getOwner().getProject().getProjectFolder(),
                                            RelativePath::projectFolder);
            else
                return aaxSDK.getChildFile ("Utilities").getChildFile ("PlugIn.ico");
        }

        String getExtraPostBuildSteps (const MSVCBuildConfiguration& config) const
        {
            if (type == AAXPlugIn)
            {
                const RelativePath aaxSDK (getOwner().getAAXPathValue().toString(), RelativePath::projectFolder);
                const RelativePath aaxLibsFolder = aaxSDK.getChildFile ("Libs");
                const RelativePath bundleScript  = aaxSDK.getChildFile ("Utilities").getChildFile ("CreatePackage.bat");
                const RelativePath iconFilePath  = getAAXIconFile();

                const bool is64Bit = (config.config [Ids::winArchitecture] == "x64");
                const String bundleDir      = getOwner().getOutDirFile (config, config.getOutputFilename (".aaxplugin", true));
                const String bundleContents = bundleDir + "\\Contents";
                const String macOSDir       = bundleContents + String ("\\") + (is64Bit ? "x64" : "Win32");
                const String executable     = macOSDir + String ("\\") + config.getOutputFilename (".aaxplugin", true);

                return String ("copy /Y \"") + getOutputFilePath (config) + String ("\" \"") + executable + String ("\"\r\ncall ") +
                    createRebasedPath (bundleScript) + String (" \"") + macOSDir + String ("\" ") + createRebasedPath (iconFilePath);
            }

            return {};
        }

        String getExtraPreBuildSteps (const MSVCBuildConfiguration& config) const
        {
            if (type == AAXPlugIn)
            {
                String script;

                const bool is64Bit = (config.config [Ids::winArchitecture] == "x64");
                const String bundleDir      = getOwner().getOutDirFile (config, config.getOutputFilename (".aaxplugin", false));

                const String bundleContents = bundleDir + "\\Contents";
                const String macOSDir       = bundleContents + String ("\\") + (is64Bit ? "x64" : "Win32");

                StringArray folders = {bundleDir.toRawUTF8(), bundleContents.toRawUTF8(), macOSDir.toRawUTF8()};
                for (int i = 0; i < folders.size(); ++i)
                    script += String ("if not exist \"") + folders[i] + String ("\" mkdir \"") + folders[i] + String ("\"\r\n");

                return script;
            }

            return {};
        }

        String getPostBuildSteps (const MSVCBuildConfiguration& config) const
        {
            String postBuild            = config.getPostbuildCommandString();
            const String extraPostBuild = getExtraPostBuildSteps (config);

            postBuild += String (postBuild.isNotEmpty() && extraPostBuild.isNotEmpty() ? "\r\n" : "") + extraPostBuild;

            return postBuild;
        }

        String getPreBuildSteps (const MSVCBuildConfiguration& config) const
        {
            String preBuild            = config.getPrebuildCommandString();
            const String extraPreBuild = getExtraPreBuildSteps (config);

            preBuild += String (preBuild.isNotEmpty() && extraPreBuild.isNotEmpty() ? "\r\n" : "") + extraPreBuild;

            return preBuild;
        }

        void addExtraPreprocessorDefines (StringPairArray& defines) const
        {
            switch (type)
            {
            case AAXPlugIn:
                {
                    const RelativePath aaxLibsFolder = RelativePath (getOwner().getAAXPathValue().toString(), RelativePath::projectFolder).getChildFile ("Libs");
                    defines.set ("JucePlugin_AAXLibs_path", createRebasedPath (aaxLibsFolder));
                }
                break;
            case RTASPlugIn:
                {
                    const RelativePath rtasFolder (getOwner().getRTASPathValue().toString(), RelativePath::projectFolder);
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
                const RelativePath rtasFolder (getOwner().getRTASPathValue().toString(), RelativePath::projectFolder);
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
                    searchPaths.add (createRebasedPath (rtasFolder.getChildFile (p[i])));
            }

            return searchPaths;
        }

        String getBinaryNameWithSuffix (const MSVCBuildConfiguration& config) const
        {
            return config.getOutputFilename (getTargetSuffix(), true);
        }

        String getOutputFilePath (const MSVCBuildConfiguration& config) const
        {
            return getOwner().getOutDirFile (config, getBinaryNameWithSuffix (config));
        }

        StringArray getLibrarySearchPaths (const BuildConfiguration& config) const
        {
            StringArray librarySearchPaths (config.getLibrarySearchPaths());

            if (type != SharedCodeTarget)
                if (const MSVCTargetBase* shared = getOwner().getSharedCodeTarget())
                    librarySearchPaths.add (shared->getConfigTargetPath (config));

            return librarySearchPaths;
        }

        String getExternalLibraries (const MSVCBuildConfiguration& config, const String& otherLibs) const
        {
            StringArray libraries;

            if (otherLibs.isNotEmpty())
                libraries.add (otherLibs);

            StringArray moduleLibs = getOwner().getModuleLibs();
            if (! moduleLibs.isEmpty())
                libraries.addArray (moduleLibs);

            if (type != SharedCodeTarget)
                if (const MSVCTargetBase* shared = getOwner().getSharedCodeTarget())
                    libraries.add (shared->getBinaryNameWithSuffix (config));

            return libraries.joinIntoString (";");
        }

        String getDelayLoadedDLLs() const
        {
            String delayLoadedDLLs = getOwner().msvcDelayLoadedDLLs;

            if (type == RTASPlugIn)
                delayLoadedDLLs += "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; "
                    "DSPManager.dll; DSPManager.dll; DSPManagerClientLib.dll; RTASClientLib.dll";

            return delayLoadedDLLs;
        }

        String getModuleDefinitions (const MSVCBuildConfiguration& config) const
        {
            const String& moduleDefinitions = config.config [Ids::msvcModuleDefinitionFile].toString();

            if (moduleDefinitions.isNotEmpty())
                return moduleDefinitions;

            if (type == RTASPlugIn)
            {
                const ProjectExporter& exp = getOwner();

                RelativePath moduleDefPath
                    = RelativePath (exp.getPathForModuleString ("juce_audio_plugin_client"), RelativePath::projectFolder)
                         .getChildFile ("juce_audio_plugin_client").getChildFile ("RTAS").getChildFile ("juce_RTAS_WinExports.def");

                return prependDot (moduleDefPath.rebased (exp.getProject().getProjectFolder(),
                                                            exp.getTargetFolder(),
                                                            RelativePath::buildTargetFolder).toWindowsStyle());
            }

            return {};
        }

        bool shouldUseRuntimeDLL (const MSVCBuildConfiguration& config) const
        {
            return (config.config [Ids::useRuntimeLibDLL].isVoid() ? (getOwner().hasTarget (AAXPlugIn) || getOwner().hasTarget (RTASPlugIn))
                    : config.isUsingRuntimeLibDLL());
        }

        virtual String getProjectFileSuffix() const = 0;
        virtual String getFiltersFileSuffix() const = 0;

        File getVCProjFile() const            { return getOwner().getProjectFile (getProjectFileSuffix(), getName()); }
        File getVCProjFiltersFile() const     { return getOwner().getProjectFile (getFiltersFileSuffix(), getName()); }

        String createRebasedPath (const RelativePath& path) const    {  return getOwner().createRebasedPath (path); }

        //==============================================================================
        virtual String getProjectVersionString() const = 0;
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
        case ProjectType::Target::DynamicLibrary:
            return true;
        default:
            break;
        }

        return false;
    }

    //==============================================================================
    Value getManifestFile() { return getSetting (Ids::msvcManifestFile); }
    RelativePath getManifestPath() const
    {
        const String& path = settings [Ids::msvcManifestFile].toString();
        return path.isEmpty() ? RelativePath() : RelativePath (settings [Ids::msvcManifestFile], RelativePath::projectFolder);
    }

    //==============================================================================
    const String& getProjectName() const         { return projectName; }

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
        props.add(new TextPropertyComponent(getManifestFile(), "Manifest file", 8192, false),
            "Path to a manifest input file which should be linked into your binary (path is relative to jucer file).");
    }

    enum OptimisationLevel
    {
        optimisationOff = 1,
        optimiseMinSize = 2,
        optimiseMaxSpeed = 3
    };

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType& type) override
    {
        msvcExtraPreprocessorDefs.set ("_CRT_SECURE_NO_WARNINGS", "");

        if (type.isCommandLineApp())
            msvcExtraPreprocessorDefs.set("_CONSOLE", "");
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
        String rebasedPath = rebaseFromProjectFolderToBuildTarget (path).toWindowsStyle();

        return getVisualStudioVersion() < 10  // (VS10 automatically adds escape characters to the quotes for this definition)
                                          ? CppTokeniserFunctions::addEscapeChars (rebasedPath.quoted())
                                          : CppTokeniserFunctions::addEscapeChars (rebasedPath).quoted();
    }

protected:
    //==============================================================================
    mutable File rcFile, iconFile;
    OwnedArray<MSVCTargetBase> targets;

    File getProjectFile (const String& extension, const String& target) const
    {
        String filename = project.getProjectFilenameRoot();

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
            const String oldStylePrebuildCommand (getSettingString (Ids::prebuildCommand));
            settings.removeProperty (Ids::prebuildCommand, nullptr);

            if (oldStylePrebuildCommand.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    dynamic_cast<MSVCBuildConfiguration&> (*config).getPrebuildCommand() = oldStylePrebuildCommand;
        }

        {
            const String oldStyleLibName (getSettingString ("libraryName_Debug"));
            settings.removeProperty ("libraryName_Debug", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (config->isDebug())
                        config->getTargetBinaryName() = oldStyleLibName;
        }

        {
            const String oldStyleLibName (getSettingString ("libraryName_Release"));
            settings.removeProperty ("libraryName_Release", nullptr);

            if (oldStyleLibName.isNotEmpty())
                for (ConfigIterator config (*this); config.next();)
                    if (! config->isDebug())
                        config->getTargetBinaryName() = oldStyleLibName;
        }
    }

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new MSVCBuildConfiguration (project, v, *this);
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        return getCleanedStringArray (searchPaths);
    }

    String getSharedCodeGuid() const
    {
        String sharedCodeGuid;

        for (int i = 0; i < targets.size(); ++i)
            if (MSVCTargetBase* target = targets[i])
                if (target->type == ProjectType::Target::SharedCodeTarget)
                    return target->getProjectGuid();

        return {};
    }

    //==============================================================================
    void writeProjectDependencies (OutputStream& out) const
    {
        const String sharedCodeGuid = getSharedCodeGuid();

        for (int addingOtherTargets = 0; addingOtherTargets < (sharedCodeGuid.isNotEmpty() ? 2 : 1); ++addingOtherTargets)
        {
            for (int i = 0; i < targets.size(); ++i)
            {
                if (MSVCTargetBase* target = targets[i])
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
            const MSVCBuildConfiguration& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);
            const String configName = config.createMSVCConfigName();
            out << "\t\t" << configName << " = " << configName << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (auto& target : targets)
            for (ConstConfigIterator i (*this); i.next();)
            {
                const MSVCBuildConfiguration& config = dynamic_cast<const MSVCBuildConfiguration&> (*i);
                const String configName = config.createMSVCConfigName();

                for (auto& suffix : { "ActiveCfg", "Build.0", "Deploy.0" })
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
        const int maskStride = (w / 8 + 3) & ~3;

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

        const Image::BitmapData bitmap (image, Image::BitmapData::readOnly);
        const int alphaThreshold = 5;

        int y;
        for (y = h; --y >= 0;)
        {
            for (int x = 0; x < w; ++x)
            {
                const Colour pixel (bitmap.getPixelColour (x, y));

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
                const Colour pixel (bitmap.getPixelColour (x, y));

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

        const int imageDirEntrySize = 16;
        const int dataBlockStart = 6 + images.size() * imageDirEntrySize;

        for (int i = 0; i < images.size(); ++i)
        {
            const size_t oldDataSize = dataBlock.getDataSize();

            const Image& image = images.getReference (i);
            const int w = image.getWidth();
            const int h = image.getHeight();

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
            const int sizes[] = { 16, 32, 48, 256 };

            for (int i = 0; i < numElementsInArray (sizes); ++i)
            {
                Image im (getBestIconForSize (sizes[i], true));
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

        const String version (project.getVersionString());

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

        writeRCValue (mo, "CompanyName", project.getCompanyName().toString());
        writeRCValue (mo, "FileDescription", project.getTitle());
        writeRCValue (mo, "FileVersion", version);
        writeRCValue (mo, "ProductName", project.getTitle());
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
        StringArray versionParts;
        versionParts.addTokens (version, ",.", "");
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

    void initialiseDependencyPathValues()
    {
        vst3Path.referTo (Value (new DependencyPathValueSource (getSetting (Ids::vst3Folder),
                                                                Ids::vst3Path,
                                                                TargetOS::windows)));

        aaxPath.referTo (Value (new DependencyPathValueSource (getSetting (Ids::aaxFolder),
                                                               Ids::aaxPath,
                                                               TargetOS::windows)));

        rtasPath.referTo (Value (new DependencyPathValueSource (getSetting (Ids::rtasFolder),
                                                                Ids::rtasPath,
                                                                TargetOS::windows)));
    }

    static bool shouldUseStdCall (const RelativePath& path)
    {
        return path.getFileNameWithoutExtension().startsWithIgnoreCase ("juce_audio_plugin_client_RTAS_");
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
        : MSVCProjectExporterBase (p, t, "VisualStudio2013")
    {
        name = getName();
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

        static const char* toolsetNames[] = { "(default)", "v120", "v120_xp", "Windows7.1SDK", "CTP_Nov2013" };
        const var toolsets[]              = { var(),       "v120", "v120_xp", "Windows7.1SDK", "CTP_Nov2013" };
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
        : MSVCProjectExporterBase (p, t, "VisualStudio2015")
    {
        name = getName();
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

        static const char* toolsetNames[] = { "(default)", "v140", "v140_xp", "CTP_Nov2013" };
        const var toolsets[]              = { var(),       "v140", "v140_xp", "CTP_Nov2013" };
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
        : MSVCProjectExporterBase (p, t, "VisualStudio2017")
    {
        name = getName();
    }

    static const char* getName()                                     { return "Visual Studio 2017"; }
    static const char* getValueTreeTypeName()                        { return "VS2017"; }
    int getVisualStudioVersion() const override                      { return 15; }
    String getSolutionComment() const override                       { return "# Visual Studio 2017"; }
    String getToolsVersion() const override                          { return "15.0"; }
    String getDefaultToolset() const override                        { return "v141"; }
    String getDefaultWindowsTargetPlatformVersion() const override   { return "10.0.15063.0"; }

    static MSVCProjectExporterVC2017* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2017 (project, settings);

        return nullptr;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        MSVCProjectExporterBase::createExporterProperties (props);

        static const char* toolsetNames[] = { "(default)", "v140", "v140_xp", "v141", "v141_xp" };
        const var toolsets[]              = { var(),       "v140", "v140_xp", "v141", "v141_xp" };
        addToolsetProperty (props, toolsetNames, toolsets, numElementsInArray (toolsets));

        addIPPLibraryProperty (props);

        addWindowsTargetPlatformProperties (props);

        static const char* cppStandardNames[]  = { "(default)", "C++14",    "Latest C++ Standard"};
        const var standards[]                  = { var(),       "stdcpp14", "stdcpplatest" };
        addCppStandardProperty (props, cppStandardNames, standards, numElementsInArray (standards));
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2017)
};
