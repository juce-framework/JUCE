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

#include "jucer_ProjectExport_MSVC.h"

//==============================================================================
class CodeBlocksProjectExporter  : public ProjectExporter
{
public:
    enum CodeBlocksOS
    {
        windowsTarget,
        linuxTarget
    };

    //==============================================================================
    static String getDisplayNameWindows()        { return "[Deprecated] Code::Blocks (Windows)"; }
    static String getDisplayNameLinux()          { return "[Deprecated] Code::Blocks (Linux)"; }

    static String getValueTreeTypeNameWindows()  { return "CODEBLOCKS_WINDOWS"; }
    static String getValueTreeTypeNameLinux()    { return "CODEBLOCKS_LINUX"; }

    static String getTargetFolderNameWindows()   { return "CodeBlocksWindows"; }
    static String getTargetFolderNameLinux()     { return "CodeBlocksLinux"; }

    //==============================================================================
    static CodeBlocksProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        // this will also import legacy jucer files where CodeBlocks only worked for Windows,
        // had valueTreetTypeName "CODEBLOCKS", and there was no OS distinction
        if (settingsToUse.hasType (getValueTreeTypeNameWindows()) || settingsToUse.hasType ("CODEBLOCKS"))
            return new CodeBlocksProjectExporter (projectToUse, settingsToUse, windowsTarget);

        if (settingsToUse.hasType (getValueTreeTypeNameLinux()))
            return new CodeBlocksProjectExporter (projectToUse, settingsToUse, linuxTarget);

        return nullptr;
    }

    //==============================================================================
    CodeBlocksProjectExporter (Project& p, const ValueTree& t, CodeBlocksOS codeBlocksOs)
        : ProjectExporter (p, t), os (codeBlocksOs)
    {
        if (isWindows())
        {
            name = getDisplayNameWindows();
            targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderNameWindows());
            targetPlatformValue.referTo (settings, Ids::codeBlocksWindowsTarget, getUndoManager());
        }
        else
        {
            name = getDisplayNameLinux();
            targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderNameLinux());
        }
    }

    //==============================================================================
    bool canLaunchProject() override                 { return false; }
    bool launchProject() override                    { return false; }
    bool usesMMFiles() const override                { return false; }
    bool canCopeWithDuplicateFiles() override        { return false; }
    bool supportsUserDefinedConfigurations() const override { return true; }

    bool isXcode() const override                    { return false; }
    bool isVisualStudio() const override             { return false; }
    bool isCodeBlocks() const override               { return true; }
    bool isMakefile() const override                 { return false; }
    bool isAndroidStudio() const override            { return false; }

    bool isAndroid() const override                  { return false; }
    bool isWindows() const override                  { return os == windowsTarget; }
    bool isLinux() const override                    { return os == linuxTarget; }
    bool isOSX() const override                      { return false; }
    bool isiOS() const override                      { return false; }

    Identifier getExporterIdentifier() const override
    {
        return isLinux() ? getValueTreeTypeNameLinux() : getValueTreeTypeNameWindows();
    }

    String getNewLineString() const override         { return isWindows() ? "\r\n" : "\n"; }

    bool supportsTargetType (build_tools::ProjectType::Target::Type type) const override
    {
        using Target = build_tools::ProjectType::Target;

        switch (type)
        {
            case Target::StandalonePlugIn:
            case Target::GUIApp:
            case Target::ConsoleApp:
            case Target::StaticLibrary:
            case Target::SharedCodeTarget:
            case Target::AggregateTarget:
            case Target::VSTPlugIn:
            case Target::DynamicLibrary:
                return true;
            case Target::AAXPlugIn:
            case Target::UnityPlugIn:
            case Target::LV2PlugIn:
            case Target::LV2Helper:
            case Target::VST3PlugIn:
            case Target::VST3Helper:
            case Target::AudioUnitPlugIn:
            case Target::AudioUnitv3PlugIn:
            case Target::unspecified:
            default:
                break;
        }

        return false;
    }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        if (isWindows())
        {
            props.add (new ChoicePropertyComponent (targetPlatformValue, "Target platform",
                                                    { "Windows Vista", "Windows Server 2008", "Windows 7", "Windows 8", "Windows 8.1", "Windows 10" },
                                                    { "0x0600",        "0x0600",              "0x0601",    "0x0602",    "0x0603",      "0x0A00" }),
                       "This sets the preprocessor macro WINVER to an appropriate value for the corresponding platform.");
        }
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        auto cbpFile = getTargetFolder().getChildFile (project.getProjectFilenameRootString())
                                        .withFileExtension (".cbp");

        XmlElement xml ("CodeBlocks_project_file");
        addVersion (xml);
        createProject (*xml.createNewChildElement ("Project"));
        writeXmlOrThrow (xml, cbpFile, "UTF-8", 10, true);

        linuxSubprocessHelperProperties.deployLinuxSubprocessHelperSourceFilesIfNecessary();
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const build_tools::ProjectType&) override
    {
        linuxSubprocessHelperProperties.addToExtraSearchPathsIfNecessary();

        // add shared code target first as order matters for Codeblocks
        if (shouldBuildTargetType (build_tools::ProjectType::Target::SharedCodeTarget))
            targets.add (new CodeBlocksTarget (*this, build_tools::ProjectType::Target::SharedCodeTarget));

        //resource::ProjectType::Target::SharedCodeTarget
        callForAllSupportedTargets ([this] (build_tools::ProjectType::Target::Type targetType)
                                    {
                                        if (targetType == build_tools::ProjectType::Target::SharedCodeTarget)
                                            return;

                                        targets.insert (targetType == build_tools::ProjectType::Target::AggregateTarget ? 0 : -1,
                                                        new CodeBlocksTarget (*this, targetType));
                                    });

        // If you hit this assert, you tried to generate a project for an exporter
        // that does not support any of your targets!
        jassert (targets.size() > 0);
    }

    void initialiseDependencyPathValues() override
    {
        auto targetOS = isWindows() ? TargetOS::windows : TargetOS::linux;

        vstLegacyPathValueWrapper.init ({ settings, Ids::vstLegacyFolder, nullptr },
                                        getAppSettings().getStoredPath (Ids::vstLegacyPath, targetOS), targetOS);
    }

