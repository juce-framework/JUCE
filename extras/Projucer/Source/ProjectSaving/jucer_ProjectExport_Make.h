/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
              architectureTypeValue     (config, Ids::linuxArchitecture,          getUndoManager(), String()),
              pluginBinaryCopyStepValue (config, Ids::enablePluginBinaryCopyStep, getUndoManager(), true),
              vstBinaryLocation         (config, Ids::vstBinaryLocation,          getUndoManager(), "$(HOME)/.vst"),
              vst3BinaryLocation        (config, Ids::vst3BinaryLocation,         getUndoManager(), "$(HOME)/.vst3"),
              lv2BinaryLocation         (config, Ids::lv2BinaryLocation,          getUndoManager(), "$(HOME)/.lv2"),
              unityPluginBinaryLocation (config, Ids::unityPluginBinaryLocation,  getUndoManager(), "$(HOME)/UnityPlugins")
        {
            linkTimeOptimisationValue.setDefault (false);
            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);
        }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addRecommendedLinuxCompilerWarningsProperty (props);
            addGCCOptimisationProperty (props);

            props.add (new ChoicePropertyComponent (architectureTypeValue, "Architecture",
                                                    { "<None>",     "Native",        "32-bit (-m32)", "64-bit (-m64)", "ARM v6",       "ARM v7",       "ARM v8-a" },
                                                    { { String() }, "-march=native", "-m32",          "-m64",          "-march=armv6", "-march=armv7", "-march=armv8-a" }),
                       "Specifies the 32/64-bit architecture to use. If you don't see the required architecture in this list, you can also specify the desired "
                       "flag on the command-line when invoking make by passing \"TARGET_ARCH=-march=<arch to use>\"");

            auto isBuildingAnyPlugins = (project.shouldBuildVST() || project.shouldBuildVST3() || project.shouldBuildUnityPlugin() || project.shouldBuildLV2());

            if (isBuildingAnyPlugins)
            {
                props.add (new ChoicePropertyComponent (pluginBinaryCopyStepValue, "Enable Plugin Copy Step"),
                           "Enable this to copy plugin binaries to a specified folder after building.");

                if (project.shouldBuildVST3())
                    props.add (new TextPropertyComponentWithEnablement (vst3BinaryLocation, pluginBinaryCopyStepValue, "VST3 Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled VST3 binary should be placed.");

                if (project.shouldBuildLV2())
                    props.add (new TextPropertyComponentWithEnablement (lv2BinaryLocation, pluginBinaryCopyStepValue, "LV2 Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled LV2 binary should be placed.");

                if (project.shouldBuildUnityPlugin())
                    props.add (new TextPropertyComponentWithEnablement (unityPluginBinaryLocation, pluginBinaryCopyStepValue, "Unity Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled Unity plugin binary and associated C# GUI script should be placed.");

                if (project.shouldBuildVST())
                    props.add (new TextPropertyComponentWithEnablement (vstBinaryLocation, pluginBinaryCopyStepValue, "VST (Legacy) Binary Location",
                                                                        1024, false),
                               "The folder in which the compiled legacy VST binary should be placed.");
            }
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

        String getArchitectureTypeString() const           { return architectureTypeValue.get(); }

        bool isPluginBinaryCopyStepEnabled() const         { return pluginBinaryCopyStepValue.get(); }
        String getVSTBinaryLocationString() const          { return vstBinaryLocation.get(); }
        String getVST3BinaryLocationString() const         { return vst3BinaryLocation.get(); }
        String getLV2BinaryLocationString() const          { return lv2BinaryLocation.get(); }
        String getUnityPluginBinaryLocationString() const  { return unityPluginBinaryLocation.get(); }

    private:
        //==============================================================================
        ValueTreePropertyWithDefault architectureTypeValue, pluginBinaryCopyStepValue,
                                     vstBinaryLocation, vst3BinaryLocation, lv2BinaryLocation, unityPluginBinaryLocation;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return *new MakeBuildConfiguration (project, tree, *this);
    }

public:
    //==============================================================================
    class MakefileTarget : public build_tools::ProjectType::Target
    {
    public:
        MakefileTarget (build_tools::ProjectType::Target::Type targetType, const MakefileProjectExporter& exporter)
            : build_tools::ProjectType::Target (targetType), owner (exporter)
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
            auto commonOptionKeys = owner.getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified).getAllKeys();
            auto targetSpecific = owner.getAllPreprocessorDefs (config, type);

            for (auto& key : targetSpecific.getAllKeys())
                if (! commonOptionKeys.contains (key))
                    result.set (key, targetSpecific[key]);

            return result;
        }

        StringArray getTargetSettings (const MakeBuildConfiguration& config) const
        {
            if (type == AggregateTarget) // the aggregate target should not specify any settings at all!
                return {};               // it just defines dependencies on the other targets.

            StringArray s;

            auto cppflagsVarName = "JUCE_CPPFLAGS_" + getTargetVarName();

            s.add (cppflagsVarName + " := " + createGCCPreprocessorFlags (getDefines (config)));

            auto cflags = getCompilerFlags();

            if (! cflags.isEmpty())
                s.add ("JUCE_CFLAGS_" + getTargetVarName() + " := " + cflags.joinIntoString (" "));

            auto ldflags = getLinkerFlags();

            if (! ldflags.isEmpty())
                s.add ("JUCE_LDFLAGS_" + getTargetVarName() + " := " + ldflags.joinIntoString (" "));

            auto targetName = owner.replacePreprocessorTokens (config, config.getTargetBinaryNameString (type == UnityPlugIn));

            if (owner.projectType.isStaticLibrary())
                targetName = getStaticLibbedFilename (targetName);
            else if (owner.projectType.isDynamicLibrary())
                targetName = getDynamicLibbedFilename (targetName);
            else
                targetName = targetName.upToLastOccurrenceOf (".", false, false) + getTargetFileSuffix();

            if (type == VST3PlugIn)
            {
                s.add ("JUCE_VST3DIR := " + escapeQuotesAndSpaces (targetName).upToLastOccurrenceOf (".", false, false) + ".vst3");
                s.add ("VST3_PLATFORM_ARCH := $(shell $(CXX) make_helpers/arch_detection.cpp 2>&1 | tr '\\n' ' ' | sed \"s/.*JUCE_ARCH \\([a-zA-Z0-9_-]*\\).*/\\1/\")");
                s.add ("JUCE_VST3SUBDIR := Contents/$(VST3_PLATFORM_ARCH)-linux");

                targetName = "$(JUCE_VST3DIR)/$(JUCE_VST3SUBDIR)/" + targetName;
            }
            else if (type == UnityPlugIn)
            {
                s.add ("JUCE_UNITYDIR := Unity");
                targetName = "$(JUCE_UNITYDIR)/" + targetName;
            }
            else if (type == LV2PlugIn)
            {
                s.add ("JUCE_LV2DIR := " + escapeQuotesAndSpaces (targetName) + ".lv2");
                targetName = "$(JUCE_LV2DIR)/" + targetName + ".so";
            }
            else if (type == LV2Helper)
            {
                targetName = Project::getLV2FileWriterName();
            }
            else if (type == VST3Helper)
            {
                targetName = Project::getVST3FileWriterName();
            }

            s.add ("JUCE_TARGET_" + getTargetVarName() + String (" := ") + escapeQuotesAndSpaces (targetName));

            if (type == LV2PlugIn)
                s.add ("JUCE_LV2_FULL_PATH := $(JUCE_OUTDIR)/$(JUCE_TARGET_LV2_PLUGIN)");

            if (config.isPluginBinaryCopyStepEnabled()
                && (type == VST3PlugIn || type == VSTPlugIn || type == UnityPlugIn || type == LV2PlugIn))
            {
                String copyCmd ("JUCE_COPYCMD_" + getTargetVarName() + String (" := $(JUCE_OUTDIR)/"));

                if (type == VST3PlugIn)
                {
                    s.add ("JUCE_VST3DESTDIR := " + config.getVST3BinaryLocationString());
                    s.add (copyCmd + "$(JUCE_VST3DIR) $(JUCE_VST3DESTDIR)");
                }
                else if (type == VSTPlugIn)
                {
                    s.add ("JUCE_VSTDESTDIR := " + config.getVSTBinaryLocationString());
                    s.add (copyCmd + escapeQuotesAndSpaces (targetName) + " $(JUCE_VSTDESTDIR)");
                }
                else if (type == UnityPlugIn)
                {
                    s.add ("JUCE_UNITYDESTDIR := " + config.getUnityPluginBinaryLocationString());
                    s.add (copyCmd + "$(JUCE_UNITYDIR)/. $(JUCE_UNITYDESTDIR)");
                }
                else if (type == LV2PlugIn)
                {
                    s.add ("JUCE_LV2DESTDIR := " + config.getLV2BinaryLocationString());
                    s.add (copyCmd + "$(JUCE_LV2DIR) $(JUCE_LV2DESTDIR)");
                }
            }

            return s;
        }

        String getTargetFileSuffix() const
        {
            if (type == VSTPlugIn || type == VST3PlugIn || type == UnityPlugIn || type == DynamicLibrary)
                return ".so";

            if (type == SharedCodeTarget || type == StaticLibrary)
                return ".a";

            return {};
        }

        String getTargetVarName() const
        {
            return String (getName()).toUpperCase().replaceCharacter (L' ', L'_');
        }

        void writeObjects (OutputStream& out, const std::vector<std::pair<build_tools::RelativePath, String>>& filesToCompile) const
        {
            out << "OBJECTS_" + getTargetVarName() + String (" := \\") << newLine;

            for (auto& f : filesToCompile)
                out << "  $(JUCE_OBJDIR)/" << escapeQuotesAndSpaces (owner.getObjectFileFor (f.first))
                    << " \\" << newLine;

            out << newLine;
        }

        void addFiles (OutputStream& out, const std::vector<std::pair<build_tools::RelativePath, String>>& filesToCompile)
        {
            auto cppflagsVarName = "JUCE_CPPFLAGS_" + getTargetVarName();
            auto cflagsVarName   = "JUCE_CFLAGS_"   + getTargetVarName();

            for (auto& [path, flags] : filesToCompile)
            {
                const auto additionalTargetDependencies = [&path = path, this]
                {
                    if (   owner.linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper()
                        && path.getFileName().contains ("include_juce_gui_extra.cpp"))
                    {
                        return owner.linuxSubprocessHelperProperties
                            .getLinuxSubprocessHelperBinaryDataSource()
                            .toUnixStyle();
                    }

                    return String{};
                }();

                const auto prependedWithSpaceIfNotEmpty = [] (auto s)
                {
                    return s.isEmpty() ? s : " " + s;
                };

                out << "$(JUCE_OBJDIR)/" << escapeQuotesAndSpaces (owner.getObjectFileFor (path)) << ": " << escapeQuotesAndSpaces (path.toUnixStyle())
                    << prependedWithSpaceIfNotEmpty (additionalTargetDependencies)                                                                      << newLine
                    << "\t-$(V_AT)mkdir -p $(@D)"                                                                                                       << newLine
                    << "\t@echo \"Compiling " << path.getFileName() << "\""                                                                             << newLine
                    << (path.hasFileExtension ("c;s;S") ? "\t$(V_AT)$(CC) $(JUCE_CFLAGS) " : "\t$(V_AT)$(CXX) $(JUCE_CXXFLAGS) ")
                    << "$(" << cppflagsVarName << ") $(" << cflagsVarName << ")"
                    << (flags.isNotEmpty() ? " $(" + owner.getCompilerFlagSchemeVariableName (flags) + ")" : "") << " -o \"$@\" -c \"$<\""              << newLine
                    << newLine;
            }
        }

        String getBuildProduct() const
        {
            return "$(JUCE_OUTDIR)/$(JUCE_TARGET_" + getTargetVarName() + ")";
        }

        String getPhonyName() const
        {
            if (type == LV2Helper)
                return "LV2_MANIFEST_HELPER";

            if (type == VST3Helper)
                return "VST3_MANIFEST_HELPER";

            return String (getName()).upToFirstOccurrenceOf (" ", false, false);
        }

        void writeTargetLine (OutputStream& out, const StringArray& packages)
        {
            jassert (type != AggregateTarget);

            out << getBuildProduct() << " : "
                << "$(OBJECTS_" << getTargetVarName() << ") $(JUCE_OBJDIR)/execinfo.cmd $(RESOURCES)";

            if (type != SharedCodeTarget && owner.shouldBuildTargetType (SharedCodeTarget))
                out << " $(JUCE_OUTDIR)/$(JUCE_TARGET_SHARED_CODE)";

            if (type == LV2PlugIn)
                out << " $(JUCE_OUTDIR)/$(JUCE_TARGET_LV2_MANIFEST_HELPER)";
            else if (type == VST3PlugIn)
                out << " $(JUCE_OUTDIR)/$(JUCE_TARGET_VST3_MANIFEST_HELPER)";

            out << newLine;

            if (! packages.isEmpty())
            {
                out << "\t@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo >&2 \"pkg-config not installed. Please, install it.\"; exit 1; }" << newLine
                    << "\t@$(PKG_CONFIG) --print-errors";

                for (auto& pkg : packages)
                    out << " " << pkg;

                out << newLine;
            }

            out << "\t@echo Linking \"" << owner.projectName << " - " << getName() << "\"" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_BINDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_LIBDIR)" << newLine
                << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)" << newLine;

            if (type == VST3PlugIn)
                out << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)/$(JUCE_VST3DIR)/$(JUCE_VST3SUBDIR)" << newLine;
            else if (type == UnityPlugIn)
                out << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)/$(JUCE_UNITYDIR)" << newLine;
            else if (type == LV2PlugIn)
                out << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)/$(JUCE_LV2DIR)" << newLine;

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

                out << "$(JUCE_LDFLAGS) $(shell cat $(JUCE_OBJDIR)/execinfo.cmd) ";

                if (getTargetFileType() == sharedLibraryOrDLL || getTargetFileType() == pluginBundle
                        || type == GUIApp || type == StandalonePlugIn)
                    out << "$(JUCE_LDFLAGS_" << getTargetVarName() << ") ";

                out << "$(RESOURCES) $(TARGET_ARCH)" << newLine;
            }

            if (type == VST3PlugIn)
            {
                out << "\t-$(V_AT)mkdir -p $(JUCE_OUTDIR)/$(JUCE_VST3DIR)/Contents/Resources" << newLine
                    << "\t-$(V_AT)rm -f $(JUCE_OUTDIR)/$(JUCE_VST3DIR)/Contents/moduleinfo.json" << newLine
                    << "\t$(V_AT) $(JUCE_OUTDIR)/$(JUCE_TARGET_VST3_MANIFEST_HELPER) "
                       "-create "
                       "-version " << owner.project.getVersionString().quoted() << " "
                       "-path $(JUCE_OUTDIR)/$(JUCE_VST3DIR) "
                       "-output $(JUCE_OUTDIR)/$(JUCE_VST3DIR)/Contents/Resources/moduleinfo.json" << newLine
                    << "\t-$(V_AT)[ ! \"$(JUCE_VST3DESTDIR)\" ] || (mkdir -p $(JUCE_VST3DESTDIR) && cp -R $(JUCE_COPYCMD_VST3))" << newLine;
            }
            else if (type == VSTPlugIn)
            {
                out << "\t-$(V_AT)[ ! \"$(JUCE_VSTDESTDIR)\" ]  || (mkdir -p $(JUCE_VSTDESTDIR)  && cp -R $(JUCE_COPYCMD_VST))"  << newLine;
            }
            else if (type == UnityPlugIn)
            {
                auto scriptName = owner.getProject().getUnityScriptName();

                build_tools::RelativePath scriptPath (owner.getProject().getGeneratedCodeFolder().getChildFile (scriptName),
                                                      owner.getTargetFolder(),
                                                      build_tools::RelativePath::projectFolder);

                out << "\t-$(V_AT)cp " + scriptPath.toUnixStyle() + " $(JUCE_OUTDIR)/$(JUCE_UNITYDIR)" << newLine
                    << "\t-$(V_AT)[ ! \"$(JUCE_UNITYDESTDIR)\" ] || (mkdir -p $(JUCE_UNITYDESTDIR) && cp -R $(JUCE_COPYCMD_UNITY_PLUGIN))" << newLine;
            }
            else if (type == LV2PlugIn)
            {
                out << "\t$(V_AT) $(JUCE_OUTDIR)/$(JUCE_TARGET_LV2_MANIFEST_HELPER) $(JUCE_LV2_FULL_PATH)" << newLine
                    << "\t-$(V_AT)[ ! \"$(JUCE_LV2DESTDIR)\" ] || (mkdir -p $(JUCE_LV2DESTDIR) && cp -R $(JUCE_COPYCMD_LV2_PLUGIN))" << newLine;
            }

            out << newLine;
        }

        const MakefileProjectExporter& owner;
    };

    //==============================================================================
    static String getDisplayName()        { return "Linux Makefile"; }
    static String getValueTreeTypeName()  { return "LINUX_MAKE"; }
    static String getTargetFolderName()   { return "LinuxMakefile"; }

    Identifier getExporterIdentifier() const override { return getValueTreeTypeName(); }

    static MakefileProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName()))
            return new MakefileProjectExporter (projectToUse, settingsToUse);

        return nullptr;
    }

    //==============================================================================
    MakefileProjectExporter (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          extraPkgConfigValue (settings, Ids::linuxExtraPkgConfig, getUndoManager())
    {
        name = getDisplayName();
        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderName());
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

    bool isAndroid() const override                         { return false; }
    bool isWindows() const override                         { return false; }
    bool isLinux() const override                           { return true; }
    bool isOSX() const override                             { return false; }
    bool isiOS() const override                             { return false; }

    String getNewLineString() const override                { return "\n"; }

    bool supportsTargetType (build_tools::ProjectType::Target::Type type) const override
    {
        using Target = build_tools::ProjectType::Target;

        switch (type)
        {
            case Target::GUIApp:
            case Target::ConsoleApp:
            case Target::StaticLibrary:
            case Target::SharedCodeTarget:
            case Target::AggregateTarget:
            case Target::VSTPlugIn:
            case Target::VST3PlugIn:
            case Target::VST3Helper:
            case Target::StandalonePlugIn:
            case Target::DynamicLibrary:
            case Target::UnityPlugIn:
            case Target::LV2PlugIn:
            case Target::LV2Helper:
                return true;
            case Target::AAXPlugIn:
            case Target::AudioUnitPlugIn:
            case Target::AudioUnitv3PlugIn:
            case Target::unspecified:
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

    void initialiseDependencyPathValues() override
    {
        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                          getAppSettings().getStoredPath (Ids::vstLegacyPath, TargetOS::linux), TargetOS::linux);

        araPathValueWrapper.init ({ settings, Ids::araFolder, nullptr },
                                  getAppSettings().getStoredPath (Ids::araPath, TargetOS::linux), TargetOS::linux);
    }

    //==============================================================================
    bool anyTargetIsSharedLibrary() const
    {
        for (auto* target : targets)
        {
            auto fileType = target->getTargetFileType();

            if (fileType == build_tools::ProjectType::Target::sharedLibraryOrDLL
             || fileType == build_tools::ProjectType::Target::pluginBundle)
                return true;
        }

        return false;
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        build_tools::writeStreamToFile (getTargetFolder().getChildFile ("Makefile"), [&] (MemoryOutputStream& mo)
        {
            mo.setNewLineString (getNewLineString());
            writeMakefile (mo);
        });

        if (project.shouldBuildVST3())
        {
            auto helperDir = getTargetFolder().getChildFile ("make_helpers");
            helperDir.createDirectory();
            build_tools::overwriteFileIfDifferentOrThrow (helperDir.getChildFile ("arch_detection.cpp"),
                                                          BinaryData::juce_runtime_arch_detection_cpp);
        }

        linuxSubprocessHelperProperties.deployLinuxSubprocessHelperSourceFilesIfNecessary();
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) override
    {
        linuxSubprocessHelperProperties.addToExtraSearchPathsIfNecessary();

        callForAllSupportedTargets ([this] (build_tools::ProjectType::Target::Type targetType)
                                    {
                                        targets.insert (targetType == build_tools::ProjectType::Target::AggregateTarget ? 0 : -1,
                                                        new MakefileTarget (targetType, *this));
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

private:
    ValueTreePropertyWithDefault extraPkgConfigValue;

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

        result = mergePreprocessorDefs (result, getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified));

        return result;
    }

    StringArray getExtraPkgConfigPackages() const
    {
        auto packages = StringArray::fromTokens (extraPkgConfigValue.get().toString(), " ", "\"'");
        packages.removeEmptyStrings();

        return packages;
    }

    StringArray getCompilePackages() const
    {
        auto packages = getLinuxPackages (PackageDependencyType::compile);
        packages.addArray (getExtraPkgConfigPackages());

        return packages;
    }

    StringArray getLinkPackages() const
    {
        auto packages = getLinuxPackages (PackageDependencyType::link);
        packages.addArray (getExtraPkgConfigPackages());

        return packages;
    }

    String getPreprocessorPkgConfigFlags() const
    {
        auto compilePackages = getCompilePackages();

        if (compilePackages.size() > 0)
            return "$(shell $(PKG_CONFIG) --cflags " + compilePackages.joinIntoString (" ") + ")";

        return {};
    }

    String getLinkerPkgConfigFlags() const
    {
        auto linkPackages = getLinkPackages();

        if (linkPackages.size() > 0)
            return "$(shell $(PKG_CONFIG) --libs " + linkPackages.joinIntoString (" ") + ")";

        return {};
    }

    StringArray getCPreprocessorFlags (const BuildConfiguration&) const
    {
        StringArray result;

        if (linuxLibs.contains ("pthread"))
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

        for (auto& recommended : config.getRecommendedCompilerWarningFlags().common)
            result.add (recommended);

        auto extra = replacePreprocessorTokens (config, config.getAllCompilerFlagsString()).trim();

        if (extra.isNotEmpty())
            result.add (extra);

        return result;
    }

    StringArray getCXXFlags (const BuildConfiguration& config) const
    {
        StringArray result;

        for (auto& recommended : config.getRecommendedCompilerWarningFlags().cpp)
            result.add (recommended);

        auto cppStandard = project.getCppStandardString();

        if (cppStandard == "latest")
            cppStandard = project.getLatestNumberedCppStandardString();

        result.add ("-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard);

        return result;
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths = getCleanedStringArray (searchPaths);

        StringArray result;

        for (auto& path : searchPaths)
            result.add (build_tools::unixStylePath (replacePreprocessorTokens (config, path)));

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

        result.add ("-fvisibility=hidden");

        if (config.isLinkTimeOptimisationEnabled())
            result.add ("-flto");

        const auto extraFlags = config.getAllLinkerFlagsString().trim();

        if (extraFlags.isNotEmpty())
            result.add (replacePreprocessorTokens (config, extraFlags));

        return result;
    }

    //==============================================================================
    void writeDefineFlags (OutputStream& out, const MakeBuildConfiguration& config) const
    {
        out << createGCCPreprocessorFlags (mergePreprocessorDefs (getDefines (config), getAllPreprocessorDefs (config, build_tools::ProjectType::Target::unspecified)));
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
            out << " -I" << escapeQuotesAndSpaces (path).replace ("~", "$(HOME)");
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
            out << " -L" << escapeQuotesAndSpaces (path).replace ("~", "$(HOME)");

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
                if (target->type == build_tools::ProjectType::Target::AggregateTarget)
                {
                    StringArray dependencies;
                    MemoryOutputStream subTargetLines;

                    for (int j = 0; j < n; ++j)
                    {
                        if (i == j) continue;

                        if (auto* dependency = targets.getUnchecked (j))
                        {
                            if (dependency->type != build_tools::ProjectType::Target::SharedCodeTarget)
                            {
                                auto phonyName = dependency->getPhonyName();

                                subTargetLines << phonyName << " : " << dependency->getBuildProduct() << newLine;
                                dependencies.add (phonyName);
                            }
                        }
                    }

                    out << "all : " << dependencies.joinIntoString (" ") << newLine << newLine;
                    out << subTargetLines.toString()                     << newLine << newLine;
                }
                else
                {
                    if (! getProject().isAudioPluginProject())
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
            build_tools::RelativePath binaryPath (config.getTargetBinaryRelativePathString(), build_tools::RelativePath::projectFolder);
            outputDir = binaryPath.rebased (projectFolder, getTargetFolder(), build_tools::RelativePath::buildTargetFolder).toUnixStyle();
        }

        out << "ifeq ($(CONFIG)," << escapeQuotesAndSpaces (config.getName()) << ")" << newLine
            << "  JUCE_BINDIR := " << escapeQuotesAndSpaces (buildDirName)           << newLine
            << "  JUCE_LIBDIR := " << escapeQuotesAndSpaces (buildDirName)           << newLine
            << "  JUCE_OBJDIR := " << escapeQuotesAndSpaces (intermediatesDirName)   << newLine
            << "  JUCE_OUTDIR := " << escapeQuotesAndSpaces (outputDir)              << newLine
            << newLine
            << "  ifeq ($(TARGET_ARCH),)"                                   << newLine
            << "    TARGET_ARCH := " << getArchFlags (config)               << newLine
            << "  endif"                                                    << newLine
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

        auto cxxflags = getCXXFlags (config).joinIntoString (" ");

        if (cxxflags.isNotEmpty())
            out << " " << cxxflags;

        out << " $(CXXFLAGS)" << newLine;

        writeLinkerFlags (out, config);

        out << newLine;

        const auto preBuildDirectory = [&]() -> String
        {
            if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
            {
                using LSHP = LinuxSubprocessHelperProperties;
                const auto dataSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource();

                if (auto preBuildDir = LSHP::getParentDirectoryRelativeToBuildTargetFolder (dataSource))
                    return " " + *preBuildDir;
            }

            return "";
        }();

        out << "  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)" << preBuildDirectory << newLine
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
                if (target->type == build_tools::ProjectType::Target::AggregateTarget)
                    continue;

                out << "-include $(OBJECTS_" << target->getTargetVarName()
                    << ":%.o=%.d)" << newLine;
            }
        }
    }

    static String getCompilerFlagSchemeVariableName (const String& schemeName)   { return "JUCE_COMPILERFLAGSCHEME_" + schemeName; }

    std::vector<std::pair<File, String>> findAllFilesToCompile (const Project::Item& projectItem) const
    {
        std::vector<std::pair<File, String>> results;

        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
            {
                auto inner = findAllFilesToCompile (projectItem.getChild (i));
                results.insert (results.end(),
                                std::make_move_iterator (inner.cbegin()),
                                std::make_move_iterator (inner.cend()));
            }
        }
        else
        {
            if (projectItem.shouldBeCompiled())
            {
                auto f = projectItem.getFile();

                if (shouldFileBeCompiledByDefault (f))
                {
                    auto scheme = projectItem.getCompilerFlagSchemeString();
                    auto flags = getCompilerFlagsForProjectItem (projectItem);

                    if (scheme.isNotEmpty() && flags.isNotEmpty())
                        results.emplace_back (f, scheme);
                    else
                        results.emplace_back (f, String{});
                }
            }
        }

        return results;
    }

    void writeCompilerFlagSchemes (OutputStream& out, const std::vector<std::pair<File, String>>& filesToCompile) const
    {
        std::set<String> schemesToWrite;

        for (const auto& pair : filesToCompile)
            if (pair.second.isNotEmpty())
                schemesToWrite.insert (pair.second);

        if (schemesToWrite.empty())
            return;

        for (const auto& s : schemesToWrite)
            if (const auto flags = getCompilerFlagsForFileCompilerFlagScheme (s); flags.isNotEmpty())
                out << getCompilerFlagSchemeVariableName (s) << " := " << flags << newLine;

        out << newLine;
    }

    /*  These targets are responsible for building the juce_linux_subprocess_helper, the
        juce_simple_binary_builder, and then using the binary builder to create embeddable .h and .cpp
        files from the linux subprocess helper.
    */
    void writeSubprocessHelperTargets (OutputStream& out) const
    {
        using LSHP = LinuxSubprocessHelperProperties;

        const auto ensureDirs = [] (auto& outStream, std::vector<String> dirs)
        {
            for (const auto& dir : dirs)
                outStream << "\t-$(V_AT)mkdir -p " << dir << newLine;
        };

        const auto makeTarget = [&ensureDirs] (auto& outStream, String input, String output)
        {
            const auto isObjectTarget = output.endsWith (".o");
            const auto isSourceInput  = input.endsWith (".cpp");

            const auto targetOutput = isObjectTarget ? "$(JUCE_OBJDIR)/" + output : output;

            outStream << (isObjectTarget ? "$(JUCE_OBJDIR)/" : "") << output << ": " << input << newLine;

            const auto createBuildTargetRelative = [] (auto path)
            {
                return build_tools::RelativePath { path, build_tools::RelativePath::buildTargetFolder };
            };

            if (isObjectTarget)
                ensureDirs (outStream, { "$(JUCE_OBJDIR)" });
            else if (auto outputParentFolder = LSHP::getParentDirectoryRelativeToBuildTargetFolder (createBuildTargetRelative (output)))
                ensureDirs (outStream, { *outputParentFolder });

            outStream << (isObjectTarget ? "\t@echo \"Compiling " : "\t@echo \"Linking ")
                      << (isObjectTarget ? input : output) << "\"" << newLine
                      << "\t$(V_AT)$(CXX) $(JUCE_CXXFLAGS) -o " << targetOutput.quoted()
                      << " " << (isSourceInput ? "-c \"$<\"" : input.quoted());

            if (! isObjectTarget)
                outStream << " $(JUCE_LDFLAGS)";

            outStream << " $(TARGET_ARCH)" << newLine << newLine;

            return targetOutput;
        };

        const auto subprocessHelperSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperSource();

        const auto subprocessHelperObj = makeTarget (out,
                                                     subprocessHelperSource.toUnixStyle(),
                                                     getObjectFileFor (subprocessHelperSource));

        const auto subprocessHelperPath = makeTarget (out,
                                                      subprocessHelperObj,
                                                      "$(JUCE_BINDIR)/" + LSHP::getBinaryNameFromSource (subprocessHelperSource));

        const auto binaryBuilderSource = linuxSubprocessHelperProperties.getSimpleBinaryBuilderSource();

        const auto binaryBuilderObj = makeTarget (out,
                                                 binaryBuilderSource.toUnixStyle(),
                                                 getObjectFileFor (binaryBuilderSource));

        const auto binaryBuilderPath = makeTarget (out,
                                                   binaryBuilderObj,
                                                   "$(JUCE_BINDIR)/" + LSHP::getBinaryNameFromSource (binaryBuilderSource));

        const auto binaryDataSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource();
        jassert (binaryDataSource.getRoot() == build_tools::RelativePath::buildTargetFolder);

        out << binaryDataSource.toUnixStyle() << ": " << subprocessHelperPath
                                              << " " << binaryBuilderPath
                                              << newLine;

        const auto binarySourceDir = [&]() -> String
        {
            if (const auto p = LSHP::getParentDirectoryRelativeToBuildTargetFolder (binaryDataSource))
                return *p;

            return ".";
        }();

        out << "\t$(V_AT)" << binaryBuilderPath.quoted() << " " << subprocessHelperPath.quoted()
            << " " << binarySourceDir.quoted() << " " << binaryDataSource.getFileNameWithoutExtension().quoted()
            << " LinuxSubprocessHelperBinaryData" << newLine;

        out << newLine;
    }

    void writeMakefile (OutputStream& out) const
    {
        out << "# Automatically generated makefile, created by the Projucer"                                     << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
            << newLine;

        out << "# build with \"V=1\" for verbose builds" << newLine
            << "ifeq ($(V), 1)"                          << newLine
            << "V_AT ="                                  << newLine
            << "else"                                    << newLine
            << "V_AT = @"                                << newLine
            << "endif"                                   << newLine
            << newLine;

        out << "# (this disables dependency generation if multiple architectures are set)" << newLine
            << "DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)"                       << newLine
            << newLine;

        out << "ifndef PKG_CONFIG"       << newLine
            << "  PKG_CONFIG=pkg-config" << newLine
            << "endif"                   << newLine
            << newLine;

        out << "ifndef STRIP"  << newLine
            << "  STRIP=strip" << newLine
            << "endif"         << newLine
            << newLine;

        out << "ifndef AR" << newLine
            << "  AR=ar"   << newLine
            << "endif"     << newLine
            << newLine;

        out << "ifndef CONFIG"                                                       << newLine
            << "  CONFIG=" << escapeQuotesAndSpaces (getConfiguration(0)->getName()) << newLine
            << "endif"                                                               << newLine
            << newLine;

        out << "JUCE_ARCH_LABEL := $(shell uname -m)" << newLine
            << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            writeConfig (out, dynamic_cast<const MakeBuildConfiguration&> (*config));

        std::vector<std::pair<File, String>> filesToCompile;

        for (int i = 0; i < getAllGroups().size(); ++i)
        {
            auto group = findAllFilesToCompile (getAllGroups().getReference (i));
            filesToCompile.insert (filesToCompile.end(),
                                   std::make_move_iterator (group.cbegin()),
                                   std::make_move_iterator (group.cend()));
        }

        writeCompilerFlagSchemes (out, filesToCompile);

        const auto getFilesForTarget = [this] (const std::vector<std::pair<File, String>>& files,
                                               MakefileTarget* target,
                                               const Project& p)
        {
            std::vector<std::pair<build_tools::RelativePath, String>> targetFiles;

            auto targetType = (p.isAudioPluginProject() ? target->type : MakefileTarget::SharedCodeTarget);

            for (auto& [path, flags] : files)
            {
                if (p.getTargetTypeFromFilePath (path, true) == targetType)
                {
                    targetFiles.emplace_back (build_tools::RelativePath { path,
                                                                          getTargetFolder(),
                                                                          build_tools::RelativePath::buildTargetFolder },
                                              flags);
                }
            }

            if ((      targetType == MakefileTarget::SharedCodeTarget
                    || targetType == MakefileTarget::StaticLibrary
                    || targetType == MakefileTarget::DynamicLibrary)
                && linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
            {
                targetFiles.emplace_back (linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource(), "");
            }

            if (targetType == MakefileTarget::LV2Helper)
            {
                targetFiles.emplace_back (getLV2HelperProgramSource().rebased (projectFolder,
                                                                               getTargetFolder(),
                                                                               build_tools::RelativePath::buildTargetFolder),
                                          String{});
            }
            else if (targetType == MakefileTarget::VST3Helper)
            {
                targetFiles.emplace_back (getVST3HelperProgramSource().rebased (projectFolder,
                                                                                getTargetFolder(),
                                                                                build_tools::RelativePath::buildTargetFolder),
                                          String{});
            }

            return targetFiles;
        };

        for (auto target : targets)
            target->writeObjects (out, getFilesForTarget (filesToCompile, target, project));

        out << getPhonyTargetLine() << newLine << newLine;

        writeTargetLines (out, getLinkPackages());

        for (auto target : targets)
            target->addFiles (out, getFilesForTarget (filesToCompile, target, project));

        // libexecinfo is a separate library on BSD
        out << "$(JUCE_OBJDIR)/execinfo.cmd:" << newLine
            << "\t-$(V_AT)mkdir -p $(@D)" << newLine
            << "\t-@if [ -z \"$(V_AT)\" ]; then echo \"Checking if we need to link libexecinfo\"; fi" << newLine
            << "\t$(V_AT)printf \"int main() { return 0; }\" | $(CXX) -x c++ -o $(@D)/execinfo.x -lexecinfo - >/dev/null 2>&1 && printf -- \"-lexecinfo\" > \"$@\" || touch \"$@\"" << newLine
            << newLine;

        if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
            writeSubprocessHelperTargets (out);

        out << "clean:"                           << newLine
            << "\t@echo Cleaning " << projectName << newLine
            << "\t$(V_AT)$(CLEANCMD)"             << newLine
            << newLine;

        out << "strip:"                                                       << newLine
            << "\t@echo Stripping " << projectName                            << newLine
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

    String getObjectFileFor (const build_tools::RelativePath& file) const
    {
        return file.getFileNameWithoutExtension()
                + "_" + String::toHexString (file.toUnixStyle().hashCode()) + ".o";
    }

    String getPhonyTargetLine() const
    {
        MemoryOutputStream phonyTargetLine;

        phonyTargetLine << ".PHONY: clean all strip";

        if (! getProject().isAudioPluginProject())
            return phonyTargetLine.toString();

        for (auto target : targets)
            if (target->type != build_tools::ProjectType::Target::SharedCodeTarget
                && target->type != build_tools::ProjectType::Target::AggregateTarget)
                phonyTargetLine << " " << target->getPhonyName();

        return phonyTargetLine.toString();
    }

    OwnedArray<MakefileTarget> targets;

    JUCE_DECLARE_NON_COPYABLE (MakefileProjectExporter)
};
