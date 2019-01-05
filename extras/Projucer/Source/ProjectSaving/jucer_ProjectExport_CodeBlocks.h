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
class CodeBlocksProjectExporter  : public ProjectExporter
{
public:
    enum CodeBlocksOS
    {
        windowsTarget,
        linuxTarget
    };

    //==============================================================================
    static const char* getNameWindows() noexcept    { return "Code::Blocks (Windows)"; }
    static const char* getNameLinux() noexcept      { return "Code::Blocks (Linux)"; }

    static const char* getName (CodeBlocksOS os) noexcept
    {
        if (os == windowsTarget) return getNameWindows();
        if (os == linuxTarget)   return getNameLinux();

        // currently no other OSes supported by Codeblocks exporter!
        jassertfalse;
        return "Code::Blocks (Unknown OS)";
    }

    //==============================================================================
    static const char* getValueTreeTypeName (CodeBlocksOS os)
    {
        if (os == windowsTarget)  return "CODEBLOCKS_WINDOWS";
        if (os == linuxTarget)    return "CODEBLOCKS_LINUX";

        // currently no other OSes supported by Codeblocks exporter!
        jassertfalse;
        return "CODEBLOCKS_UNKNOWN_OS";
    }

    //==============================================================================
    static CodeBlocksProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        // this will also import legacy jucer files where CodeBlocks only worked for Windows,
        // had valueTreetTypeName "CODEBLOCKS", and there was no OS distinction
        if (settings.hasType (getValueTreeTypeName (windowsTarget)) || settings.hasType ("CODEBLOCKS"))
            return new CodeBlocksProjectExporter (project, settings, windowsTarget);

        if (settings.hasType (getValueTreeTypeName (linuxTarget)))
            return new CodeBlocksProjectExporter (project, settings, linuxTarget);

