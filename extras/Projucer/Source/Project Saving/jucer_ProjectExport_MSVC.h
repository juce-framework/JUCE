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

class MSVCProjectExporterBase   : public ProjectExporter
{
public:
    MSVCProjectExporterBase (Project& p, const ValueTree& t, const char* const folderName)
        : ProjectExporter (p, t)
    {
        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + folderName;

        projectGUID = createGUID (project.getProjectUID());
        updateOldSettings();

        initialiseDependencyPathValues();
    }

    //==============================================================================
    bool usesMMFiles() const override            { return false; }
    bool canCopeWithDuplicateFiles() override    { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                { return false; }
    bool isVisualStudio() const override         { return true; }
    bool isCodeBlocks() const override           { return false; }
    bool isMakefile() const override             { return false; }
    bool isAndroidStudio() const override        { return false; }
    bool isAndroidAnt() const override           { return false; }

    bool isAndroid() const override              { return false; }
    bool isWindows() const override              { return true; }
    bool isLinux() const override                { return false; }
    bool isOSX() const override                  { return false; }
    bool isiOS() const override                  { return false; }

    bool supportsVST() const override            { return true;  }
    bool supportsVST3() const override           { return true;  }
    bool supportsAAX() const override            { return true;  }
    bool supportsRTAS() const override           { return true;  }
    bool supportsAU()   const override           { return false; }
    bool supportsAUv3() const override           { return false; }
    bool supportsStandalone() const override     { return false;  }

    //==============================================================================
    virtual int getVisualStudioVersion() const = 0;

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

    void createExporterProperties (PropertyListBuilder&) override
    {
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

        if (type.isGUIApplication())
        {
            msvcIsWindowsSubsystem = true;
            msvcTargetSuffix = ".exe";
        }
        else if (type.isCommandLineApp())
        {
            msvcIsWindowsSubsystem = false;
            msvcTargetSuffix = ".exe";
            msvcExtraPreprocessorDefs.set ("_CONSOLE", "");
        }
        else if (type.isStaticLibrary())
        {
            msvcTargetSuffix = ".lib";
            msvcExtraPreprocessorDefs.set ("_LIB", "");
        }
        else if (type.isDynamicLibrary())
        {
            msvcTargetSuffix = ".dll";
            msvcExtraPreprocessorDefs.set ("_LIB", "");
            msvcIsDLL = true;
        }
        else if (type.isAudioPlugin())
        {
            msvcTargetSuffix = ".dll";
            msvcIsDLL = true;

            if (project.shouldBuildVST().getValue())
                addVSTPluginSettings (false);

            if (project.shouldBuildVST3().getValue())
                addVSTPluginSettings (true);

            if (project.shouldBuildAAX().getValue())
                addAAXPluginSettings();

            if (project.shouldBuildRTAS().getValue())
                addRTASPluginSettings();
        }
    }

private:
    //==============================================================================
    String createRebasedPath (const RelativePath& path)
    {
        String rebasedPath = rebaseFromProjectFolderToBuildTarget (path).toWindowsStyle();

        return getVisualStudioVersion() < 10  // (VS10 automatically adds escape characters to the quotes for this definition)
                 ? CppTokeniserFunctions::addEscapeChars (rebasedPath.quoted())
                 : CppTokeniserFunctions::addEscapeChars (rebasedPath).quoted();
    }

    void addVSTPluginSettings (bool isVST3)
    {
        RelativePath modulePath (rebaseFromProjectFolderToBuildTarget (RelativePath (getPathForModuleString ("juce_audio_plugin_client"),
                                                                                              RelativePath::projectFolder)
                                                                                .getChildFile ("juce_audio_plugin_client")
                                                                                .getChildFile ("VST3")));

        for (ProjectExporter::ConfigIterator config (*this); config.next();)
        {
            if (config->getValue (Ids::useRuntimeLibDLL).getValue().isVoid())
                config->getValue (Ids::useRuntimeLibDLL) = true;

            if (isVST3)
            {
                if (config->getValue (Ids::postbuildCommand).toString().isEmpty())
                {
                    const String previousBuildCommands = config->getValue (Ids::internalPostBuildComamnd).toString();

                    String script;
                    if (previousBuildCommands.isNotEmpty())
                        script += "\r\n";

                    script += "copy /Y \"$(OutDir)$(TargetFileName)\" \"$(OutDir)$(TargetName).vst3\"";

                    config->getValue (Ids::internalPostBuildComamnd) = previousBuildCommands + script;
                }
            }
        }
    }

    void addAAXPluginSettings()
    {
        const RelativePath aaxLibsFolder = RelativePath (getAAXPathValue().toString(), RelativePath::projectFolder).getChildFile ("Libs");

        for (ProjectExporter::ConfigIterator config (*this); config.next();)
        {
            if (config->getValue (Ids::useRuntimeLibDLL).getValue().isVoid())
                config->getValue (Ids::useRuntimeLibDLL) = true;

            if (config->getValue(Ids::postbuildCommand).toString().isEmpty())
            {
                const String previousBuildCommands = config->getValue (Ids::internalPostBuildComamnd).toString();
                const String aaxSDKPath = File::isAbsolutePath(aaxPath.toString())
                                                          ? aaxPath.toString()
                                                          : String ("..\\..\\") + aaxPath.toString();

                const bool is64Bit = (config->getValue (Ids::winArchitecture) == "x64");
                const String bundleDir      = "$(OutDir)$(TargetName).aaxplugin";
                const String bundleContents = bundleDir + "\\Contents";
                const String macOSDir       = bundleContents + String ("\\") + (is64Bit ? "x64" : "Win32");
                const String executable     = macOSDir + String ("\\$(TargetName).aaxplugin");
                const String bundleScript   = aaxSDKPath + String ("\\Utilities\\CreatePackage.bat");
                String iconFilePath         = getTargetFolder().getChildFile ("icon.ico").getFullPathName();

                if (! File (iconFilePath).existsAsFile())
                    iconFilePath = aaxSDKPath + String ("\\Utilities\\PlugIn.ico");

                String script;

                if (previousBuildCommands.isNotEmpty())
                    script += "\r\n";

                StringArray folders;
                folders.add (bundleDir);
                folders.add (bundleContents);
                folders.add (macOSDir);

                for (int i = 0; i < folders.size(); ++i)
                    script += String ("if not exist \"") + folders[i] + String ("\" mkdir \"") + folders[i] + String ("\"\r\n");

                script += String ("copy /Y \"$(OutDir)$(TargetFileName)\" \"") + executable + String ("\"\r\n");
                script += String ("\"") + bundleScript + String ("\" \"") + macOSDir + String ("\" \"") + iconFilePath + String ("\"");

                config->getValue (Ids::internalPostBuildComamnd) = previousBuildCommands + script;
            }
        }

        msvcExtraPreprocessorDefs.set ("JucePlugin_AAXLibs_path",
                                       createRebasedPath (aaxLibsFolder));
    }

    void addRTASPluginSettings()
    {
       RelativePath rtasFolder (getRTASPathValue().toString(), RelativePath::projectFolder);

        msvcTargetSuffix = ".dpm";

        msvcExtraPreprocessorDefs.set ("JucePlugin_WinBag_path",
                                                createRebasedPath (rtasFolder.getChildFile ("WinBag")));

        msvcDelayLoadedDLLs = "DAE.dll; DigiExt.dll; DSI.dll; PluginLib.dll; "
        "DSPManager.dll; DSPManager.dll; DSPManagerClientLib.dll; RTASClientLib.dll";

        if (! getExtraLinkerFlagsString().contains ("/FORCE:multiple"))
            getExtraLinkerFlags() = getExtraLinkerFlags().toString() + " /FORCE:multiple";

        RelativePath modulePath (rebaseFromProjectFolderToBuildTarget (RelativePath (getPathForModuleString ("juce_audio_plugin_client"),
                                                                                              RelativePath::projectFolder)
                                                                                .getChildFile ("juce_audio_plugin_client")
                                                                                .getChildFile ("RTAS")));

        for (ProjectExporter::ConfigIterator config (*this); config.next();)
        {
            config->getValue (Ids::msvcModuleDefinitionFile) = modulePath.getChildFile ("juce_RTAS_WinExports.def").toWindowsStyle();

            if (config->getValue (Ids::useRuntimeLibDLL).getValue().isVoid())
                config->getValue (Ids::useRuntimeLibDLL) = true;

            if (config->getValue (Ids::postbuildCommand).toString().isEmpty())
            {
                const String previousBuildCommands = config->getValue (Ids::internalPostBuildComamnd).toString();

                String script;
                if (previousBuildCommands.isNotEmpty())
                    script += "\r\n";

                script += "copy /Y "
                       + modulePath.getChildFile("juce_RTAS_WinResources.rsr").toWindowsStyle().quoted()
                       + " \"$(TargetPath)\".rsr";

                config->getValue (Ids::internalPostBuildComamnd) = previousBuildCommands + script;
            }
        }

        RelativePath juceWrapperFolder (project.getGeneratedCodeFolder(),
                                        getTargetFolder(), RelativePath::buildTargetFolder);

        extraSearchPaths.add (juceWrapperFolder.toWindowsStyle());

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
            addToExtraSearchPaths (rtasFolder.getChildFile (p[i]));
    }

protected:
    //==============================================================================
    String projectGUID;
    mutable File rcFile, iconFile;

    File getProjectFile (const String& extension) const   { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (extension); }
    File getSLNFile() const     { return getProjectFile (".sln"); }

    bool isLibraryDLL() const   { return msvcIsDLL || projectType.isDynamicLibrary(); }

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

        Value getInternalPostbuildCommands()        { return getValue (Ids::internalPostBuildComamnd); }

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
                                                        StringArray (runtimeNames), Array<var> (runtimeValues, numElementsInArray (runtimeValues))));
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
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new MSVCBuildConfiguration (project, v, *this);
    }

    //==============================================================================
    String getConfigTargetPath (const BuildConfiguration& config) const
    {
        const String binaryPath (config.getTargetBinaryRelativePathString().trim());
        if (binaryPath.isEmpty())
            return binaryPath;

        RelativePath binaryRelPath (binaryPath, RelativePath::projectFolder);

        if (binaryRelPath.isAbsolute())
            return binaryRelPath.toWindowsStyle();

        return prependDot (binaryRelPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder)
                                        .toWindowsStyle());
    }

    String getPreprocessorDefs (const BuildConfiguration& config, const String& joinString) const
    {
        StringPairArray defines (msvcExtraPreprocessorDefs);
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

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

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

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        return getCleanedStringArray (searchPaths);
    }

    virtual String createConfigName (const BuildConfiguration& config) const
    {
        return config.getName() + "|Win32";
    }

    //==============================================================================
    void writeSolutionFile (OutputStream& out, const String& versionString, String commentString, const File& vcProject) const
    {
        if (commentString.isNotEmpty())
            commentString += newLine;

        out << "Microsoft Visual Studio Solution File, Format Version " << versionString << newLine
            << commentString
            << "Project(\"" << createGUID (projectName + "sln_guid") << "\") = \"" << projectName << "\", \""
            << vcProject.getFileName() << "\", \"" << projectGUID << '"' << newLine
            << "EndProject" << newLine
            << "Global" << newLine
            << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" << newLine;

        for (ConstConfigIterator i (*this); i.next();)
        {
            const String configName (createConfigName (*i));
            out << "\t\t" << configName << " = " << configName << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" << newLine;

        for (ConstConfigIterator i (*this); i.next();)
        {
            const String configName (createConfigName (*i));
            out << "\t\t" << projectGUID << "." << configName << ".ActiveCfg = " << configName << newLine;
            out << "\t\t" << projectGUID << "." << configName << ".Build.0 = " << configName << newLine;
        }

        out << "\tEndGlobalSection" << newLine
            << "\tGlobalSection(SolutionProperties) = preSolution" << newLine
            << "\t\tHideSolutionNode = FALSE" << newLine
            << "\tEndGlobalSection" << newLine
            << "EndGlobal" << newLine;
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

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterBase)
};


