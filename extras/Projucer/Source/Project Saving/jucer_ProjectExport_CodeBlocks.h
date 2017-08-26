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
    static String getTargetFolderName (CodeBlocksOS os)
    {
        if (os == windowsTarget)  return "CodeBlocksWindows";
        if (os == linuxTarget)    return "CodeBlocksLinux";

        // currently no other OSes supported by Codeblocks exporter!
        jassertfalse;
        return "CodeBlocksUnknownOS";
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

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + getTargetFolderName (os);
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

    void createExporterProperties (PropertyListBuilder&) override
    {
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        const File cbpFile (getTargetFolder().getChildFile (project.getProjectFilenameRoot())
                                             .withFileExtension (".cbp"));

        XmlElement xml ("CodeBlocks_project_file");
        addVersion (xml);
        createProject (*xml.createNewChildElement ("Project"));
        writeXmlOrThrow (xml, cbpFile, "UTF-8", 10);
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

    //==============================================================================
    void initialiseDependencyPathValues() override
    {
        DependencyPathOS pathOS = isLinux() ? TargetOS::linux
        : TargetOS::windows;

        vst3Path.referTo (Value (new DependencyPathValueSource (getSetting (Ids::vst3Folder), Ids::vst3Path, pathOS)));

        if (! isLinux())
        {
            aaxPath.referTo  (Value (new DependencyPathValueSource (getSetting (Ids::aaxFolder), Ids::aaxPath, pathOS)));
            rtasPath.referTo (Value (new DependencyPathValueSource (getSetting (Ids::rtasFolder), Ids::rtasPath, pathOS)));
        }
    }

private:
    //==============================================================================
    class CodeBlocksBuildConfiguration  : public BuildConfiguration
    {
    public:
        CodeBlocksBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
            if (getArchitectureType().toString().isEmpty())
                getArchitectureType() = static_cast<const char* const> ("-m64");
        }

        Value getArchitectureType()
        {
            const auto archID = exporter.isWindows() ? Ids::windowsCodeBlocksArchitecture
                                                     : Ids::linuxCodeBlocksArchitecture;
            return getValue (archID);
        }
        var getArchitectureTypeVar() const
        {
            const auto archID = exporter.isWindows() ? Ids::windowsCodeBlocksArchitecture
                                                     : Ids::linuxCodeBlocksArchitecture;
            return config [archID];
        }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);

            static const char* const archNames[] = { "32-bit (-m32)", "64-bit (-m64)", "ARM v6",       "ARM v7" };
            const var archFlags[]                = { "-m32",          "-m64",          "-march=armv6", "-march=armv7" };

            props.add (new ChoicePropertyComponent (getArchitectureType(), "Architecture",
                                                    StringArray (archNames, numElementsInArray (archNames)),
                                                    Array<var> (archFlags, numElementsInArray (archFlags))));
        }

        String getModuleLibraryArchName() const override
        {
            const String archFlag = getArchitectureTypeVar();

            const auto prefix = String ("-march=");
            if (archFlag.startsWith (prefix))
                return String ("/") + archFlag.substring (prefix.length());
            else if (archFlag == "-m64")
                return "/x86_64";
            else if (archFlag == "-m32")
                return "/i386";

            jassertfalse;
            return {};
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return new CodeBlocksBuildConfiguration (project, tree, *this);
    }

    //==============================================================================
    class CodeBlocksTarget : public ProjectType::Target
    {
    public:
        CodeBlocksTarget (CodeBlocksProjectExporter&, ProjectType::Target::Type typeToUse)
            : ProjectType::Target (typeToUse)
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

            switch (fileType)
            {
                case executable:            return {};
                case staticLibrary:         return ".a";
                case sharedLibraryOrDLL:    return ".so";

                case pluginBundle:
                    switch (type)
                    {
                        case VST3PlugIn:    return ".vst3";
                        case VSTPlugIn:     return ".so";
                        default:            break;
                    }

                    return ".so";

                default:
                    break;
            }

            return {};
        }

        bool isDynamicLibrary() const
        {
            return (type == DynamicLibrary || type == VST3PlugIn
                     || type == VSTPlugIn || type == AAXPlugIn);
        }
    };

    //==============================================================================
    void addVersion (XmlElement& xml) const
    {
        XmlElement* fileVersion = xml.createNewChildElement ("FileVersion");
        fileVersion->setAttribute ("major", 1);
        fileVersion->setAttribute ("minor", 6);
    }

    void addOptions (XmlElement& xml) const
    {
        xml.createNewChildElement ("Option")->setAttribute ("title", project.getTitle());
        xml.createNewChildElement ("Option")->setAttribute ("pch_mode", 2);
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");
    }

    StringArray getDefines (const BuildConfiguration& config, CodeBlocksTarget& target) const
    {
        StringPairArray defines;

        if (isCodeBlocks() && isWindows())
        {
            defines.set ("__MINGW__", "1");
            defines.set ("__MINGW_EXTENSION", String());
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
        for (int i = 0; i < defines.size(); ++i)
            defs.add (defines.getAllKeys()[i] + "=" + defines.getAllValues()[i]);

        return getCleanedStringArray (defs);
    }

    StringArray getCompilerFlags (const BuildConfiguration& config, CodeBlocksTarget& target) const
    {
        StringArray flags;
        if (const auto codeBlocksConfig = dynamic_cast<const CodeBlocksBuildConfiguration*> (&config))
            flags.add (codeBlocksConfig->getArchitectureTypeVar());

        flags.add ("-O" + config.getGCCOptimisationFlag());

        {
            auto cppStandard = config.project.getCppStandardValue().toString();

            if (cppStandard == "latest")
                cppStandard = "1z";

            cppStandard = "-std=" + String (shouldUseGNUExtensions() ? "gnu++" : "c++") + cppStandard;

            flags.add (cppStandard);
        }

        flags.add ("-mstackrealign");

        if (config.isDebug())
            flags.add ("-g");

        flags.addTokens (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim(),
                         " \n", "\"'");

        {
            const StringArray defines (getDefines (config, target));

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines[i]);

                if (! def.containsChar ('='))
                    def << '=';

                flags.add ("-D" + def);
            }
        }

        if (config.exporter.isLinux())
        {
            if (target.isDynamicLibrary() || getProject().getProjectType().isAudioPlugin())
                flags.add ("-fPIC");

            if (linuxPackages.size() > 0)
            {
                auto pkgconfigFlags = String ("`pkg-config --cflags");
                for (auto p : linuxPackages)
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
        StringArray flags (makefileExtraLinkerFlags);

        if (const auto codeBlocksConfig = dynamic_cast<const CodeBlocksBuildConfiguration*> (&config))
            flags.add (codeBlocksConfig->getArchitectureTypeVar());

        if (! config.isDebug())
            flags.add ("-s");

        flags.addTokens (replacePreprocessorTokens (config, getExtraLinkerFlagsString()).trim(),
                         " \n", "\"'");

        if (getProject().getProjectType().isAudioPlugin() && target.type != ProjectType::Target::SharedCodeTarget)
            flags.add ("-l" + config.getTargetBinaryNameString());

        if (config.exporter.isLinux() && linuxPackages.size() > 0)
        {
            if (target.isDynamicLibrary())
                flags.add ("-shared");

            auto pkgconfigLibs = String ("`pkg-config --libs");
            for (auto p : linuxPackages)
                pkgconfigLibs << " " << p;

            pkgconfigLibs << "`";
            flags.add (pkgconfigLibs);
        }

        return getCleanedStringArray (flags);
    }

    StringArray getIncludePaths (const BuildConfiguration& config) const
    {
        StringArray paths;

        paths.add (".");
        paths.addArray (extraSearchPaths);
        paths.addArray (config.getHeaderSearchPaths());

        if (! (isCodeBlocks() && isWindows()))
            paths.add ("/usr/include/freetype2");

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
            case ProjectType::Target::VST3PlugIn:
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
        const String outputPath = getOutputPathForTarget (getTargetWithType (ProjectType::Target::SharedCodeTarget), config);
        RelativePath path (outputPath, RelativePath::buildTargetFolder);

        const String autoPrefixedFilename = "lib" + path.getFileName();
        return path.getParentDirectory().getChildFile (autoPrefixedFilename).toUnixStyle();
    }

    void createBuildTarget (XmlElement& xml, CodeBlocksTarget& target, const BuildConfiguration& config) const
    {
        xml.setAttribute ("title", target.getTargetNameForConfiguration (config));

        {
            XmlElement* output = xml.createNewChildElement ("Option");

            output->setAttribute ("output", getOutputPathForTarget (target, config));

            const bool keepPrefix = (target.type == ProjectType::Target::VSTPlugIn || target.type == ProjectType::Target::VST3PlugIn
                                  || target.type == ProjectType::Target::AAXPlugIn || target.type == ProjectType::Target::RTASPlugIn);

            output->setAttribute ("prefix_auto", keepPrefix ? 0 : 1);
            output->setAttribute ("extension_auto", 0);
        }

        xml.createNewChildElement ("Option")
             ->setAttribute ("object_output", "obj/" + File::createLegalFileName (config.getName().trim()));

        xml.createNewChildElement ("Option")->setAttribute ("type", getTypeIndex (target.type));
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");

        if (getProject().getProjectType().isAudioPlugin() && target.type != ProjectType::Target::SharedCodeTarget)
            xml.createNewChildElement ("Option")->setAttribute ("external_deps", getSharedCodePath (config));

        {
            XmlElement* const compiler = xml.createNewChildElement ("Compiler");

            {
                const StringArray compilerFlags (getCompilerFlags (config, target));

                for (auto flag : compilerFlags)
                    setAddOption (*compiler, "option", flag);
            }

            {
                const StringArray includePaths (getIncludePaths (config));

                for (auto path : includePaths)
                    setAddOption (*compiler, "directory", path);
            }
        }

        {
            XmlElement* const linker = xml.createNewChildElement ("Linker");

            const StringArray linkerFlags (getLinkerFlags (config, target));
            for (auto flag : linkerFlags)
                setAddOption (*linker, "option", flag);

            const StringArray& libs = isWindows() ? mingwLibs : linuxLibs;

            for (auto lib : libs)
                setAddOption (*linker, "library", lib);

            StringArray librarySearchPaths (config.getLibrarySearchPaths());

            if (getProject().getProjectType().isAudioPlugin() && target.type != ProjectType::Target::SharedCodeTarget)
                librarySearchPaths.add (RelativePath (getSharedCodePath (config), RelativePath::buildTargetFolder).getParentDirectory().toUnixStyle());

            for (auto path : librarySearchPaths)
            {
                setAddOption (*linker, "directory", replacePreprocessorDefs (getAllPreprocessorDefs(), path));
            }
        }
    }

    void addBuild (XmlElement& xml) const
    {
        XmlElement* const build = xml.createNewChildElement ("Build");

        for (ConstConfigIterator config (*this); config.next();)
        {
            for (auto target : targets)
                if (target->type != ProjectType::Target::AggregateTarget)
                    createBuildTarget (*build->createNewChildElement ("Target"), *target, *config);
        }
    }

    void addVirtualTargets (XmlElement& xml) const
    {
        XmlElement* const virtualTargets = xml.createNewChildElement ("VirtualTargets");

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

    void addProjectCompilerOptions (XmlElement& xml) const
    {
        XmlElement* const compiler = xml.createNewChildElement ("Compiler");
        setAddOption (*compiler, "option", "-Wall");
        setAddOption (*compiler, "option", "-Wno-strict-aliasing");
        setAddOption (*compiler, "option", "-Wno-strict-overflow");
    }

    void addProjectLinkerOptions (XmlElement& xml) const
    {
        XmlElement* const linker = xml.createNewChildElement ("Linker");

        StringArray libs;

        if (isWindows())
        {
            static const char* defaultLibs[] = { "gdi32", "user32", "kernel32", "comctl32" };
            libs = StringArray (defaultLibs, numElementsInArray (defaultLibs));
        }

        libs.addTokens (getExternalLibrariesString(), ";\n", "\"'");

        libs = getCleanedStringArray (libs);

        for (int i = 0; i < libs.size(); ++i)
            setAddOption (*linker, "library", replacePreprocessorDefs (getAllPreprocessorDefs(), libs[i]));
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
            const RelativePath file (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            XmlElement* unit = xml.createNewChildElement ("Unit");
            unit->setAttribute ("filename", file.toUnixStyle());

            for (ConstConfigIterator config (*this); config.next();)
            {
                const String& targetName = getTargetForProjectItem (projectItem).getTargetNameForConfiguration (*config);
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

    JUCE_DECLARE_NON_COPYABLE (CodeBlocksProjectExporter)
};
