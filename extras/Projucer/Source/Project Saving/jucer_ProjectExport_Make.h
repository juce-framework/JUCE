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

class MakefileProjectExporter  : public ProjectExporter
{
protected:
    //==============================================================================
    class MakeBuildConfiguration  : public BuildConfiguration
    {
    public:
        MakeBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
        : BuildConfiguration (p, settings, e)
        {
        }

        Value getArchitectureType()             { return getValue (Ids::linuxArchitecture); }
        var getArchitectureTypeVar() const      { return config [Ids::linuxArchitecture]; }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);

            static const char* const archNames[] = { "(Default)", "<None>",       "32-bit (-m32)", "64-bit (-m64)", "ARM v6",       "ARM v7" };
            const var archFlags[]                = { var(),       var (String()), "-m32",         "-m64",           "-march=armv6", "-march=armv7" };

            props.add (new ChoicePropertyComponent (getArchitectureType(), "Architecture",
                                                    StringArray (archNames, numElementsInArray (archNames)),
                                                    Array<var> (archFlags, numElementsInArray (archFlags))));
        }

        String getModuleLibraryArchName() const override
        {
            String archFlag = getArchitectureTypeVar();
            String prefix ("-march=");

            if (archFlag.startsWith (prefix))
                return archFlag.substring (prefix.length());

            if (archFlag == "-m64")
                return "x86_64";

            if (archFlag == "-m32")
                return "i386";

            return "$(shell uname -m)";
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return new MakeBuildConfiguration (project, tree, *this);
    }