//==============================================================================
class MSVCProjectExporterVC2008   : public MSVCProjectExporterBase
{
public:
    //==============================================================================
    MSVCProjectExporterVC2008 (Project& p, const ValueTree& s,
                               const char* folderName = "VisualStudio2008")
        : MSVCProjectExporterBase (p, s, folderName)
    {
        name = getName();
    }

    static const char* getName()                    { return "Visual Studio 2008"; }
    static const char* getValueTreeTypeName()       { return "VS2008"; }
    int getVisualStudioVersion() const override     { return 9; }

    static MSVCProjectExporterVC2008* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2008 (project, settings);

        return nullptr;
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        createResourcesAndIcon();

        if (hasResourceFile())
        {
            for (int i = 0; i < getAllGroups().size(); ++i)
            {
                Project::Item& group = getAllGroups().getReference(i);

                if (group.getID() == ProjectSaver::getGeneratedGroupID())
                {
                    if (iconFile != File())
                    {
                        group.addFileAtIndex (iconFile, -1, true);
                        group.findItemForFile (iconFile).getShouldAddToBinaryResourcesValue() = false;
                    }

                    group.addFileAtIndex (rcFile, -1, true);
                    group.findItemForFile (rcFile).getShouldAddToBinaryResourcesValue() = false;

                    break;
                }
            }
        }

