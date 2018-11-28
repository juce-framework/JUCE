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
class MakefileProjectExporter  : public ProjectExporter
{
protected:
    //==============================================================================
    class MakeBuildConfiguration  : public BuildConfiguration
    {
    public:
        MakeBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e),
              architectureTypeValue (config, Ids::linuxArchitecture, getUndoManager(), "-march=native")
        {
            linkTimeOptimisationValue.setDefault (false);
            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);
        }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);

            props.add (new ChoicePropertyComponent (architectureTypeValue, "Architecture",
                                                    { "<None>",      "Native",        "32-bit (-m32)", "64-bit (-m64)", "ARM v6",       "ARM v7" },
                                                    { { String() } , "-march=native", "-m32",          "-m64",          "-march=armv6", "-march=armv7" }),
                       "Specifies the 32/64-bit architecture to use.");
        }

        String getModuleLibraryArchName() const override
        {
            auto archFlag = getArchitectureTypeString();
            String prefix ("-march=");

            if (archFlag.startsWith (prefix))
                return archFlag.substring (prefix.length());

            if (archFlag == "-m64")
                return "x86_64";

            if (archFlag == "-m32")
                return "i386";

            return "${JUCE_ARCH_LABEL}";
        }

        String getArchitectureTypeString() const    { return architectureTypeValue.get(); }

        //==============================================================================
        ValueWithDefault architectureTypeValue;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return *new MakeBuildConfiguration (project, tree, *this);
    }