        return nullptr;
    }

    //==============================================================================
    CodeBlocksProjectExporter (Project& p, const ValueTree& t, CodeBlocksOS codeBlocksOs)
        : ProjectExporter (p, t), os (codeBlocksOs)
    {
        name = getName (os);

        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderForExporter (getValueTreeTypeName (os)));

        if (isWindows())
            targetPlatformValue.referTo (settings, Ids::codeBlocksWindowsTarget, getUndoManager());
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
    bool isCLion() const override                    { return false; }

    bool isAndroid() const override                  { return false; }
    bool isWindows() const override                  { return os == windowsTarget; }
    bool isLinux() const override                    { return os == linuxTarget; }
    bool isOSX() const override                      { return false; }
    bool isiOS() const override                      { return false; }

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
            case ProjectType::Target::DynamicLibrary:
                return true;
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
                                                    { "Windows NT 4.0", "Windows 2000", "Windows XP", "Windows Server 2003", "Windows Vista", "Windows Server 2008",
                                                      "Windows 7", "Windows 8", "Windows 8.1", "Windows 10" },
                                                    { "0x0400", "0x0500", "0x0501", "0x0502", "0x0600", "0x0600",
                                                      "0x0601", "0x0602", "0x0603", "0x0A00" }),
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
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override
    {
        // add shared code target first as order matters for Codeblocks
        if (shouldBuildTargetType (ProjectType::Target::SharedCodeTarget))
            targets.add (new CodeBlocksTarget (*this, ProjectType::Target::SharedCodeTarget));

        //ProjectType::Target::SharedCodeTarget
        callForAllSupportedTargets ([this] (ProjectType::Target::Type targetType)
                                    {
                                        if (targetType == ProjectType::Target::SharedCodeTarget)
                                            return;

                                        if (auto* target = new CodeBlocksTarget (*this, targetType))
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
    ValueWithDefault targetPlatformValue;

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
            else if (archFlag == "-m64")
                return "x86_64";
            else if (archFlag == "-m32")
                return "i386";

            jassertfalse;
            return {};
        }

        String getArchitectureTypeString() const    { return architectureTypeValue.get(); }

        //==============================================================================
        ValueWithDefault architectureTypeValue;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return *new CodeBlocksBuildConfiguration (project, tree, *this);
    }

    //==============================================================================
    class CodeBlocksTarget : public ProjectType::Target
    {
    public:
        CodeBlocksTarget (const CodeBlocksProjectExporter& e, ProjectType::Target::Type typeToUse)
            : ProjectType::Target (typeToUse),
              exporter (e)
        {}

        String getTargetNameForConfiguration (const BuildConfiguration& config) const
        {
            if (type == ProjectType::Target::AggregateTarget)
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
                    case sharedLibraryOrDLL:    return ".so";

                    case pluginBundle:
                        switch (type)
                        {
                            case VSTPlugIn:     return ".so";
                            default:            break;
                        }

                        return ".so";

                    default:
                        break;
                }
            }

            return {};
        }

        bool isDynamicLibrary() const
        {
            return (type == DynamicLibrary || type == VSTPlugIn);
        }

        const CodeBlocksProjectExporter& exporter;
    };

    //==============================================================================

    StringArray getPackages() const
    {
        auto result = linuxPackages;

        if (project.getEnabledModules().isModuleEnabled ("juce_gui_extra")
            && project.isConfigFlagEnabled ("JUCE_WEB_BROWSER", true))
        {
            result.add ("webkit2gtk-4.0");
            result.add ("gtk+-x11-3.0");
        }

        if (project.getEnabledModules().isModuleEnabled ("juce_core")
            && ! project.isConfigFlagEnabled ("JUCE_LOAD_CURL_SYMBOLS_LAZILY", false))
            result.add ("libcurl");

        result.removeDuplicates (false);

        return result;
    }

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

        for (int i = 0; i < defines.size(); ++i)
        {
            auto result = keys[i];

            if (values[i].isNotEmpty())
                result += "=" + values[i];

            defs.add (result);
        }

        return getCleanedStringArray (defs);
    }

    StringArray getCompilerFlags (const BuildConfiguration& config, CodeBlocksTarget& target) const
    {
        StringArray flags;
        if (auto* codeBlocksConfig = dynamic_cast<const CodeBlocksBuildConfiguration*> (&config))
            flags.add (codeBlocksConfig->getArchitectureTypeString());

        flags.add ("-O" + config.getGCCOptimisationFlag());

        if (config.isLinkTimeOptimisationEnabled())
            flags.add ("-flto");

        {
            auto cppStandard = config.project.getCppStandardString();

            if (cppStandard == "latest")
                cppStandard = "17";

            cppStandard = "-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard;

            flags.add (cppStandard);
        }

        flags.add ("-mstackrealign");

        if (config.isDebug())
            flags.add ("-g");

        flags.addTokens (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim(),
                         " \n", "\"'");

        if (config.exporter.isLinux())
        {
            if (target.isDynamicLibrary() || getProject().getProjectType().isAudioPlugin())
                flags.add ("-fPIC");

            auto packages = getPackages();

            if (packages.size() > 0)
            {
                auto pkgconfigFlags = String ("`pkg-config --cflags");
                for (auto p : packages)
                    pkgconfigFlags << " " << p;

                pkgconfigFlags << "`";
                flags.add (pkgconfigFlags);
            }

            if (linuxLibs.contains("pthread"))
                flags.add ("-pthread");
        }

        return getCleanedStringArray (flags);
    }

    StringArray getLinkerFlags (const BuildConfiguration& config, CodeBlocksTarget& target) const
    {
        auto flags = makefileExtraLinkerFlags;

        if (auto* codeBlocksConfig = dynamic_cast<const CodeBlocksBuildConfiguration*> (&config))
            flags.add (codeBlocksConfig->getArchitectureTypeString());

        if (! config.isDebug())
            flags.add ("-s");

        if (config.isLinkTimeOptimisationEnabled())
            flags.add ("-flto");

        flags.addTokens (replacePreprocessorTokens (config, getExtraLinkerFlagsString()).trim(), " \n", "\"'");

        auto packages = getPackages();

        if (config.exporter.isLinux())
        {
            if (target.isDynamicLibrary())
                flags.add ("-shared");

            if (packages.size() > 0)
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

        if (getProject().getProjectType().isAudioPlugin() && target.type != ProjectType::Target::SharedCodeTarget)
            librarySearchPaths.add (RelativePath (getSharedCodePath (config), RelativePath::buildTargetFolder).getParentDirectory().toUnixStyle().quoted());

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

    static int getTypeIndex (const ProjectType::Target::Type& type)
    {
        switch (type)
        {
            case ProjectType::Target::GUIApp:
            case ProjectType::Target::StandalonePlugIn:
                return 0;
            case ProjectType::Target::ConsoleApp:
                return 1;
            case ProjectType::Target::StaticLibrary:
            case ProjectType::Target::SharedCodeTarget:
                return 2;
            case ProjectType::Target::DynamicLibrary:
            case ProjectType::Target::VSTPlugIn:
                return 3;
            default:
                break;
        }

        return 0;
    }

    String getOutputPathForTarget (CodeBlocksTarget& target, const BuildConfiguration& config) const
    {
        String outputPath;
        if (config.getTargetBinaryRelativePathString().isNotEmpty())
        {
            RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
            binaryPath = binaryPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder);
            outputPath = config.getTargetBinaryRelativePathString();
        }
        else
        {
            outputPath ="bin/" + File::createLegalFileName (config.getName().trim());
        }

        return outputPath + "/" + replacePreprocessorTokens (config, config.getTargetBinaryNameString() + target.getTargetSuffix());
    }

    String getSharedCodePath (const BuildConfiguration& config) const
    {
        auto outputPath = getOutputPathForTarget (getTargetWithType (ProjectType::Target::SharedCodeTarget), config);
        RelativePath path (outputPath, RelativePath::buildTargetFolder);
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
                bool keepPrefix = (target.type == ProjectType::Target::VSTPlugIn);

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

        if (getProject().getProjectType().isAudioPlugin() && target.type != ProjectType::Target::SharedCodeTarget)
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

                flags.addArray (getCompilerFlags (config, target));

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

            if (getProject().getProjectType().isAudioPlugin() && target.type != ProjectType::Target::SharedCodeTarget)
                setAddOption (*linker, "option", getSharedCodePath (config).quoted());

            for (auto& flag : getLinkerFlags (config, target))
                setAddOption (*linker, "option", flag);

            const StringArray& libs = isWindows() ? mingwLibs : linuxLibs;

            for (auto lib : libs)
                setAddOption (*linker, "library", lib);

            for (auto& path : getLinkerSearchPaths (config, target))
                setAddOption (*linker, "directory", replacePreprocessorDefs (getAllPreprocessorDefs(), path));
        }
    }

    void addBuild (XmlElement& xml) const
    {
        auto* build = xml.createNewChildElement ("Build");

        for (ConstConfigIterator config (*this); config.next();)
            for (auto target : targets)
                if (target->type != ProjectType::Target::AggregateTarget)
                    createBuildTarget (*build->createNewChildElement ("Target"), *target, *config);
    }

    void addVirtualTargets (XmlElement& xml) const
    {
        auto* virtualTargets = xml.createNewChildElement ("VirtualTargets");

        for (ConstConfigIterator config (*this); config.next();)
        {
            StringArray allTargets;

            for (auto target : targets)
                if (target->type != ProjectType::Target::AggregateTarget)
                    allTargets.add (target->getTargetNameForConfiguration (*config));

            for (auto target : targets)
            {
                if (target->type == ProjectType::Target::AggregateTarget)
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
            option = replacePreprocessorDefs (getAllPreprocessorDefs(), option);

        return result;
    }

    void addProjectLinkerOptions (XmlElement& xml) const
    {
        auto* linker = xml.createNewChildElement ("Linker");

        for (auto& lib : getProjectLinkerLibs())
            setAddOption (*linker, "library", lib);
    }

    CodeBlocksTarget& getTargetWithType (ProjectType::Target::Type type) const
    {
        CodeBlocksTarget* nonAggregrateTarget = nullptr;

        for (auto* target : targets)
        {
            if (target->type == type)
                return *target;

            if (target->type != ProjectType::Target::AggregateTarget)
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
        if (getProject().getProjectType().isAudioPlugin())
            return getTargetWithType (ProjectType::Target::SharedCodeTarget);

        for (auto* target : targets)
            if (target->type != ProjectType::Target::AggregateTarget)
                return *target;

        jassertfalse;

        return *targets[0];
    }

    CodeBlocksTarget& getTargetForProjectItem (const Project::Item& projectItem) const
    {
        if (getProject().getProjectType().isAudioPlugin())
        {
            if (! projectItem.shouldBeCompiled())
                return getTargetWithType (ProjectType::Target::SharedCodeTarget);

            return getTargetWithType (getProject().getTargetTypeFromFilePath (projectItem.getFile(), true));
        }

        return getMainTarget();
    }

    void addCompileUnits (const Project::Item& projectItem, XmlElement& xml) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                addCompileUnits (projectItem.getChild(i), xml);
        }
        else if (projectItem.shouldBeAddedToTargetProject())
        {
            RelativePath file (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            auto* unit = xml.createNewChildElement ("Unit");
            unit->setAttribute ("filename", file.toUnixStyle());

            for (ConstConfigIterator config (*this); config.next();)
            {
                auto targetName = getTargetForProjectItem (projectItem).getTargetNameForConfiguration (*config);
                unit->createNewChildElement ("Option")->setAttribute ("target", targetName);
            }

            if (! projectItem.shouldBeCompiled())
            {
                unit->createNewChildElement ("Option")->setAttribute ("compile", 0);
                unit->createNewChildElement ("Option")->setAttribute ("link", 0);
            }
        }
    }

    void addCompileUnits (XmlElement& xml) const
    {
        for (int i = 0; i < getAllGroups().size(); ++i)
            addCompileUnits (getAllGroups().getReference(i), xml);
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

    friend class CLionProjectExporter;

    JUCE_DECLARE_NON_COPYABLE (CodeBlocksProjectExporter)
};