        {
            XmlElement projectXml ("VisualStudioProject");
            fillInProjectXml (projectXml);
            writeXmlOrThrow (projectXml, getVCProjFile(), "UTF-8", 10);
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, getSolutionVersionString(), String(), getVCProjFile());

            overwriteFileIfDifferentOrThrow (getSLNFile(), mo);
        }
    }

protected:
    virtual String getProjectVersionString() const    { return "9.00"; }
    virtual String getSolutionVersionString() const   { return String ("10.00") + newLine + "# Visual C++ Express 2008"; }

    File getVCProjFile() const    { return getProjectFile (".vcproj"); }

    //==============================================================================
    void fillInProjectXml (XmlElement& projectXml) const
    {
        projectXml.setAttribute ("ProjectType", "Visual C++");
        projectXml.setAttribute ("Version", getProjectVersionString());
        projectXml.setAttribute ("Name", projectName);
        projectXml.setAttribute ("ProjectGUID", projectGUID);
        projectXml.setAttribute ("TargetFrameworkVersion", "131072");

        {
            XmlElement* platforms = projectXml.createNewChildElement ("Platforms");
            XmlElement* platform = platforms->createNewChildElement ("Platform");
            platform->setAttribute ("Name", "Win32");
        }

        projectXml.createNewChildElement ("ToolFiles");
        createConfigs (*projectXml.createNewChildElement ("Configurations"));
        projectXml.createNewChildElement ("References");
        createFiles (*projectXml.createNewChildElement ("Files"));
        projectXml.createNewChildElement ("Globals");
    }

    //==============================================================================
    void addFile (const RelativePath& file, XmlElement& parent, const bool excludeFromBuild, const bool useStdcall) const
    {
        jassert (file.getRoot() == RelativePath::buildTargetFolder);

        XmlElement* fileXml = parent.createNewChildElement ("File");
        fileXml->setAttribute ("RelativePath", file.toWindowsStyle());

        if (excludeFromBuild || useStdcall)
        {
            for (ConstConfigIterator i (*this); i.next();)
            {
                XmlElement* fileConfig = fileXml->createNewChildElement ("FileConfiguration");
                fileConfig->setAttribute ("Name", createConfigName (*i));

                if (excludeFromBuild)
                    fileConfig->setAttribute ("ExcludedFromBuild", "true");

                XmlElement* tool = createToolElement (*fileConfig, "VCCLCompilerTool");

                if (useStdcall)
                    tool->setAttribute ("CallingConvention", "2");
            }
        }
    }

    XmlElement* createGroup (const String& groupName, XmlElement& parent) const
    {
        XmlElement* filter = parent.createNewChildElement ("Filter");
        filter->setAttribute ("Name", groupName);
        return filter;
    }

    void addFiles (const Project::Item& projectItem, XmlElement& parent) const
    {
        if (projectItem.isGroup())
        {
            XmlElement* filter = createGroup (projectItem.getName(), parent);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFiles (projectItem.getChild(i), *filter);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            addFile (path, parent,
                     projectItem.shouldBeAddedToBinaryResources()
                       || (shouldFileBeCompiledByDefault (path) && ! projectItem.shouldBeCompiled()),
                     shouldFileBeCompiledByDefault (path) && shouldUseStdCall (path));
        }
    }

    void createFiles (XmlElement& files) const
    {
        for (int i = 0; i < getAllGroups().size(); ++i)
        {
            const Project::Item& group = getAllGroups().getReference(i);

            if (group.getNumChildren() > 0)
                addFiles (group, files);
        }
    }

    //==============================================================================
    XmlElement* createToolElement (XmlElement& parent, const String& toolName) const
    {
        XmlElement* const e = parent.createNewChildElement ("Tool");
        e->setAttribute ("Name", toolName);
        return e;
    }

    void createConfig (XmlElement& xml, const MSVCBuildConfiguration& config) const
    {
        const bool isDebug = config.isDebug();

        xml.setAttribute ("Name", createConfigName (config));

        if (getConfigTargetPath (config).isNotEmpty())
            xml.setAttribute ("OutputDirectory", FileHelpers::windowsStylePath (getConfigTargetPath (config)));

        if (config.getIntermediatesPath().isNotEmpty())
            xml.setAttribute ("IntermediateDirectory", FileHelpers::windowsStylePath (config.getIntermediatesPath()));

        xml.setAttribute ("ConfigurationType", isLibraryDLL() ? "2" : (projectType.isStaticLibrary() ? "4" : "1"));
        xml.setAttribute ("UseOfMFC", "0");
        xml.setAttribute ("ATLMinimizesCRunTimeLibraryUsage", "false");
        xml.setAttribute ("CharacterSet", "2");

        if (! (isDebug || config.shouldDisableWholeProgramOpt()))
            xml.setAttribute ("WholeProgramOptimization", "1");

        XmlElement* preBuildEvent = createToolElement (xml, "VCPreBuildEventTool");

        if (config.getPrebuildCommandString().isNotEmpty())
        {
            preBuildEvent->setAttribute ("Description", "Pre-build");
            preBuildEvent->setAttribute ("CommandLine", config.getPrebuildCommandString());
        }

        createToolElement (xml, "VCCustomBuildTool");
        createToolElement (xml, "VCXMLDataGeneratorTool");
        createToolElement (xml, "VCWebServiceProxyGeneratorTool");

        if (! projectType.isStaticLibrary())
        {
            XmlElement* midl = createToolElement (xml, "VCMIDLTool");
            midl->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
            midl->setAttribute ("MkTypLibCompatible", "true");
            midl->setAttribute ("SuppressStartupBanner", "true");
            midl->setAttribute ("TargetEnvironment", "1");
            midl->setAttribute ("TypeLibraryName", getIntDirFile (config, config.getOutputFilename (".tlb", true)));
            midl->setAttribute ("HeaderFileName", "");
        }

        {
            XmlElement* compiler = createToolElement (xml, "VCCLCompilerTool");

            compiler->setAttribute ("Optimization", getOptimisationLevelString (config.getOptimisationLevelInt()));

            if (isDebug)
            {
                compiler->setAttribute ("BufferSecurityCheck", "");
                compiler->setAttribute ("DebugInformationFormat", projectType.isStaticLibrary() ? "3" : "4");
            }
            else
            {
                compiler->setAttribute ("InlineFunctionExpansion", "1");
                compiler->setAttribute ("StringPooling", "true");
            }

            compiler->setAttribute ("AdditionalIncludeDirectories", replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (";")));
            compiler->setAttribute ("PreprocessorDefinitions", getPreprocessorDefs (config, ";"));
            compiler->setAttribute ("RuntimeLibrary", config.isUsingRuntimeLibDLL() ? (isDebug ? 3 : 2) // MT DLL
                                                                                    : (isDebug ? 1 : 0)); // MT static
            compiler->setAttribute ("RuntimeTypeInfo", "true");
            compiler->setAttribute ("UsePrecompiledHeader", "0");
            compiler->setAttribute ("PrecompiledHeaderFile", getIntDirFile (config, config.getOutputFilename (".pch", true)));
            compiler->setAttribute ("AssemblerListingLocation", "$(IntDir)\\");
            compiler->setAttribute ("ObjectFile", "$(IntDir)\\");
            compiler->setAttribute ("ProgramDataBaseFileName", "$(IntDir)\\");
            compiler->setAttribute ("WarningLevel", String (config.getWarningLevel()));
            compiler->setAttribute ("SuppressStartupBanner", "true");

            const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim());
            if (extraFlags.isNotEmpty())
                compiler->setAttribute ("AdditionalOptions", extraFlags);
        }

        createToolElement (xml, "VCManagedResourceCompilerTool");

        {
            XmlElement* resCompiler = createToolElement (xml, "VCResourceCompilerTool");
            resCompiler->setAttribute ("PreprocessorDefinitions", isDebug ? "_DEBUG" : "NDEBUG");
        }

        createToolElement (xml, "VCPreLinkEventTool");

        if (! projectType.isStaticLibrary())
        {
            XmlElement* linker = createToolElement (xml, "VCLinkerTool");

            linker->setAttribute ("OutputFile", getOutDirFile (config, config.getOutputFilename (msvcTargetSuffix, false)));
            linker->setAttribute ("SuppressStartupBanner", "true");

            linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            linker->setAttribute ("GenerateDebugInformation", (isDebug || config.shouldGenerateDebugSymbols()) ? "true" : "false");
            linker->setAttribute ("LinkIncremental", config.shouldLinkIncremental() ? "2" : "1");
            linker->setAttribute ("ProgramDatabaseFile", getIntDirFile (config, config.getOutputFilename (".pdb", true)));
            linker->setAttribute ("SubSystem", msvcIsWindowsSubsystem ? "2" : "1");

            const StringArray librarySearchPaths (config.getLibrarySearchPaths());
            if (librarySearchPaths.size() > 0)
                linker->setAttribute ("AdditionalLibraryDirectories", librarySearchPaths.joinIntoString (";"));

            linker->setAttribute ("GenerateManifest", config.shouldGenerateManifest() ? "true" : "false");

            if (! isDebug)
            {
                linker->setAttribute ("OptimizeReferences", "2");
                linker->setAttribute ("EnableCOMDATFolding", "2");
            }

            linker->setAttribute ("TargetMachine", "1"); // (64-bit build = 5)

            if (msvcDelayLoadedDLLs.isNotEmpty())
                linker->setAttribute ("DelayLoadDLLs", msvcDelayLoadedDLLs);

            if (config.config [Ids::msvcModuleDefinitionFile].toString().isNotEmpty())
                linker->setAttribute ("ModuleDefinitionFile", config.config [Ids::msvcModuleDefinitionFile].toString());

            String externalLibraries (getExternalLibrariesString());
            if (externalLibraries.isNotEmpty())
                linker->setAttribute ("AdditionalDependencies", replacePreprocessorTokens (config, externalLibraries).trim());

            String extraLinkerOptions (getExtraLinkerFlagsString());
            if (extraLinkerOptions.isNotEmpty())
                linker->setAttribute ("AdditionalOptions", replacePreprocessorTokens (config, extraLinkerOptions).trim());
        }
        else
        {
            if (isLibraryDLL())
            {
                XmlElement* linker = createToolElement (xml, "VCLinkerTool");

                String extraLinkerOptions (getExtraLinkerFlagsString());
                extraLinkerOptions << " /IMPLIB:" << getOutDirFile (config, config.getOutputFilename (".lib", true));
                linker->setAttribute ("AdditionalOptions", replacePreprocessorTokens (config, extraLinkerOptions).trim());

                String externalLibraries (getExternalLibrariesString());
                if (externalLibraries.isNotEmpty())
                    linker->setAttribute ("AdditionalDependencies", replacePreprocessorTokens (config, externalLibraries).trim());

                linker->setAttribute ("OutputFile", getOutDirFile (config, config.getOutputFilename (msvcTargetSuffix, false)));
                linker->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");

                linker->setAttribute ("LinkIncremental", config.shouldLinkIncremental() ? "2" : "1");
            }
            else
            {
                XmlElement* librarian = createToolElement (xml, "VCLibrarianTool");

                librarian->setAttribute ("OutputFile", getOutDirFile (config, config.getOutputFilename (msvcTargetSuffix, false)));
                librarian->setAttribute ("IgnoreDefaultLibraryNames", isDebug ? "libcmt.lib, msvcrt.lib" : "");
            }
        }

        createToolElement (xml, "VCALinkTool");
        createToolElement (xml, "VCManifestTool");
        createToolElement (xml, "VCXDCMakeTool");

        {
            XmlElement* bscMake = createToolElement (xml, "VCBscMakeTool");
            bscMake->setAttribute ("SuppressStartupBanner", "true");
            bscMake->setAttribute ("OutputFile", getIntDirFile (config, config.getOutputFilename (".bsc", true)));
        }

        createToolElement (xml, "VCFxCopTool");

        if (! projectType.isStaticLibrary())
            createToolElement (xml, "VCAppVerifierTool");

        XmlElement* postBuildEvent = createToolElement (xml, "VCPostBuildEventTool");

        if (config.getPostbuildCommandString().isNotEmpty())
        {
            postBuildEvent->setAttribute ("Description", "Post-build");
            postBuildEvent->setAttribute ("CommandLine", config.getPostbuildCommandString());
        }
    }

    void createConfigs (XmlElement& xml) const
    {
        for (ConstConfigIterator config (*this); config.next();)
            createConfig (*xml.createNewChildElement ("Configuration"),
                          dynamic_cast<const MSVCBuildConfiguration&> (*config));
    }

    static const char* getOptimisationLevelString (int level)
    {
        switch (level)
        {
            case optimiseMaxSpeed:  return "3";
            case optimiseMinSize:   return "1";
            default:                return "0";
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2008)
};