public:
    //==============================================================================
    class MakefileTarget : public ProjectType::Target
    {
    public:
        MakefileTarget (ProjectType::Target::Type targetType, const MakefileProjectExporter& exporter)
            : ProjectType::Target (targetType), owner (exporter)
        {}

        StringArray getCompilerFlags() const
        {
            StringArray result;

            if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle)
            {
                result.add ("-fPIC");
                result.add ("-fvisibility=hidden");
            }

            return result;
        }

        StringArray getLinkerFlags() const
        {
            StringArray result;

            if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle)
            {
                result.add ("-shared");

                if (getTargetFileType() == pluginBundle)
                    result.add ("-Wl,--no-undefined");
            }

            return result;
        }

        StringPairArray getDefines (const BuildConfiguration& config) const
        {
            StringPairArray result;
            auto commonOptionKeys = owner.getAllPreprocessorDefs (config, ProjectType::Target::unspecified).getAllKeys();
            auto targetSpecific = owner.getAllPreprocessorDefs (config, type);

            for (auto& key : targetSpecific.getAllKeys())
                if (! commonOptionKeys.contains (key))
                    result.set (key, targetSpecific[key]);

            return result;
        }

        StringArray getTargetSettings (const MakeBuildConfiguration& config) const
        {
            if (type == AggregateTarget)
                // the aggregate target should not specify any settings at all!
                // it just defines dependencies on the other targets.
                return {};

            StringArray defines;
            auto defs = getDefines (config);

            for (auto& key : defs.getAllKeys())
                defines.add ("-D" + key + "=" + defs[key]);

            StringArray s;

            auto cppflagsVarName = "JUCE_CPPFLAGS_" + getTargetVarName();

            s.add (cppflagsVarName + " := " + defines.joinIntoString (" "));

            auto cflags = getCompilerFlags();

            if (! cflags.isEmpty())
                s.add ("JUCE_CFLAGS_" + getTargetVarName() + " := " + cflags.joinIntoString (" "));

            auto ldflags = getLinkerFlags();

            if (! ldflags.isEmpty())
                s.add ("JUCE_LDFLAGS_" + getTargetVarName() + " := " + ldflags.joinIntoString (" "));

            auto targetName = owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString());

            if (owner.projectType.isStaticLibrary())
                targetName = getStaticLibbedFilename (targetName);
            else if (owner.projectType.isDynamicLibrary())
                targetName = getDynamicLibbedFilename (targetName);
            else
                targetName = targetName.upToLastOccurrenceOf (".", false, false) + getTargetFileSuffix();

            s.add ("JUCE_TARGET_" + getTargetVarName() + String (" := ") + escapeSpaces (targetName));

            return s;
        }

        String getTargetFileSuffix() const
        {
            switch (type)
            {
                case VSTPlugIn:
                case UnityPlugIn:
                case DynamicLibrary:        return ".so";
                case SharedCodeTarget:
                case StaticLibrary:         return ".a";
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
                    auto targetType = (owner.getProject().getProjectType().isAudioPlugin() ? type : SharedCodeTarget);
                    auto f = projectItem.getFile();
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

            auto cppflagsVarName = "JUCE_CPPFLAGS_" + getTargetVarName();
            auto cflagsVarName   = "JUCE_CFLAGS_"   + getTargetVarName();

            for (int i = 0; i < targetFiles.size(); ++i)
            {
                jassert (targetFiles.getReference(i).getRoot() == RelativePath::buildTargetFolder);

                out << "$(JUCE_OBJDIR)/" << escapeSpaces (owner.getObjectFileFor (targetFiles.getReference(i)))
                    << ": " << escapeSpaces (targetFiles.getReference(i).toUnixStyle())            << newLine
                    << "\t-$(V_AT)mkdir -p $(JUCE_OBJDIR)"                                         << newLine
                    << "\t@echo \"Compiling " << targetFiles.getReference(i).getFileName() << "\"" << newLine
                    << (targetFiles.getReference(i).hasFileExtension ("c;s;S") ? "\t$(V_AT)$(CC) $(JUCE_CFLAGS) "
                                                                               : "\t$(V_AT)$(CXX) $(JUCE_CXXFLAGS) ")
                    << "$(" << cppflagsVarName << ") $(" << cflagsVarName << ") -o \"$@\" -c \"$<\""
                    << newLine
                    << newLine;
            }
        }

        String getBuildProduct() const
        {
            return "$(JUCE_OUTDIR)/$(JUCE_TARGET_" + getTargetVarName() + ")";
        }

        String getPhonyName() const
        {
            return String (getName()).upToFirstOccurrenceOf (" ", false, false);
        }

        void writeTargetLine (OutputStream& out, const StringArray& packages)
        {
            jassert (type != AggregateTarget);

            out << getBuildProduct() << " : "
                << "$(OBJECTS_" << getTargetVarName() << ") $(RESOURCES)";

            if (type != SharedCodeTarget && owner.shouldBuildTargetType (SharedCodeTarget))
                out << " $(JUCE_OUTDIR)/$(JUCE_TARGET_SHARED_CODE)";

            out << newLine;

            if (! packages.isEmpty())
            {
                out << "\t@command -v pkg-config >/dev/null 2>&1 || { echo >&2 \"pkg-config not installed. Please, install it.\"; exit 1; }" << newLine
                    << "\t@pkg-config --print-errors";

                for (auto& pkg : packages)
                    out << " " << pkg;

                out << newLine;
            }

            out << "\t@echo Linking \"" << owner.projectName << " - " << getName() << "\"" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_BINDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_LIBDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)" << newLine;

            if (type == UnityPlugIn)
            {
                auto scriptName = owner.getProject().getUnityScriptName();

                RelativePath scriptPath (owner.getProject().getGeneratedCodeFolder().getChildFile (scriptName),
                                         owner.getTargetFolder(),
                                         RelativePath::projectFolder);

                out << "\t-$(V_AT)cp " + scriptPath.toUnixStyle() + " $(JUCE_OUTDIR)/" + scriptName << newLine;
            }

            if (owner.projectType.isStaticLibrary() || type == SharedCodeTarget)
            {
                out << "\t$(V_AT)$(AR) -rcs " << getBuildProduct()
                    << " $(OBJECTS_" << getTargetVarName() << ")" << newLine;
            }
            else
            {
                out << "\t$(V_AT)$(CXX) -o " << getBuildProduct()
                    << " $(OBJECTS_" << getTargetVarName() << ") ";

                if (owner.shouldBuildTargetType (SharedCodeTarget))
                    out << "$(JUCE_OUTDIR)/$(JUCE_TARGET_SHARED_CODE) ";

                out << "$(JUCE_LDFLAGS) ";

                if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle
                        || type == GUIApp || type == StandalonePlugIn)
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

    String getExtraPkgConfigString() const      { return extraPkgConfigValue.get(); }

    static MakefileProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MakefileProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    MakefileProjectExporter (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          extraPkgConfigValue (settings, Ids::linuxExtraPkgConfig, getUndoManager())
    {
        name = getNameLinux();

        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderForExporter (getValueTreeTypeName()));
    }

    //==============================================================================
    bool canLaunchProject() override                        { return false; }
    bool launchProject() override                           { return false; }
    bool usesMMFiles() const override                       { return false; }
    bool canCopeWithDuplicateFiles() override               { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                           { return false; }
    bool isVisualStudio() const override                    { return false; }
    bool isCodeBlocks() const override                      { return false; }
    bool isMakefile() const override                        { return true; }
    bool isAndroidStudio() const override                   { return false; }
    bool isCLion() const override                           { return false; }

    bool isAndroid() const override                         { return false; }
    bool isWindows() const override                         { return false; }
    bool isLinux() const override                           { return true; }
    bool isOSX() const override                             { return false; }
    bool isiOS() const override                             { return false; }

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
            case ProjectType::Target::UnityPlugIn:
                return true;
            default:
                break;
        }

        return false;
    }

    void createExporterProperties (PropertyListBuilder& properties) override
    {
        properties.add (new TextPropertyComponent (extraPkgConfigValue, "pkg-config libraries", 8192, false),
                   "Extra pkg-config libraries for you application. Each package should be space separated.");
    }

    //==============================================================================
    bool anyTargetIsSharedLibrary() const
    {
        for (auto* target : targets)
        {
            auto fileType = target->getTargetFileType();

            if (fileType == ProjectType::Target::sharedLibraryOrDLL
             || fileType == ProjectType::Target::pluginBundle)
                return true;
        }

        return false;
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        MemoryOutputStream mo;
        mo.setNewLineString ("\n");

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

private:
    ValueWithDefault extraPkgConfigValue;

    //==============================================================================
    StringPairArray getDefines (const BuildConfiguration& config) const
    {
        StringPairArray result;

        result.set ("LINUX", "1");

        if (config.isDebug())
        {
            result.set ("DEBUG", "1");
            result.set ("_DEBUG", "1");
        }
        else
        {
            result.set ("NDEBUG", "1");
        }

        result = mergePreprocessorDefs (result, getAllPreprocessorDefs (config, ProjectType::Target::unspecified));

        return result;
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

        // don't add libcurl if curl symbols are loaded at runtime
        if (! isLoadCurlSymbolsLazilyEnabled())
            packages.add ("libcurl");

        packages.removeDuplicates (false);

        return packages;
    }

    String getPreprocessorPkgConfigFlags() const
    {
        auto packages = getPackages();

        if (packages.size() > 0)
            return "$(shell pkg-config --cflags " + packages.joinIntoString (" ") + ")";

        return {};
    }

    String getLinkerPkgConfigFlags() const
    {
        auto packages = getPackages();

        if (packages.size() > 0)
            return "$(shell pkg-config --libs " + packages.joinIntoString (" ") + ")";

        return {};
    }

    StringArray getCPreprocessorFlags (const BuildConfiguration&) const
    {
        StringArray result;

        if (linuxLibs.contains("pthread"))
            result.add ("-pthread");

        return result;
    }

    StringArray getCFlags (const BuildConfiguration& config) const
    {
        StringArray result;

        if (anyTargetIsSharedLibrary())
            result.add ("-fPIC");

        if (config.isDebug())
        {
            result.add ("-g");
            result.add ("-ggdb");
        }

        result.add ("-O" + config.getGCCOptimisationFlag());

        if (config.isLinkTimeOptimisationEnabled())
            result.add ("-flto");

        auto extra = replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim();

        if (extra.isNotEmpty())
            result.add (extra);

        return result;
    }

    StringArray getCXXFlags() const
    {
        StringArray result;

        auto cppStandard = project.getCppStandardString();

        if (cppStandard == "latest")
            cppStandard = "17";

        cppStandard = "-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard;

        result.add (cppStandard);

        return result;
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths = getCleanedStringArray (searchPaths);

        StringArray result;

        for (auto& path : searchPaths)
            result.add (FileHelpers::unixStylePath (replacePreprocessorTokens (config, path)));

        return result;
    }

    StringArray getLibraryNames (const BuildConfiguration& config) const
    {
        StringArray result (linuxLibs);

        auto libraries = StringArray::fromTokens (getExternalLibrariesString(), ";", "\"'");
        libraries.removeEmptyStrings();

        for (auto& lib : libraries)
            result.add (replacePreprocessorTokens (config, lib).trim());

        return result;
    }

    StringArray getLibrarySearchPaths (const BuildConfiguration& config) const
    {
        auto result = getSearchPathsFromString (config.getLibrarySearchPathString());

        for (auto path : moduleLibSearchPaths)
            result.add (path + "/" + config.getModuleLibraryArchName());

        return result;
    }

    StringArray getLinkerFlags (const BuildConfiguration& config) const
    {
        auto result = makefileExtraLinkerFlags;

        if (! config.isDebug())
            result.add ("-fvisibility=hidden");

        if (config.isLinkTimeOptimisationEnabled())
            result.add ("-flto");

        auto extraFlags = getExtraLinkerFlagsString().trim();

        if (extraFlags.isNotEmpty())
            result.add (replacePreprocessorTokens (config, extraFlags));

        return result;
    }

    bool isWebBrowserComponentEnabled() const
    {
        static String guiExtrasModule ("juce_gui_extra");

        return (project.getEnabledModules().isModuleEnabled (guiExtrasModule)
                && project.isConfigFlagEnabled ("JUCE_WEB_BROWSER", true));
    }

    bool isLoadCurlSymbolsLazilyEnabled() const
    {
        static String juceCoreModule ("juce_core");

        return (project.getEnabledModules().isModuleEnabled (juceCoreModule)
                && project.isConfigFlagEnabled ("JUCE_LOAD_CURL_SYMBOLS_LAZILY", false));
    }

    //==============================================================================
    void writeDefineFlags (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        out << createGCCPreprocessorFlags (mergePreprocessorDefs (getDefines (config), getAllPreprocessorDefs (config, ProjectType::Target::unspecified)));
    }

    void writePkgConfigFlags (OutputStream& out) const
    {
        auto flags = getPreprocessorPkgConfigFlags();

        if (flags.isNotEmpty())
            out << " " << flags;
    }

    void writeCPreprocessorFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        auto flags = getCPreprocessorFlags (config);

        if (! flags.isEmpty())
            out << " " << flags.joinIntoString (" ");
    }

    void writeHeaderPathFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        for (auto& path : getHeaderSearchPaths (config))
            out << " -I" << escapeSpaces (path).replace ("~", "$(HOME)");
    }

    void writeCppFlags (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        out << "  JUCE_CPPFLAGS := $(DEPFLAGS)";
        writeDefineFlags (out, config);
        writePkgConfigFlags (out);
        writeCPreprocessorFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << " $(CPPFLAGS)" << newLine;
    }

    void writeLinkerFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        out << "  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR)";

        for (auto path : getLibrarySearchPaths (config))
            out << " -L" << escapeSpaces (path).replace ("~", "$(HOME)");

        auto pkgConfigFlags = getLinkerPkgConfigFlags();

        if (pkgConfigFlags.isNotEmpty())
            out << " " << getLinkerPkgConfigFlags();

        auto linkerFlags = getLinkerFlags (config).joinIntoString (" ");

        if (linkerFlags.isNotEmpty())
            out << " " << linkerFlags;

        for (auto& libName : getLibraryNames (config))
            out << " -l" << libName;

        out << " $(LDFLAGS)" << newLine;
    }

    void writeTargetLines (OutputStream& out, const StringArray& packages) const
    {
        auto n = targets.size();

        for (int i = 0; i < n; ++i)
        {
            if (auto* target = targets.getUnchecked (i))
            {
                if (target->type == ProjectType::Target::AggregateTarget)
                {
                    StringArray dependencies;
                    MemoryOutputStream subTargetLines;

                    for (int j = 0; j < n; ++j)
                    {
                        if (i == j) continue;

                        if (auto* dependency = targets.getUnchecked (j))
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

                    target->writeTargetLine (out, packages);
                }
            }
        }
    }

    void writeConfig (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        String buildDirName ("build");
        auto intermediatesDirName = buildDirName + "/intermediate/" + config.getName();
        auto outputDir = buildDirName;

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
            auto lines = target->getTargetSettings (config);

            if (lines.size() > 0)
                out << "  " << lines.joinIntoString ("\n  ") << newLine;

            out << newLine;
        }

        out << "  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH)";

        auto cflags = getCFlags (config).joinIntoString (" ");

        if (cflags.isNotEmpty())
            out << " " << cflags;

        out << " $(CFLAGS)" << newLine;

        out << "  JUCE_CXXFLAGS += $(JUCE_CFLAGS)";

        auto cxxflags = getCXXFlags().joinIntoString (" ");

        if (cxxflags.isNotEmpty())
            out << " " << cxxflags;

        out << " $(CXXFLAGS)" << newLine;

        writeLinkerFlags (out, config);

        out << newLine;

        out << "  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)" << newLine
            << "endif" << newLine
            << newLine;
    }

    void writeIncludeLines (OutputStream& out) const
    {
        auto n = targets.size();

        for (int i = 0; i < n; ++i)
        {
            if (auto* target = targets.getUnchecked (i))
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

        out << "JUCE_ARCH_LABEL := $(shell uname -m)" << newLine
            << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            writeConfig (out, dynamic_cast<const MakeBuildConfiguration&> (*config));

        for (auto target : targets)
            target->writeObjects (out);

        out << getPhonyTargetLine() << newLine << newLine;

        writeTargetLines (out, getPackages());

        for (auto target : targets)
            target->addFiles (out);out << "clean:" << newLine
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
        if (auto* makeConfig = dynamic_cast<const MakeBuildConfiguration*> (&config))
            return makeConfig->getArchitectureTypeString();

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

        phonyTargetLine << ".PHONY: clean all strip";

        if (! getProject().getProjectType().isAudioPlugin())
            return phonyTargetLine.toString();

        for (auto target : targets)
            if (target->type != ProjectType::Target::SharedCodeTarget
                   && target->type != ProjectType::Target::AggregateTarget)
                phonyTargetLine << " " << target->getPhonyName();

        return phonyTargetLine.toString();
    }

    friend class CLionProjectExporter;

    OwnedArray<MakefileTarget> targets;

    JUCE_DECLARE_NON_COPYABLE (MakefileProjectExporter)
};