private:
    ValueTreePropertyWithDefault targetPlatformValue;

    String getTargetPlatformString() const    { return targetPlatformValue.get(); }

    //==============================================================================
    class CodeBlocksBuildConfiguration  : public BuildConfiguration
    {
    public:
        CodeBlocksBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e),
              architectureTypeValue (config, exporter.isWindows() ? Ids::windowsCodeBlocksArchitecture
                                                                  : Ids::linuxCodeBlocksArchitecture, getUndoManager(), "-m64")
        {
            linkTimeOptimisationValue.setDefault (false);
            optimisationLevelValue.setDefault (isDebug() ? gccO0 : gccO3);
        }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addRecommendedLinuxCompilerWarningsProperty (props);
            addGCCOptimisationProperty (props);

            props.add (new ChoicePropertyComponent (architectureTypeValue, "Architecture",
                                                    { "32-bit (-m32)", "64-bit (-m64)", "ARM v6",       "ARM v7" },
                                                    { "-m32",          "-m64",          "-march=armv6", "-march=armv7" }),
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

            jassertfalse;
            return {};
        }

        String getArchitectureTypeString() const    { return architectureTypeValue.get(); }

        //==============================================================================
        ValueTreePropertyWithDefault architectureTypeValue;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return *new CodeBlocksBuildConfiguration (project, tree, *this);
    }

    //==============================================================================
    enum class IsDynamicLibrary
    {
        no,
        yes
    };

    enum class ShouldBeCompiled
    {
        no,
        yes
    };

    class CodeBlocksTarget : public build_tools::ProjectType::Target
    {
    public:
        CodeBlocksTarget (const CodeBlocksProjectExporter& e,
                          build_tools::ProjectType::Target::Type typeToUse)
            : Target (typeToUse),
              exporter (e)
        {}

        String getTargetNameForConfiguration (const BuildConfiguration& config) const
        {
            if (type == build_tools::ProjectType::Target::AggregateTarget)
                return config.getName();

            return getName() + String (" | ") + config.getName();
        }

        String getTargetSuffix() const
        {
            auto fileType = getTargetFileType();

            if (exporter.isWindows())
            {
                switch (fileType)
                {
                    case executable:            return ".exe";
                    case staticLibrary:         return ".lib";
                    case sharedLibraryOrDLL:
                    case pluginBundle:          return ".dll";
                    case macOSAppex:
                    case unknown:
                    default:
                        break;
                }
            }
            else
            {
                switch (fileType)
                {
                    case executable:            return {};
                    case staticLibrary:         return ".a";
                    case pluginBundle:
                    case sharedLibraryOrDLL:    return ".so";
                    case macOSAppex:
                    case unknown:
                    default:
                        break;
                }
            }

            return {};
        }

        IsDynamicLibrary isDynamicLibrary() const
        {
            return (type == DynamicLibrary || type == VSTPlugIn) ? IsDynamicLibrary::yes
                                                                 : IsDynamicLibrary::no;
        }

        const CodeBlocksProjectExporter& exporter;
    };

    //==============================================================================
    class LinuxSubprocessHelperTarget
    {
    public:
        enum class HelperType
        {
            linuxSubprocessHelper,
            simpleBinaryBuilder
        };

        LinuxSubprocessHelperTarget (const CodeBlocksProjectExporter& exporter, HelperType helperTypeIn)
            : owner (exporter),
              helperType (helperTypeIn)
        {
        }

        String getTargetNameForConfiguration (const BuildConfiguration& config) const
        {
            return getName (helperType) + " | " + config.getName();
        }

        void addTarget (XmlElement& xml, const BuildConfiguration& config) const
        {
            xml.setAttribute ("title", getTargetNameForConfiguration (config));
            auto* output = xml.createNewChildElement ("Option");
            output->setAttribute ("output", getOutput (helperType, config));
            xml.createNewChildElement ("Option")->setAttribute ("object_output",
                                                                "obj/" + File::createLegalFileName (config.getName().trim()));
            xml.createNewChildElement ("Option")->setAttribute ("type", getTypeIndex (type));
            xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");

            const auto isDynamicLibrary = IsDynamicLibrary::no;

            {
                auto* compiler = xml.createNewChildElement ("Compiler");

                for (auto flag : owner.getCompilerFlags (config, isDynamicLibrary))
                    owner.setAddOption (*compiler, "option", flag);
            }

            {
                auto* linker = xml.createNewChildElement ("Linker");

                for (auto& flag : owner.getLinkerFlags (config, isDynamicLibrary))
                    owner.setAddOption (*linker, "option", flag);
            }

            if (helperType == HelperType::simpleBinaryBuilder)
            {
                auto* postBuildCommands = xml.createNewChildElement ("ExtraCommands");

                const auto binaryDataSource = owner.linuxSubprocessHelperProperties
                                                   .getLinuxSubprocessHelperBinaryDataSource();

                owner.setAddOption (*postBuildCommands,
                                    "after",
                                    getOutput (HelperType::simpleBinaryBuilder, config)
                                        + " " + getOutput (HelperType::linuxSubprocessHelper, config)
                                        + " pre_build"
                                        + " " + binaryDataSource.getFileNameWithoutExtension().quoted()
                                        + " LinuxSubprocessHelperBinaryData");
            }
        }

        void addCompileUnits (XmlElement& xml) const
        {
            const auto file = getSource (helperType);
            auto* unit = xml.createNewChildElement ("Unit");
            unit->setAttribute ("filename", file.toUnixStyle());

            for (ConstConfigIterator config (owner); config.next();)
            {
                auto targetName = getTargetNameForConfiguration (*config);
                unit->createNewChildElement ("Option")->setAttribute ("target", targetName);
            }
        }

    private:
        build_tools::RelativePath getSource (HelperType helperTypeForSource) const
        {
            if (helperTypeForSource == HelperType::linuxSubprocessHelper)
                return owner.linuxSubprocessHelperProperties.getLinuxSubprocessHelperSource();

            return owner.linuxSubprocessHelperProperties.getSimpleBinaryBuilderSource();
        }

        String getName (HelperType helperTypeForName) const
        {
            return LinuxSubprocessHelperProperties::getBinaryNameFromSource (getSource (helperTypeForName));
        }

        String getOutput (HelperType helperTypeForOutput, const BuildConfiguration& config) const
        {
            return owner.getOutputPathForConfig (config) + "/" + getName (helperTypeForOutput);
        }

        const CodeBlocksProjectExporter& owner;
        HelperType helperType;
        build_tools::ProjectType::Target::Type type = build_tools::ProjectType::Target::ConsoleApp;
    };

    void addSubprocessHelperBinarySourceCompileUnit (XmlElement& xml) const
    {
        auto* unit = xml.createNewChildElement ("Unit");
        const auto binaryDataSource = linuxSubprocessHelperProperties.getLinuxSubprocessHelperBinaryDataSource();
        unit->setAttribute ("filename", binaryDataSource.toUnixStyle());

        for (ConstConfigIterator config (*this); config.next();)
        {
            const auto& target = getTargetForFile (resolveRelativePath (binaryDataSource), ShouldBeCompiled::yes);
            const auto targetName = target.getTargetNameForConfiguration (*config);
            unit->createNewChildElement ("Option")->setAttribute ("target", targetName);
        }
    }

    //==============================================================================
    void addVersion (XmlElement& xml) const
    {
        auto* fileVersion = xml.createNewChildElement ("FileVersion");
        fileVersion->setAttribute ("major", 1);
        fileVersion->setAttribute ("minor", 6);
    }

    void addOptions (XmlElement& xml) const
    {
        xml.createNewChildElement ("Option")->setAttribute ("title", project.getProjectNameString());
        xml.createNewChildElement ("Option")->setAttribute ("pch_mode", 2);
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");
    }

    StringArray getDefines (const BuildConfiguration& config, CodeBlocksTarget& target) const
    {
        StringPairArray defines;

        if (isWindows())
        {
            defines.set ("__MINGW__", "1");
            defines.set ("__MINGW_EXTENSION", {});

            auto targetPlatform = getTargetPlatformString();

            if (targetPlatform.isNotEmpty())
                defines.set ("WINVER", targetPlatform);
        }
        else
        {
            defines.set ("LINUX", "1");
        }

        if (config.isDebug())
        {
            defines.set ("DEBUG", "1");
            defines.set ("_DEBUG", "1");
        }
        else
        {
            defines.set ("NDEBUG", "1");
        }

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config, target.type));

        StringArray defs;
        auto keys = defines.getAllKeys();
        auto values = defines.getAllValues();

        const auto escapedQuote = isWindows() ? "\\\"" : "\\\\\"";

        for (int i = 0; i < defines.size(); ++i)
        {
            auto result = keys[i];

            if (values[i].isNotEmpty())
                result += "=\"" + values[i].replace ("\"", escapedQuote) + "\"";

            defs.add (result);
        }

        return getCleanedStringArray (defs);
    }

    StringArray getCompilerFlags (const BuildConfiguration& config, IsDynamicLibrary isDynamicLibrary) const
    {
        StringArray flags;

        if (auto* codeBlocksConfig = dynamic_cast<const CodeBlocksBuildConfiguration*> (&config))
            flags.add (codeBlocksConfig->getArchitectureTypeString());

        auto recommendedFlags = config.getRecommendedCompilerWarningFlags();

        for (auto& recommendedFlagsType : { recommendedFlags.common, recommendedFlags.cpp })
            for (auto& recommended : recommendedFlagsType)
                flags.add (recommended);

        flags.add ("-O" + config.getGCCOptimisationFlag());

        if (config.isLinkTimeOptimisationEnabled())
            flags.add ("-flto");

        {
            auto cppStandard = config.project.getCppStandardString();

            if (cppStandard == "latest")
                cppStandard = project.getLatestNumberedCppStandardString();

            flags.add ("-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard);
        }

        flags.add ("-mstackrealign");

        if (config.isDebug())
            flags.add ("-g");

        flags.addTokens (replacePreprocessorTokens (config, config.getAllCompilerFlagsString()).trim(),
                         " \n", "\"'");

        if (config.exporter.isLinux())
        {
            if (isDynamicLibrary == IsDynamicLibrary::yes || getProject().isAudioPluginProject())
                flags.add ("-fPIC");

            auto packages = config.exporter.getLinuxPackages (PackageDependencyType::compile);

            if (! packages.isEmpty())
            {
                auto pkgconfigFlags = String ("`pkg-config --cflags");

                for (auto& p : packages)
                    pkgconfigFlags << " " << p;

                pkgconfigFlags << "`";
                flags.add (pkgconfigFlags);
            }

            if (linuxLibs.contains ("pthread"))
                flags.add ("-pthread");
        }

        return getCleanedStringArray (flags);
    }

    StringArray getLinkerFlags (const BuildConfiguration& config, IsDynamicLibrary isDynamicLibrary) const
    {
        auto flags = makefileExtraLinkerFlags;

        if (auto* codeBlocksConfig = dynamic_cast<const CodeBlocksBuildConfiguration*> (&config))
            flags.add (codeBlocksConfig->getArchitectureTypeString());

        if (! config.isDebug())
            flags.add ("-s");

        if (config.isLinkTimeOptimisationEnabled())
            flags.add ("-flto");

        flags.addTokens (replacePreprocessorTokens (config, config.getAllLinkerFlagsString()).trim(), " \n", "\"'");

        if (config.exporter.isLinux())
        {
            if (isDynamicLibrary == IsDynamicLibrary::yes)
                flags.add ("-shared");

            auto packages = config.exporter.getLinuxPackages (PackageDependencyType::link);

            if (! packages.isEmpty())
            {
                String pkgconfigLibs ("`pkg-config --libs");

                for (auto& p : packages)
                    pkgconfigLibs << " " << p;

                pkgconfigLibs << "`";
                flags.add (pkgconfigLibs);
            }
        }

        return getCleanedStringArray (flags);
    }

    StringArray getLinkerSearchPaths (const BuildConfiguration& config, CodeBlocksTarget& target) const
    {
        auto librarySearchPaths = config.getLibrarySearchPaths();

        if (getProject().isAudioPluginProject() && target.type != build_tools::ProjectType::Target::SharedCodeTarget)
            librarySearchPaths.add (build_tools::RelativePath (getSharedCodePath (config), build_tools::RelativePath::buildTargetFolder).getParentDirectory().toUnixStyle().quoted());

        return librarySearchPaths;
    }

    StringArray getIncludePaths (const BuildConfiguration& config) const
    {
        StringArray paths;

        paths.add (".");
        paths.addArray (extraSearchPaths);
        paths.addArray (config.getHeaderSearchPaths());

        if (! isWindows())
        {
            paths.add ("/usr/include/freetype2");

            // Replace ~ character with $(HOME) environment variable
            for (auto& path : paths)
                path = path.replace ("~", "$(HOME)");
        }

        return getCleanedStringArray (paths);
    }

    static int getTypeIndex (const build_tools::ProjectType::Target::Type& type)
    {
        if (type == build_tools::ProjectType::Target::GUIApp || type == build_tools::ProjectType::Target::StandalonePlugIn)         return 0;
        if (type == build_tools::ProjectType::Target::ConsoleApp)                                                                   return 1;
        if (type == build_tools::ProjectType::Target::StaticLibrary || type == build_tools::ProjectType::Target::SharedCodeTarget)  return 2;
        if (type == build_tools::ProjectType::Target::DynamicLibrary || type == build_tools::ProjectType::Target::VSTPlugIn)        return 3;

        return 0;
    }

    String getOutputPathForConfig (const BuildConfiguration& config) const
    {
        if (config.getTargetBinaryRelativePathString().isNotEmpty())
        {
            build_tools::RelativePath binaryPath (config.getTargetBinaryRelativePathString(), build_tools::RelativePath::projectFolder);
            binaryPath = binaryPath.rebased (projectFolder, getTargetFolder(), build_tools::RelativePath::buildTargetFolder);
            return config.getTargetBinaryRelativePathString();
        }

        return "bin/" + File::createLegalFileName (config.getName().trim());
    }

    String getOutputPathForTarget (CodeBlocksTarget& target, const BuildConfiguration& config) const
    {
        return   getOutputPathForConfig (config)
               + "/"
               + replacePreprocessorTokens (config, config.getTargetBinaryNameString() + target.getTargetSuffix());
    }

    String getSharedCodePath (const BuildConfiguration& config) const
    {
        auto outputPath = getOutputPathForTarget (getTargetWithType (build_tools::ProjectType::Target::SharedCodeTarget), config);
        build_tools::RelativePath path (outputPath, build_tools::RelativePath::buildTargetFolder);
        auto filename = path.getFileName();

        if (isLinux())
            filename = "lib" + filename;

        return path.getParentDirectory().getChildFile (filename).toUnixStyle();
    }

    void createBuildTarget (XmlElement& xml, CodeBlocksTarget& target, const BuildConfiguration& config) const
    {
        xml.setAttribute ("title", target.getTargetNameForConfiguration (config));

        {
            auto* output = xml.createNewChildElement ("Option");

            output->setAttribute ("output", getOutputPathForTarget (target, config));

            if (isLinux())
            {
                bool keepPrefix = (target.type == build_tools::ProjectType::Target::VSTPlugIn);

                output->setAttribute ("prefix_auto", keepPrefix ? 0 : 1);
            }
            else
            {
                output->setAttribute ("prefix_auto", 0);
            }

            output->setAttribute ("extension_auto", 0);
        }

        xml.createNewChildElement ("Option")
             ->setAttribute ("object_output", "obj/" + File::createLegalFileName (config.getName().trim()));

        xml.createNewChildElement ("Option")->setAttribute ("type", getTypeIndex (target.type));
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");

        if (getProject().isAudioPluginProject() && target.type != build_tools::ProjectType::Target::SharedCodeTarget)
            xml.createNewChildElement ("Option")->setAttribute ("external_deps", getSharedCodePath (config));

        {
            auto* compiler = xml.createNewChildElement ("Compiler");

            {
                StringArray flags;

                for (auto& def : getDefines (config, target))
                {
                    if (! def.containsChar ('='))
                        def << '=';

                    flags.add ("-D" + def);
                }

                flags.addArray (getCompilerFlags (config, target.isDynamicLibrary()));

                for (auto flag : flags)
                    setAddOption (*compiler, "option", flag);
            }

            {
                auto includePaths = getIncludePaths (config);

                for (auto path : includePaths)
                    setAddOption (*compiler, "directory", path);
            }
        }

        {
            auto* linker = xml.createNewChildElement ("Linker");

            if (getProject().isAudioPluginProject() && target.type != build_tools::ProjectType::Target::SharedCodeTarget)
                setAddOption (*linker, "option", getSharedCodePath (config).quoted());

            for (auto& flag : getLinkerFlags (config, target.isDynamicLibrary()))
                setAddOption (*linker, "option", flag);

            const StringArray& libs = isWindows() ? mingwLibs : linuxLibs;

            for (auto lib : libs)
                setAddOption (*linker, "library", lib);

            for (auto& path : getLinkerSearchPaths (config, target))
                setAddOption (*linker, "directory",
                              build_tools::replacePreprocessorDefs (getAllPreprocessorDefs(), path));
        }
    }

    void addBuild (XmlElement& xml) const
    {
        auto* build = xml.createNewChildElement ("Build");

        for (ConstConfigIterator config (*this); config.next();)
        {
            if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
                for (const auto& helperTarget : helperTargets)
                    helperTarget.addTarget (*build->createNewChildElement ("Target"), *config);

            for (auto target : targets)
                if (target->type != build_tools::ProjectType::Target::AggregateTarget)
                    createBuildTarget (*build->createNewChildElement ("Target"), *target, *config);
        }
    }

    void addVirtualTargets (XmlElement& xml) const
    {
        auto* virtualTargets = xml.createNewChildElement ("VirtualTargets");

        for (ConstConfigIterator config (*this); config.next();)
        {
            StringArray allTargets;

            if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
                for (const auto& target : helperTargets)
                    allTargets.add (target.getTargetNameForConfiguration (*config));

            for (auto target : targets)
                if (target->type != build_tools::ProjectType::Target::AggregateTarget)
                    allTargets.add (target->getTargetNameForConfiguration (*config));

            for (auto target : targets)
            {
                if (target->type == build_tools::ProjectType::Target::AggregateTarget)
                {
                    auto* configTarget = virtualTargets->createNewChildElement ("Add");

                    configTarget->setAttribute ("alias", config->getName());
                    configTarget->setAttribute ("targets", allTargets.joinIntoString (";"));
                }
            }
        }
    }

    StringArray getProjectCompilerOptions() const
    {
        return { "-Wall", "-Wno-strict-aliasing", "-Wno-strict-overflow" };
    }

    void addProjectCompilerOptions (XmlElement& xml) const
    {
        auto* compiler = xml.createNewChildElement ("Compiler");

        for (auto& option : getProjectCompilerOptions())
            setAddOption (*compiler, "option", option);
    }

    StringArray getProjectLinkerLibs() const
    {
        StringArray result;

        if (isWindows())
            result.addArray ({ "gdi32", "user32", "kernel32", "comctl32" });

        result.addTokens (getExternalLibrariesString(), ";\n", "\"'");

        result = getCleanedStringArray (result);

        for (auto& option : result)
            option = build_tools::replacePreprocessorDefs (getAllPreprocessorDefs(), option);

        return result;
    }

    void addProjectLinkerOptions (XmlElement& xml) const
    {
        auto* linker = xml.createNewChildElement ("Linker");

        for (auto& lib : getProjectLinkerLibs())
            setAddOption (*linker, "library", lib);
    }

    CodeBlocksTarget& getTargetWithType (build_tools::ProjectType::Target::Type type) const
    {
        CodeBlocksTarget* nonAggregrateTarget = nullptr;

        for (auto* target : targets)
        {
            if (target->type == type)
                return *target;

            if (target->type != build_tools::ProjectType::Target::AggregateTarget)
                nonAggregrateTarget = target;
        }

        // this project has no valid targets
        jassert (nonAggregrateTarget != nullptr);

        return *nonAggregrateTarget;
    }

    // Returns SharedCode target for multi-target projects, otherwise it returns
    // the single target
    CodeBlocksTarget& getMainTarget() const
    {
        if (getProject().isAudioPluginProject())
            return getTargetWithType (build_tools::ProjectType::Target::SharedCodeTarget);

        for (auto* target : targets)
            if (target->type != build_tools::ProjectType::Target::AggregateTarget)
                return *target;

        jassertfalse;

        return *targets[0];
    }

    CodeBlocksTarget& getTargetForFile (const File& file, ShouldBeCompiled shouldBeCompiled) const
    {
        if (getProject().isAudioPluginProject())
        {
            if (shouldBeCompiled == ShouldBeCompiled::no)
                return getTargetWithType (build_tools::ProjectType::Target::SharedCodeTarget);

            return getTargetWithType (getProject().getTargetTypeFromFilePath (file, true));
        }

        return getMainTarget();
    }

    CodeBlocksTarget& getTargetForProjectItem (const Project::Item& projectItem) const
    {
        return getTargetForFile (projectItem.getFile(),
                                 projectItem.shouldBeCompiled() ? ShouldBeCompiled::yes
                                                                : ShouldBeCompiled::no);
    }

    void addCompileUnits (const Project::Item& projectItem, XmlElement& xml) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addCompileUnits (projectItem.getChild(i), xml);
        }
        else if (projectItem.shouldBeAddedToTargetProject() && projectItem.shouldBeAddedToTargetExporter (*this))
        {
            build_tools::RelativePath file (projectItem.getFile(), getTargetFolder(), build_tools::RelativePath::buildTargetFolder);

            auto* unit = xml.createNewChildElement ("Unit");
            unit->setAttribute ("filename", file.toUnixStyle());

            for (ConstConfigIterator config (*this); config.next();)
            {
                auto targetName = getTargetForProjectItem (projectItem).getTargetNameForConfiguration (*config);
                unit->createNewChildElement ("Option")->setAttribute ("target", targetName);
            }

            if (projectItem.shouldBeCompiled())
            {
                auto extraCompilerFlags = getCompilerFlagsForProjectItem (projectItem);

                if (extraCompilerFlags.isNotEmpty())
                {
                    auto* optionElement = unit->createNewChildElement ("Option");

                    optionElement->setAttribute ("compiler",     "gcc");
                    optionElement->setAttribute ("use",          1);
                    optionElement->setAttribute ("buildCommand", "$compiler $options " + extraCompilerFlags + " $includes -c $file  -o $object");
                }
            }
            else
            {
                unit->createNewChildElement ("Option")->setAttribute ("compile", 0);
                unit->createNewChildElement ("Option")->setAttribute ("link", 0);
            }
        }
    }

    bool hasResourceFile() const
    {
        return ! projectType.isStaticLibrary();
    }

    void addCompileUnits (XmlElement& xml) const
    {
        if (linuxSubprocessHelperProperties.shouldUseLinuxSubprocessHelper())
        {
            for (const auto& helperTarget : helperTargets)
                helperTarget.addCompileUnits (xml);

            addSubprocessHelperBinarySourceCompileUnit (xml);
        }

        for (int i = 0; i < getAllGroups().size(); ++i)
            addCompileUnits (getAllGroups().getReference(i), xml);

        if (hasResourceFile())
        {
            const auto iconFile = getTargetFolder().getChildFile ("icon.ico");

            if (! build_tools::asArray (getIcons()).isEmpty())
                build_tools::writeWinIcon (getIcons(), iconFile);

            auto rcFile = getTargetFolder().getChildFile ("resources.rc");
            MSVCProjectExporterBase::createRCFile (project, iconFile, rcFile);

            auto* unit = xml.createNewChildElement ("Unit");
            unit->setAttribute ("filename", rcFile.getFileName());
            unit->createNewChildElement ("Option")->setAttribute ("compilerVar", "WINDRES");
        }
    }

    void createProject (XmlElement& xml) const
    {
        addOptions (xml);
        addBuild (xml);
        addVirtualTargets (xml);
        addProjectCompilerOptions (xml);
        addProjectLinkerOptions (xml);
        addCompileUnits (xml);
    }

    void setAddOption (XmlElement& xml, const String& nm, const String& value) const
    {
        xml.createNewChildElement ("Add")->setAttribute (nm, value);
    }

    CodeBlocksOS os;

    OwnedArray<CodeBlocksTarget> targets;

    // The order of these targets is significant, as latter targets depend on earlier ones
    const LinuxSubprocessHelperTarget helperTargets[2] { { *this, LinuxSubprocessHelperTarget::HelperType::linuxSubprocessHelper },
                                                         { *this, LinuxSubprocessHelperTarget::HelperType::simpleBinaryBuilder   } };

    JUCE_DECLARE_NON_COPYABLE (CodeBlocksProjectExporter)
};