//==============================================================================
class MSVCProjectExporterVC2005   : public MSVCProjectExporterVC2008
{
public:
    MSVCProjectExporterVC2005 (Project& p, const ValueTree& t)
        : MSVCProjectExporterVC2008 (p, t, "VisualStudio2005")
    {
        name = getName();
    }

    static const char* getName()                    { return "Visual Studio 2005"; }
    static const char* getValueTreeTypeName()       { return "VS2005"; }
    int getVisualStudioVersion() const override     { return 8; }

    static MSVCProjectExporterVC2005* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2005 (project, settings);

        return nullptr;
    }

protected:
    String getProjectVersionString() const override    { return "8.00"; }
    String getSolutionVersionString() const override   { return String ("9.00") + newLine + "# Visual C++ Express 2005"; }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2005)
};

//==============================================================================
class MSVCProjectExporterVC2010   : public MSVCProjectExporterBase
{
public:
    MSVCProjectExporterVC2010 (Project& p, const ValueTree& t, const char* folderName = "VisualStudio2010")
        : MSVCProjectExporterBase (p, t, folderName)
    {
        name = getName();
    }

    static const char* getName()                { return "Visual Studio 2010"; }
    static const char* getValueTreeTypeName()   { return "VS2010"; }
    int getVisualStudioVersion() const override { return 10; }
    virtual String getSolutionComment() const   { return "# Visual Studio 2010"; }
    virtual String getToolsVersion() const      { return "4.0"; }
    virtual String getDefaultToolset() const    { return "Windows7.1SDK"; }
    Value getPlatformToolsetValue()             { return getSetting (Ids::toolset); }
    Value getIPPLibraryValue()                  { return getSetting (Ids::IPPLibrary); }
    String getIPPLibrary() const                { return settings [Ids::IPPLibrary]; }