public:
    //==============================================================================
    class MakefileTarget : public ProjectType::Target
    {
    public:
        MakefileTarget (ProjectType::Target::Type targetType, const MakefileProjectExporter& exporter)
            : ProjectType::Target (targetType), owner (exporter)
        {}

        StringArray getTargetSettings (const BuildConfiguration& config) const
        {
            if (type == AggregateTarget)
                // the aggregate target should not specify any settings at all!
                // it just defines dependencies on the other targets.
                return {};

            StringPairArray commonOptions  = owner.getAllPreprocessorDefs (config, ProjectType::Target::unspecified);
            StringPairArray targetSpecific = owner.getAllPreprocessorDefs (config, type);

            StringArray defs;

            // remove any defines that have already been added by the configuration
            const int n = targetSpecific.size();
            for (int i = 0 ; i < n; ++i)
            {
                const String& key = targetSpecific.getAllKeys()[i];

                if (! commonOptions.getAllKeys().contains (key))
                    defs.add (String ("-D") + key + String ("=") + targetSpecific.getAllValues()[i]);
            }

            StringArray s;

            const String cppflagsVarName = String ("JUCE_CPPFLAGS_") + getTargetVarName();

            s.add (cppflagsVarName + String (" := ") + defs.joinIntoString (" "));

            if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle)
            {
                s.add (String ("JUCE_CFLAGS_") + getTargetVarName() + String (" := -fPIC -fvisibility=hidden"));

                const String ldflagsVarName = String ("JUCE_LDFLAGS_") + getTargetVarName();
                String targetLinkOptions = ldflagsVarName  + String (" := -shared");

                if (getTargetFileType() == pluginBundle)
                    targetLinkOptions += " -Wl,--no-undefined";

                s.add (targetLinkOptions);
            }

            String targetName (owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString()));

            if (owner.projectType.isStaticLibrary())
                targetName = getStaticLibbedFilename (targetName);
            else if (owner.projectType.isDynamicLibrary())
                targetName = getDynamicLibbedFilename (targetName);
            else
                targetName = targetName.upToLastOccurrenceOf (".", false, false) + getTargetFileSuffix();

            s.add (String ("JUCE_TARGET_") + getTargetVarName() + String (" := ") + escapeSpaces (targetName));

            return s;
        }

        String getTargetFileSuffix() const
        {
            switch (type)
            {
                case VSTPlugIn:             return ".so";
                case VST3PlugIn:            return ".vst3";
                case SharedCodeTarget:      return ".a";
                default:                    break;
            }

            return {};
        }

        String getTargetVarName() const
        {
            return String (getName()).toUpperCase().replaceCharacter (L' ', L'_');
        }

        void writeObjects (OutputStream& out) const
        {
            Array<RelativePath> targetFiles;
            for (int i = 0; i < owner.getAllGroups().size(); ++i)
                findAllFilesToCompile (owner.getAllGroups().getReference(i), targetFiles);

            out << "OBJECTS_" + getTargetVarName() + String (" := \\") << newLine;

            for (int i = 0; i < targetFiles.size(); ++i)
                out << "  $(JUCE_OBJDIR)/" << escapeSpaces (owner.getObjectFileFor (targetFiles.getReference(i))) << " \\" << newLine;

            out << newLine;
        }

        void findAllFilesToCompile (const Project::Item& projectItem, Array<RelativePath>& results) const
        {
            if (projectItem.isGroup())
            {
                for (int i = 0; i < projectItem.getNumChildren(); ++i)
                    findAllFilesToCompile (projectItem.getChild(i), results);
            }
            else
            {
                if (projectItem.shouldBeCompiled())
                {
                    const Type targetType = (owner.getProject().getProjectType().isAudioPlugin() ? type : SharedCodeTarget);
                    const File f = projectItem.getFile();
                    RelativePath relativePath (f, owner.getTargetFolder(), RelativePath::buildTargetFolder);

                    if (owner.shouldFileBeCompiledByDefault (relativePath)
                     && owner.getProject().getTargetTypeFromFilePath (f, true) == targetType)
                        results.add (relativePath);
                }
            }
        }

        void addFiles (OutputStream& out)
        {
            Array<RelativePath> targetFiles;
            for (int i = 0; i < owner.getAllGroups().size(); ++i)
                findAllFilesToCompile (owner.getAllGroups().getReference(i), targetFiles);

            const String cppflagsVarName = String ("JUCE_CPPFLAGS_") + getTargetVarName();
            const String cflagsVarName   = String ("JUCE_CFLAGS_")   + getTargetVarName();

            for (int i = 0; i < targetFiles.size(); ++i)
            {
                jassert (targetFiles.getReference(i).getRoot() == RelativePath::buildTargetFolder);

                out << "$(JUCE_OBJDIR)/" << escapeSpaces (owner.getObjectFileFor (targetFiles.getReference(i)))
                << ": " << escapeSpaces (targetFiles.getReference(i).toUnixStyle()) << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_OBJDIR)" << newLine
                << "\t@echo \"Compiling " << targetFiles.getReference(i).getFileName() << "\"" << newLine
                << (targetFiles.getReference(i).hasFileExtension ("c;s;S") ? "\t$(V_AT)$(CC) $(JUCE_CFLAGS) " : "\t$(V_AT)$(CXX) $(JUCE_CXXFLAGS) ")
                << "$(" << cppflagsVarName << ") $(" << cflagsVarName << ") -o \"$@\" -c \"$<\""
                << newLine << newLine;
            }
        }

        String getBuildProduct() const
        {
            return String ("$(JUCE_OUTDIR)/$(JUCE_TARGET_") + getTargetVarName() + String (")");
        }

        String getPhonyName() const
        {
            return String (getName()).upToFirstOccurrenceOf (" ", false, false);
        }

        void writeTargetLine (OutputStream& out, const bool useLinuxPackages)
        {
            jassert (type != AggregateTarget);

            out << getBuildProduct() << " : "
                << ((useLinuxPackages) ? "check-pkg-config " : "")
                << "$(OBJECTS_" << getTargetVarName() << ") $(RESOURCES)";

            if (type != SharedCodeTarget && owner.shouldBuildTargetType (SharedCodeTarget))
                out << " $(JUCE_OUTDIR)/$(JUCE_TARGET_SHARED_CODE)";

            out << newLine << "\t@echo Linking \"" << owner.projectName << " - " << getName() << "\"" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_BINDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_LIBDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)" << newLine;

            if (owner.projectType.isStaticLibrary() || type == SharedCodeTarget)
                out << "\t$(V_AT)$(AR) -rcs " << getBuildProduct()
                    << " $(OBJECTS_" << getTargetVarName() << ")" << newLine;
            else
            {
                out << "\t$(V_AT)$(CXX) -o " << getBuildProduct()
                    << " $(OBJECTS_" << getTargetVarName() << ") ";

                if (owner.shouldBuildTargetType (SharedCodeTarget))
                    out << "$(JUCE_OUTDIR)/$(JUCE_TARGET_SHARED_CODE) ";

                out << "$(JUCE_LDFLAGS) ";

                if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle)
                    out << "$(JUCE_LDFLAGS_" << getTargetVarName() << ") ";

                out << "$(RESOURCES) $(TARGET_ARCH)" << newLine;
            }

            out << newLine;
        }

        const MakefileProjectExporter& owner;
    };

    //==============================================================================
    static const char* getNameLinux()           { return "Linux Makefile"; }
    static const char* getValueTreeTypeName()   { return "LINUX_MAKE"; }

    Value getExtraPkgConfig()                   { return getSetting (Ids::linuxExtraPkgConfig); }
    String getExtraPkgConfigString() const      { return getSettingString (Ids::linuxExtraPkgConfig); }

    static MakefileProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MakefileProjectExporter (project, settings);

        return nullptr;
    }

    StringArray getPackages() const
    {
        StringArray packages;
        packages.addTokens (getExtraPkgConfigString(), " ", "\"'");
        packages.removeEmptyStrings();

        packages.addArray (linuxPackages);

        if (isWebBrowserComponentEnabled())
        {
            packages.add ("webkit2gtk-4.0");
            packages.add ("gtk+-x11-3.0");
        }

        packages.removeDuplicates (false);

        return packages;
    }


    //==============================================================================
    MakefileProjectExporter (Project& p, const ValueTree& t)   : ProjectExporter (p, t)
    {
        name = getNameLinux();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "LinuxMakefile";
    }

    //==============================================================================
    bool canLaunchProject() override                    { return false; }
    bool launchProject() override                       { return false; }
    bool usesMMFiles() const override                   { return false; }
    bool canCopeWithDuplicateFiles() override           { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                       { return false; }
    bool isVisualStudio() const override                { return false; }
    bool isCodeBlocks() const override                  { return false; }
    bool isMakefile() const override                    { return true; }
    bool isAndroidStudio() const override               { return false; }

    bool isAndroid() const override                     { return false; }
    bool isWindows() const override                     { return false; }
    bool isLinux() const override                       { return true; }
    bool isOSX() const override                         { return false; }
    bool isiOS() const override                         { return false; }

    bool supportsTargetType (ProjectType::Target::Type type) const override
    {
        switch (type)
        {
            case ProjectType::Target::GUIApp:
            case ProjectType::Target::ConsoleApp:
            case ProjectType::Target::StaticLibrary:
            case ProjectType::Target::SharedCodeTarget:
            case ProjectType::Target::AggregateTarget:
            case ProjectType::Target::VSTPlugIn:
            case ProjectType::Target::StandalonePlugIn:
            case ProjectType::Target::DynamicLibrary:
                return true;
            default:
                break;
        }

        return false;
    }

    void createExporterProperties (PropertyListBuilder& properties) override
    {
        properties.add (new TextPropertyComponent (getExtraPkgConfig(), "pkg-config libraries", 8192, false),
                   "Extra pkg-config libraries for you application. Each package should be space separated.");
    }

    //==============================================================================
    bool anyTargetIsSharedLibrary() const
    {
        const int n = targets.size();
        for (int i = 0; i < n; ++i)
        {
            const ProjectType::Target::TargetFileType fileType = targets.getUnchecked (i)->getTargetFileType();
            if (fileType == ProjectType::Target::sharedLibraryOrDLL || fileType == ProjectType::Target::pluginBundle)
                return true;
        }

        return false;
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        MemoryOutputStream mo;
        writeMakefile (mo);

        overwriteFileIfDifferentOrThrow (getTargetFolder().getChildFile ("Makefile"), mo);
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override
    {
        callForAllSupportedTargets ([this] (ProjectType::Target::Type targetType)
                                    {
                                        if (MakefileTarget* target = new MakefileTarget (targetType, *this))
                                        {
                                            if (targetType == ProjectType::Target::AggregateTarget)
                                                targets.insert (0, target);
                                            else
                                                targets.add (target);
                                        }
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

    //==============================================================================
    void initialiseDependencyPathValues() override
    {
        vst3Path.referTo (Value (new DependencyPathValueSource (getSetting (Ids::vst3Folder),
                                                                Ids::vst3Path,
                                                                TargetOS::linux)));
    }

private:
    bool isWebBrowserComponentEnabled() const
    {
        static String guiExtrasModule ("juce_gui_extra");

        return (project.getModules().isModuleEnabled (guiExtrasModule)
                && project.isConfigFlagEnabled ("JUCE_WEB_BROWSER", true));
    }

    //==============================================================================
    void writeDefineFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        StringPairArray defines;
        defines.set ("LINUX", "1");

        if (config.isDebug())
        {
            defines.set ("DEBUG", "1");
            defines.set ("_DEBUG", "1");
        }
        else
        {
            defines.set ("NDEBUG", "1");
        }

        out << createGCCPreprocessorFlags (mergePreprocessorDefs (defines, getAllPreprocessorDefs (config, ProjectType::Target::unspecified)));
    }

    void writeHeaderPathFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());

        auto packages = getPackages();
        if (packages.size() > 0)
        {
            out << " $(shell pkg-config --cflags";

            for (int i = 0; i < packages.size(); ++i)
                out << " " << packages[i];

            out << ")";
        }

        if (linuxLibs.contains("pthread"))
            out << " -pthread";

        searchPaths = getCleanedStringArray (searchPaths);

        // Replace ~ character with $(HOME) environment variable
        for (int i = 0; i < searchPaths.size(); ++i)
            out << " -I" << escapeSpaces (FileHelpers::unixStylePath (replacePreprocessorTokens (config, searchPaths[i]))).replace ("~", "$(HOME)");
    }

    void writeCppFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        out << "  JUCE_CPPFLAGS := $(DEPFLAGS)";
        writeDefineFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << " $(CPPFLAGS)" << newLine;
    }

    void writeLinkerFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        out << "  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR)";

        {
            StringArray flags (makefileExtraLinkerFlags);

            if (! config.isDebug())
                flags.add ("-fvisibility=hidden");

            if (flags.size() > 0)
                out << " " << getCleanedStringArray (flags).joinIntoString (" ");
        }

        out << config.getGCCLibraryPathFlags();

        auto packages = getPackages();
        if (packages.size() > 0 )
        {
            out << " $(shell pkg-config --libs";

            for (int i = 0; i < packages.size(); ++i)
                out << " " << packages[i];

            out << ")";
        }

        for (int i = 0; i < linuxLibs.size(); ++i)
            out << " -l" << linuxLibs[i];

        StringArray libraries;
        libraries.addTokens (getExternalLibrariesString(), ";", "\"'");
        libraries.removeEmptyStrings();

        if (libraries.size() != 0)
            out << " -l" << replacePreprocessorTokens (config, libraries.joinIntoString (" -l")).trim();

        out << " " << replacePreprocessorTokens (config, getExtraLinkerFlagsString()).trim()
            << " $(LDFLAGS)" << newLine;
    }

    void writeTargetLines (OutputStream& out, const bool useLinuxPackages) const
    {
        const int n = targets.size();

        for (int i = 0; i < n; ++i)
        {
            if (MakefileTarget* target = targets.getUnchecked (i))
            {
                if (target->type == ProjectType::Target::AggregateTarget)
                {
                    StringArray dependencies;
                    MemoryOutputStream subTargetLines;

                    for (int j = 0; j < n; ++j)
                    {
                        if (i == j) continue;

                        if (MakefileTarget* dependency = targets.getUnchecked (j))
                        {
                            if (dependency->type != ProjectType::Target::SharedCodeTarget)
                            {
                                auto phonyName = dependency->getPhonyName();

                                subTargetLines << phonyName << " : " << dependency->getBuildProduct() << newLine;
                                dependencies.add (phonyName);
                            }
                        }
                    }

                    out << "all : " << dependencies.joinIntoString (" ") << newLine << newLine;
                    out << subTargetLines.toString() << newLine << newLine;
                }
                else
                {
                    if (! getProject().getProjectType().isAudioPlugin())
                        out << "all : " << target->getBuildProduct() << newLine << newLine;

                    target->writeTargetLine (out, useLinuxPackages);
                }
            }
        }
    }

    void writeConfig (OutputStream& out, const BuildConfiguration& config) const
    {
        const String buildDirName ("build");
        const String intermediatesDirName (buildDirName + "/intermediate/" + config.getName());
        String outputDir (buildDirName);

        if (config.getTargetBinaryRelativePathString().isNotEmpty())
        {
            RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
            outputDir = binaryPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder).toUnixStyle();
        }

        out << "ifeq ($(CONFIG)," << escapeSpaces (config.getName()) << ")" << newLine;
        out << "  JUCE_BINDIR := " << escapeSpaces (buildDirName) << newLine
            << "  JUCE_LIBDIR := " << escapeSpaces (buildDirName) << newLine
            << "  JUCE_OBJDIR := " << escapeSpaces (intermediatesDirName) << newLine
            << "  JUCE_OUTDIR := " << escapeSpaces (outputDir) << newLine
            << newLine
            << "  ifeq ($(TARGET_ARCH),)" << newLine
            << "    TARGET_ARCH := " << getArchFlags (config) << newLine
            << "  endif"  << newLine
            << newLine;

        writeCppFlags (out, config);

        for (auto target : targets)
        {
            StringArray lines = target->getTargetSettings (config);

            if (lines.size() > 0)
                out << "  " << lines.joinIntoString ("\n  ") << newLine;

            out << newLine;
        }

        out << "  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH)";

        if (anyTargetIsSharedLibrary())
            out << " -fPIC";

        if (config.isDebug())
            out << " -g -ggdb";

        out << " -O" << config.getGCCOptimisationFlag()
            << (" "  + replacePreprocessorTokens (config, getExtraCompilerFlagsString())).trimEnd()
            << " $(CFLAGS)" << newLine;

        {
            auto cppStandard = config.project.getCppStandardValue().toString();

            if (cppStandard == "latest")
                cppStandard = "1z";

            cppStandard = "-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard;

            out << "  JUCE_CXXFLAGS += $(CXXFLAGS) $(JUCE_CFLAGS) "
                << cppStandard << " $(CXXFLAGS)" << newLine;
        }

        writeLinkerFlags (out, config);

        out << newLine;

        out << "  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)" << newLine
            << "endif" << newLine
            << newLine;
    }

    void writeIncludeLines (OutputStream& out) const
    {
        const int n = targets.size();

        for (int i = 0; i < n; ++i)
        {
            if (MakefileTarget* target = targets.getUnchecked (i))
            {
                if (target->type == ProjectType::Target::AggregateTarget)
                    continue;

                out << "-include $(OBJECTS_" << target->getTargetVarName()
                    << ":%.o=%.d)" << newLine;
            }
        }
    }

    void writeMakefile (OutputStream& out) const
    {
        out << "# Automatically generated makefile, created by the Projucer" << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
            << newLine;

        out << "# build with \"V=1\" for verbose builds" << newLine
            << "ifeq ($(V), 1)" << newLine
            << "V_AT =" << newLine
            << "else" << newLine
            << "V_AT = @" << newLine
            << "endif" << newLine
            << newLine;

        out << "# (this disables dependency generation if multiple architectures are set)" << newLine
            << "DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)" << newLine
            << newLine;

        out << "ifndef STRIP" << newLine
            << "  STRIP=strip" << newLine
            << "endif" << newLine
            << newLine;

        out << "ifndef AR" << newLine
            << "  AR=ar" << newLine
            << "endif" << newLine
            << newLine;

        out << "ifndef CONFIG" << newLine
            << "  CONFIG=" << escapeSpaces (getConfiguration(0)->getName()) << newLine
            << "endif" << newLine
            << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            writeConfig (out, *config);

        for (auto target : targets)
            target->writeObjects (out);

        out << getPhonyTargetLine() << newLine << newLine;

        StringArray packages;
        packages.addTokens (getExtraPkgConfigString(), " ", "\"'");
        packages.removeEmptyStrings();

        const bool useLinuxPackages = (linuxPackages.size() > 0 || packages.size() > 0);

        writeTargetLines (out, useLinuxPackages);

        for (auto target : targets)
            target->addFiles (out);

        if (useLinuxPackages)
        {
            out << "check-pkg-config:" << newLine
                << "\t@command -v pkg-config >/dev/null 2>&1 || "
                "{ echo >&2 \"pkg-config not installed. Please, install it.\"; "
                "exit 1; }" << newLine
                << "\t@pkg-config --print-errors";

            for (int i = 0; i < linuxPackages.size(); ++i)
                out << " " << linuxPackages[i];

            for (int i = 0; i < packages.size(); ++i)
                out << " " << packages[i];

            out << newLine << newLine;
        }

        out << "clean:" << newLine
            << "\t@echo Cleaning " << projectName << newLine
            << "\t$(V_AT)$(CLEANCMD)" << newLine
            << newLine;

        out << "strip:" << newLine
            << "\t@echo Stripping " << projectName << newLine
            << "\t-$(V_AT)$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(TARGET)" << newLine
            << newLine;

        writeIncludeLines (out);
    }

    String getArchFlags (const BuildConfiguration& config) const
    {
        if (const MakeBuildConfiguration* makeConfig = dynamic_cast<const MakeBuildConfiguration*> (&config))
            if (! makeConfig->getArchitectureTypeVar().isVoid())
                return makeConfig->getArchitectureTypeVar();

        return "-march=native";
    }

    String getObjectFileFor (const RelativePath& file) const
    {
        return file.getFileNameWithoutExtension()
                + "_" + String::toHexString (file.toUnixStyle().hashCode()) + ".o";
    }

    String getPhonyTargetLine() const
    {
        MemoryOutputStream phonyTargetLine;

        phonyTargetLine << ".PHONY: clean all";

        if (! getProject().getProjectType().isAudioPlugin())
            return phonyTargetLine.toString();

        for (auto target : targets)
            if (target->type != ProjectType::Target::SharedCodeTarget
                   && target->type != ProjectType::Target::AggregateTarget)
                phonyTargetLine << " " << target->getPhonyName();

        return phonyTargetLine.toString();
    }

    OwnedArray<MakefileTarget> targets;

    JUCE_DECLARE_NON_COPYABLE (MakefileProjectExporter)
};