    String getPlatformToolset() const
    {
        const String s (settings [Ids::toolset].toString());
        return s.isNotEmpty() ? s : getDefaultToolset();
    }

    static MSVCProjectExporterVC2010* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2010 (project, settings);

        return nullptr;
    }

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

    void createExporterProperties (PropertyListBuilder& props) override
    {
        MSVCProjectExporterBase::createExporterProperties (props);

        static const char* toolsetNames[] = { "(default)", "v100", "v100_xp", "Windows7.1SDK", "CTP_Nov2013" };
        const var toolsets[]              = { var(),       "v100", "v100_xp", "Windows7.1SDK", "CTP_Nov2013" };

        addToolsetProperty (props, toolsetNames, toolsets, numElementsInArray (toolsets));
        addIPPLibraryProperty (props);
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        createResourcesAndIcon();

        {
            XmlElement projectXml ("Project");
            fillInProjectXml (projectXml);
            addPlatformToolsetToPropertyGroup (projectXml);
            addIPPSettingToPropertyGroup (projectXml);

            writeXmlOrThrow (projectXml, getVCProjFile(), "utf-8", 100);
        }

        {
            XmlElement filtersXml ("Project");
            fillInFiltersXml (filtersXml);
            writeXmlOrThrow (filtersXml, getVCProjFiltersFile(), "utf-8", 100);
        }

        {
            MemoryOutputStream mo;
            writeSolutionFile (mo, "11.00", getSolutionComment(), getVCProjFile());

            overwriteFileIfDifferentOrThrow (getSLNFile(), mo);
        }
    }

protected:
    //==============================================================================
    class VC2010BuildConfiguration  : public MSVCBuildConfiguration
    {
    public:
        VC2010BuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : MSVCBuildConfiguration (p, settings, e)
        {
            if (getArchitectureType().toString().isEmpty())
                getArchitectureType() = get32BitArchName();
        }

        //==============================================================================
        static const char* get32BitArchName()   { return "32-bit"; }
        static const char* get64BitArchName()   { return "x64"; }

        Value getArchitectureType()             { return getValue (Ids::winArchitecture); }
        bool is64Bit() const                    { return config [Ids::winArchitecture].toString() == get64BitArchName(); }

        Value getFastMathValue()                { return getValue (Ids::fastMath); }
        bool isFastMathEnabled() const          { return config [Ids::fastMath]; }

        //==============================================================================
        void createConfigProperties (PropertyListBuilder& props) override
        {
            MSVCBuildConfiguration::createConfigProperties (props);

            const char* const archTypes[] = { get32BitArchName(), get64BitArchName() };

            props.add (new ChoicePropertyComponent (getArchitectureType(), "Architecture",
                                                    StringArray (archTypes, numElementsInArray (archTypes)),
                                                    Array<var> (archTypes, numElementsInArray (archTypes))));

            props.add (new BooleanPropertyComponent (getFastMathValue(), "Relax IEEE compliance", "Enabled"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");
        }
    };

    virtual void addPlatformToolsetToPropertyGroup (XmlElement&) const {}

    void addIPPSettingToPropertyGroup (XmlElement& p) const
    {
        String ippLibrary = getIPPLibrary();

        if (ippLibrary.isNotEmpty())
            forEachXmlChildElementWithTagName (p, e, "PropertyGroup")
                e->createNewChildElement ("UseIntelIPP")->addTextElement (ippLibrary);
    }

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new VC2010BuildConfiguration (project, v, *this);
    }

    static bool is64Bit (const BuildConfiguration& config)
    {
        return dynamic_cast<const VC2010BuildConfiguration&> (config).is64Bit();
    }

    //==============================================================================
    File getVCProjFile() const            { return getProjectFile (".vcxproj"); }
    File getVCProjFiltersFile() const     { return getProjectFile (".vcxproj.filters"); }

    String createConfigName (const BuildConfiguration& config) const override
    {
        return config.getName() + (is64Bit (config) ? "|x64"
                                                    : "|Win32");
    }

    void setConditionAttribute (XmlElement& xml, const BuildConfiguration& config) const
    {
        xml.setAttribute ("Condition", "'$(Configuration)|$(Platform)'=='" + createConfigName (config) + "'");
    }

    //==============================================================================
    void fillInProjectXml (XmlElement& projectXml) const
    {
        projectXml.setAttribute ("DefaultTargets", "Build");
        projectXml.setAttribute ("ToolsVersion", getToolsVersion());
        projectXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

        {
            XmlElement* configsGroup = projectXml.createNewChildElement ("ItemGroup");
            configsGroup->setAttribute ("Label", "ProjectConfigurations");

            for (ConstConfigIterator config (*this); config.next();)
            {
                XmlElement* e = configsGroup->createNewChildElement ("ProjectConfiguration");
                e->setAttribute ("Include", createConfigName (*config));
                e->createNewChildElement ("Configuration")->addTextElement (config->getName());
                e->createNewChildElement ("Platform")->addTextElement (is64Bit (*config) ? "x64" : "Win32");
            }
        }

        {
            XmlElement* globals = projectXml.createNewChildElement ("PropertyGroup");
            globals->setAttribute ("Label", "Globals");
            globals->createNewChildElement ("ProjectGuid")->addTextElement (projectGUID);
        }

        {
            XmlElement* imports = projectXml.createNewChildElement ("Import");
            imports->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.Default.props");
        }

        for (ConstConfigIterator i (*this); i.next();)
        {
            const VC2010BuildConfiguration& config = dynamic_cast<const VC2010BuildConfiguration&> (*i);

            XmlElement* e = projectXml.createNewChildElement ("PropertyGroup");
            setConditionAttribute (*e, config);
            e->setAttribute ("Label", "Configuration");
            e->createNewChildElement ("ConfigurationType")->addTextElement (getProjectType());
            e->createNewChildElement ("UseOfMfc")->addTextElement ("false");

            const String charSet (config.getCharacterSet());

            if (charSet.isNotEmpty())
                e->createNewChildElement ("CharacterSet")->addTextElement (charSet);

            if (! (config.isDebug() || config.shouldDisableWholeProgramOpt()))
                e->createNewChildElement ("WholeProgramOptimization")->addTextElement ("true");

            if (config.shouldLinkIncremental())
                e->createNewChildElement ("LinkIncremental")->addTextElement ("true");

            if (config.is64Bit())
                e->createNewChildElement ("PlatformToolset")->addTextElement (getPlatformToolset());
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("Import");
            e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.props");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("ImportGroup");
            e->setAttribute ("Label", "ExtensionSettings");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("ImportGroup");
            e->setAttribute ("Label", "PropertySheets");
            XmlElement* p = e->createNewChildElement ("Import");
            p->setAttribute ("Project", "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props");
            p->setAttribute ("Condition", "exists('$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props')");
            p->setAttribute ("Label", "LocalAppDataPlatform");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("PropertyGroup");
            e->setAttribute ("Label", "UserMacros");
        }

        {
            XmlElement* props = projectXml.createNewChildElement ("PropertyGroup");
            props->createNewChildElement ("_ProjectFileVersion")->addTextElement ("10.0.30319.1");

            for (ConstConfigIterator i (*this); i.next();)
            {
                const VC2010BuildConfiguration& config = dynamic_cast<const VC2010BuildConfiguration&> (*i);

                if (getConfigTargetPath (config).isNotEmpty())
                {
                    XmlElement* outdir = props->createNewChildElement ("OutDir");
                    setConditionAttribute (*outdir, config);
                    outdir->addTextElement (FileHelpers::windowsStylePath (getConfigTargetPath (config)) + "\\");
                }

                if (config.getIntermediatesPath().isNotEmpty())
                {
                    XmlElement* intdir = props->createNewChildElement ("IntDir");
                    setConditionAttribute (*intdir, config);

                    String intermediatesPath = config.getIntermediatesPath();
                    if (! intermediatesPath.endsWith ("\\"))
                        intermediatesPath << "\\";

                    intdir->addTextElement (FileHelpers::windowsStylePath (intermediatesPath));
                }

                {
                    XmlElement* targetName = props->createNewChildElement ("TargetName");
                    setConditionAttribute (*targetName, config);
                    targetName->addTextElement (config.getOutputFilename (String(), true));
                }

                {
                    XmlElement* manifest = props->createNewChildElement ("GenerateManifest");
                    setConditionAttribute (*manifest, config);
                    manifest->addTextElement (config.shouldGenerateManifest() ? "true" : "false");
                }

                const StringArray librarySearchPaths (config.getLibrarySearchPaths());

                if (librarySearchPaths.size() > 0)
                {
                    XmlElement* libPath = props->createNewChildElement ("LibraryPath");
                    setConditionAttribute (*libPath, config);
                    libPath->addTextElement ("$(LibraryPath);" + librarySearchPaths.joinIntoString (";"));
                }
            }
        }

        for (ConstConfigIterator i (*this); i.next();)
        {
            const VC2010BuildConfiguration& config = dynamic_cast<const VC2010BuildConfiguration&> (*i);

            const bool isDebug = config.isDebug();

            XmlElement* group = projectXml.createNewChildElement ("ItemDefinitionGroup");
            setConditionAttribute (*group, config);

            {
                XmlElement* midl = group->createNewChildElement ("Midl");
                midl->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                 : "NDEBUG;%(PreprocessorDefinitions)");
                midl->createNewChildElement ("MkTypLibCompatible")->addTextElement ("true");
                midl->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                midl->createNewChildElement ("TargetEnvironment")->addTextElement ("Win32");
                midl->createNewChildElement ("HeaderFileName");
            }

            bool isUsingEditAndContinue = false;

            {
                XmlElement* cl = group->createNewChildElement ("ClCompile");

                cl->createNewChildElement ("Optimization")->addTextElement (getOptimisationLevelString (config.getOptimisationLevelInt()));

                if (isDebug && config.getOptimisationLevelInt() <= optimisationOff)
                {
                    isUsingEditAndContinue = ! config.is64Bit();

                    cl->createNewChildElement ("DebugInformationFormat")
                            ->addTextElement (isUsingEditAndContinue ? "EditAndContinue"
                                                                     : "ProgramDatabase");
                }

                StringArray includePaths (getHeaderSearchPaths (config));
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
                cl->createNewChildElement ("MultiProcessorCompilation")->addTextElement ("true");

                if (config.isFastMathEnabled())
                    cl->createNewChildElement ("FloatingPointModel")->addTextElement ("Fast");

                const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim());
                if (extraFlags.isNotEmpty())
                    cl->createNewChildElement ("AdditionalOptions")->addTextElement (extraFlags + " %(AdditionalOptions)");

                if (config.areWarningsTreatedAsErrors())
                    cl->createNewChildElement ("TreatWarningAsError")->addTextElement ("true");
            }

            {
                XmlElement* res = group->createNewChildElement ("ResourceCompile");
                res->createNewChildElement ("PreprocessorDefinitions")->addTextElement (isDebug ? "_DEBUG;%(PreprocessorDefinitions)"
                                                                                                : "NDEBUG;%(PreprocessorDefinitions)");
            }

            {
                XmlElement* link = group->createNewChildElement ("Link");
                link->createNewChildElement ("OutputFile")->addTextElement (getOutDirFile (config, config.getOutputFilename (msvcTargetSuffix, false)));
                link->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                link->createNewChildElement ("IgnoreSpecificDefaultLibraries")->addTextElement (isDebug ? "libcmt.lib; msvcrt.lib;;%(IgnoreSpecificDefaultLibraries)"
                                                                                                        : "%(IgnoreSpecificDefaultLibraries)");
                link->createNewChildElement ("GenerateDebugInformation")->addTextElement ((isDebug || config.shouldGenerateDebugSymbols()) ? "true" : "false");
                link->createNewChildElement ("ProgramDatabaseFile")->addTextElement (getIntDirFile (config, config.getOutputFilename (".pdb", true)));
                link->createNewChildElement ("SubSystem")->addTextElement (msvcIsWindowsSubsystem ? "Windows" : "Console");

                if (! config.is64Bit())
                    link->createNewChildElement ("TargetMachine")->addTextElement ("MachineX86");

                if (isUsingEditAndContinue)
                    link->createNewChildElement ("ImageHasSafeExceptionHandlers")->addTextElement ("false");

                if (! isDebug)
                {
                    link->createNewChildElement ("OptimizeReferences")->addTextElement ("true");
                    link->createNewChildElement ("EnableCOMDATFolding")->addTextElement ("true");
                }

                const StringArray librarySearchPaths (config.getLibrarySearchPaths());
                if (librarySearchPaths.size() > 0)
                    link->createNewChildElement ("AdditionalLibraryDirectories")->addTextElement (replacePreprocessorTokens (config, librarySearchPaths.joinIntoString (";"))
                                                                                                    + ";%(AdditionalLibraryDirectories)");

                link->createNewChildElement ("LargeAddressAware")->addTextElement ("true");

                String externalLibraries (getExternalLibrariesString());
                if (externalLibraries.isNotEmpty())
                    link->createNewChildElement ("AdditionalDependencies")->addTextElement (replacePreprocessorTokens (config, externalLibraries).trim()
                                                                                              + ";%(AdditionalDependencies)");

                String extraLinkerOptions (getExtraLinkerFlagsString());
                if (extraLinkerOptions.isNotEmpty())
                    link->createNewChildElement ("AdditionalOptions")->addTextElement (replacePreprocessorTokens (config, extraLinkerOptions).trim()
                                                                                         + " %(AdditionalOptions)");

                if (msvcDelayLoadedDLLs.isNotEmpty())
                    link->createNewChildElement ("DelayLoadDLLs")->addTextElement (msvcDelayLoadedDLLs);

                if (config.config [Ids::msvcModuleDefinitionFile].toString().isNotEmpty())
                    link->createNewChildElement ("ModuleDefinitionFile")
                        ->addTextElement (config.config [Ids::msvcModuleDefinitionFile].toString());
            }

            {
                XmlElement* bsc = group->createNewChildElement ("Bscmake");
                bsc->createNewChildElement ("SuppressStartupBanner")->addTextElement ("true");
                bsc->createNewChildElement ("OutputFile")->addTextElement (getIntDirFile (config, config.getOutputFilename (".bsc", true)));
            }

            if (config.getPrebuildCommandString().isNotEmpty())
                group->createNewChildElement ("PreBuildEvent")
                     ->createNewChildElement ("Command")
                     ->addTextElement (config.getPrebuildCommandString());

            const String internalPostBuildScripts = config.config[Ids::internalPostBuildComamnd].toString();
            if (config.getPostbuildCommandString().isNotEmpty() || internalPostBuildScripts.isNotEmpty())
                group->createNewChildElement ("PostBuildEvent")
                     ->createNewChildElement ("Command")
                     ->addTextElement (config.getPostbuildCommandString() + internalPostBuildScripts);
        }

        ScopedPointer<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

        {
            XmlElement* cppFiles    = projectXml.createNewChildElement ("ItemGroup");
            XmlElement* headerFiles = projectXml.createNewChildElement ("ItemGroup");

            for (int i = 0; i < getAllGroups().size(); ++i)
            {
                const Project::Item& group = getAllGroups().getReference(i);

                if (group.getNumChildren() > 0)
                    addFilesToCompile (group, *cppFiles, *headerFiles, *otherFilesGroup);
            }
        }

        if (iconFile != File())
        {
            XmlElement* e = otherFilesGroup->createNewChildElement ("None");
            e->setAttribute ("Include", prependDot (iconFile.getFileName()));
        }

        if (otherFilesGroup->getFirstChildElement() != nullptr)
            projectXml.addChildElement (otherFilesGroup.release());

        if (hasResourceFile())
        {
            XmlElement* rcGroup = projectXml.createNewChildElement ("ItemGroup");
            XmlElement* e = rcGroup->createNewChildElement ("ResourceCompile");
            e->setAttribute ("Include", prependDot (rcFile.getFileName()));
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("Import");
            e->setAttribute ("Project", "$(VCTargetsPath)\\Microsoft.Cpp.targets");
        }

        {
            XmlElement* e = projectXml.createNewChildElement ("ImportGroup");
            e->setAttribute ("Label", "ExtensionTargets");
        }
    }

    String getProjectType() const
    {
        if (projectType.isGUIApplication() || projectType.isCommandLineApp())   return "Application";
        if (isLibraryDLL())                                                     return "DynamicLibrary";
        if (projectType.isStaticLibrary())                                      return "StaticLibrary";

        jassertfalse;
        return String();
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

    //==============================================================================
    void addFilesToCompile (const Project::Item& projectItem, XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFilesToCompile (projectItem.getChild(i), cpps, headers, otherFiles);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            jassert (path.getRoot() == RelativePath::buildTargetFolder);

            if (path.hasFileExtension (cOrCppFileExtensions) || path.hasFileExtension (asmFileExtensions))
            {
                XmlElement* e = cpps.createNewChildElement ("ClCompile");
                e->setAttribute ("Include", path.toWindowsStyle());

                if (! projectItem.shouldBeCompiled())
                    e->createNewChildElement ("ExcludedFromBuild")->addTextElement ("true");

                if (shouldUseStdCall (path))
                    e->createNewChildElement ("CallingConvention")->addTextElement ("StdCall");
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

    //==============================================================================
    void addFilterGroup (XmlElement& groups, const String& path) const
    {
        XmlElement* e = groups.createNewChildElement ("Filter");
        e->setAttribute ("Include", path);
        e->createNewChildElement ("UniqueIdentifier")->addTextElement (createGUID (path + "_guidpathsaltxhsdf"));
    }

    void addFileToFilter (const RelativePath& file, const String& groupPath,
                          XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles) const
    {
        XmlElement* e;

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

    void addFilesToFilter (const Project::Item& projectItem, const String& path,
                           XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups) const
    {
        if (projectItem.isGroup())
        {
            addFilterGroup (groups, path);

            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addFilesToFilter (projectItem.getChild(i),
                                  (path.isEmpty() ? String() : (path + "\\")) + projectItem.getChild(i).getName(),
                                  cpps, headers, otherFiles, groups);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            addFileToFilter (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder),
                             path.upToLastOccurrenceOf ("\\", false, false), cpps, headers, otherFiles);
        }
    }

    void addFilesToFilter (const Array<RelativePath>& files, const String& path,
                           XmlElement& cpps, XmlElement& headers, XmlElement& otherFiles, XmlElement& groups)
    {
        if (files.size() > 0)
        {
            addFilterGroup (groups, path);

            for (int i = 0; i < files.size(); ++i)
                addFileToFilter (files.getReference(i), path, cpps, headers, otherFiles);
        }
    }

    void fillInFiltersXml (XmlElement& filterXml) const
    {
        filterXml.setAttribute ("ToolsVersion", getToolsVersion());
        filterXml.setAttribute ("xmlns", "http://schemas.microsoft.com/developer/msbuild/2003");

        XmlElement* groupsXml  = filterXml.createNewChildElement ("ItemGroup");
        XmlElement* cpps       = filterXml.createNewChildElement ("ItemGroup");
        XmlElement* headers    = filterXml.createNewChildElement ("ItemGroup");
        ScopedPointer<XmlElement> otherFilesGroup (new XmlElement ("ItemGroup"));

        for (int i = 0; i < getAllGroups().size(); ++i)
        {
            const Project::Item& group = getAllGroups().getReference(i);

            if (group.getNumChildren() > 0)
                addFilesToFilter (group, group.getName(), *cpps, *headers, *otherFilesGroup, *groupsXml);
        }

        if (iconFile.exists())
        {
            XmlElement* e = otherFilesGroup->createNewChildElement ("None");
            e->setAttribute ("Include", prependDot (iconFile.getFileName()));
            e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getJuceCodeGroupName());
        }

        if (otherFilesGroup->getFirstChildElement() != nullptr)
            filterXml.addChildElement (otherFilesGroup.release());

        if (hasResourceFile())
        {
            XmlElement* rcGroup = filterXml.createNewChildElement ("ItemGroup");
            XmlElement* e = rcGroup->createNewChildElement ("ResourceCompile");
            e->setAttribute ("Include", prependDot (rcFile.getFileName()));
            e->createNewChildElement ("Filter")->addTextElement (ProjectSaver::getJuceCodeGroupName());
        }
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2010)
};

//==============================================================================
class MSVCProjectExporterVC2012 : public MSVCProjectExporterVC2010
{
public:
    MSVCProjectExporterVC2012 (Project& p, const ValueTree& t,
                               const char* folderName = "VisualStudio2012")
        : MSVCProjectExporterVC2010 (p, t, folderName)
    {
        name = getName();
    }

    static const char* getName()                { return "Visual Studio 2012"; }
    static const char* getValueTreeTypeName()   { return "VS2012"; }
    int getVisualStudioVersion() const override { return 11; }
    String getSolutionComment() const override  { return "# Visual Studio 2012"; }
    String getDefaultToolset() const override   { return "v110"; }

    static MSVCProjectExporterVC2012* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MSVCProjectExporterVC2012 (project, settings);

        return nullptr;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        MSVCProjectExporterBase::createExporterProperties (props);

        static const char* toolsetNames[] = { "(default)", "v110", "v110_xp", "Windows7.1SDK", "CTP_Nov2013" };
        const var toolsets[]              = { var(),       "v110", "v110_xp", "Windows7.1SDK", "CTP_Nov2013" };

        addToolsetProperty (props, toolsetNames, toolsets, numElementsInArray (toolsets));
        addIPPLibraryProperty (props);
    }

private:
    void addPlatformToolsetToPropertyGroup (XmlElement& p) const override
    {
        forEachXmlChildElementWithTagName (p, e, "PropertyGroup")
            e->createNewChildElement ("PlatformToolset")->addTextElement (getPlatformToolset());
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2012)
};

//==============================================================================
class MSVCProjectExporterVC2013  : public MSVCProjectExporterVC2012
{
public:
    MSVCProjectExporterVC2013 (Project& p, const ValueTree& t)
        : MSVCProjectExporterVC2012 (p, t, "VisualStudio2013")
    {
        name = getName();
    }

    static const char* getName()                { return "Visual Studio 2013"; }
    static const char* getValueTreeTypeName()   { return "VS2013"; }
    int getVisualStudioVersion() const override { return 12; }
    String getSolutionComment() const override  { return "# Visual Studio 2013"; }
    String getToolsVersion() const override     { return "12.0"; }
    String getDefaultToolset() const override   { return "v120"; }

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
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2013)
};

//==============================================================================
class MSVCProjectExporterVC2015  : public MSVCProjectExporterVC2012
{
public:
    MSVCProjectExporterVC2015 (Project& p, const ValueTree& t)
        : MSVCProjectExporterVC2012 (p, t, "VisualStudio2015")
    {
        name = getName();
    }

    static const char* getName()                { return "Visual Studio 2015"; }
    static const char* getValueTreeTypeName()   { return "VS2015"; }
    int getVisualStudioVersion() const override { return 14; }
    String getSolutionComment() const override  { return "# Visual Studio 2015"; }
    String getToolsVersion() const override     { return "14.0"; }
    String getDefaultToolset() const override   { return "v140"; }

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
    }

    JUCE_DECLARE_NON_COPYABLE (MSVCProjectExporterVC2015)
};
