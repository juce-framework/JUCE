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
        return "CODEBLOCKS_UNKOWN_OS";
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

        initialiseDependencyPathValues();
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
    bool isAndroidAnt() const override               { return false; }

    bool isAndroid() const override                  { return false; }
    bool isWindows() const override                  { return os == windowsTarget; }
    bool isLinux() const override                    { return os == linuxTarget; }
    bool isOSX() const override                      { return false; }
    bool isiOS() const override                      { return false; }

    bool supportsVST() const override                { return false; }
    bool supportsVST3() const override               { return false; }
    bool supportsAAX() const override                { return false; }
    bool supportsRTAS() const override               { return false; }
    bool supportsAU()   const override               { return false; }
    bool supportsAUv3() const override               { return false; }
    bool supportsStandalone() const override         { return false;  }

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
        // no-op.
    }

private:
    //==============================================================================
    class CodeBlocksBuildConfiguration  : public BuildConfiguration
    {
    public:
        CodeBlocksBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
        }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            addGCCOptimisationProperty (props);
        }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return new CodeBlocksBuildConfiguration (project, tree, *this);
    }

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

    StringArray getDefines (const BuildConfiguration& config) const
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

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

        StringArray defs;
        for (int i = 0; i < defines.size(); ++i)
            defs.add (defines.getAllKeys()[i] + "=" + defines.getAllValues()[i]);

        return getCleanedStringArray (defs);
    }

    StringArray getCompilerFlags (const BuildConfiguration& config) const
    {
        StringArray flags;
        flags.add ("-O" + config.getGCCOptimisationFlag());
        flags.add ("-std=c++11");
        flags.add ("-mstackrealign");

        if (config.isDebug())
            flags.add ("-g");

        flags.addTokens (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim(),
                         " \n", "\"'");

        {
            const StringArray defines (getDefines (config));

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines[i]);

                if (! def.containsChar ('='))
                    def << '=';

                flags.add ("-D" + def);
            }
        }

        return getCleanedStringArray (flags);
    }

    StringArray getLinkerFlags (const BuildConfiguration& config) const
    {
        StringArray flags (makefileExtraLinkerFlags);

        if (! config.isDebug())
            flags.add ("-s");

        flags.addTokens (replacePreprocessorTokens (config, getExtraLinkerFlagsString()).trim(),
                         " \n", "\"'");

        return getCleanedStringArray (flags);
    }

    StringArray getIncludePaths (const BuildConfiguration& config) const
    {
        StringArray paths;

        paths.add (".");
        paths.add (RelativePath (project.getGeneratedCodeFolder(),
                                 getTargetFolder(), RelativePath::buildTargetFolder).toWindowsStyle());

        paths.addArray (config.getHeaderSearchPaths());

        if (! (isCodeBlocks() && isWindows()))
            paths.add ("/usr/include/freetype2");

        return getCleanedStringArray (paths);
    }

    static int getTypeIndex (const ProjectType& type)
    {
        if (type.isGUIApplication()) return 0;
        if (type.isCommandLineApp()) return 1;
        if (type.isStaticLibrary())  return 2;
        if (type.isDynamicLibrary()) return 3;
        if (type.isAudioPlugin())    return 3;
        return 0;
    }

    void createBuildTarget (XmlElement& xml, const BuildConfiguration& config) const
    {
        xml.setAttribute ("title", config.getName());

        {
            XmlElement* output = xml.createNewChildElement ("Option");

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

            output->setAttribute ("output", outputPath + "/" + replacePreprocessorTokens (config, config.getTargetBinaryNameString()));

            output->setAttribute ("prefix_auto", 1);
            output->setAttribute ("extension_auto", 1);
        }

        xml.createNewChildElement ("Option")
             ->setAttribute ("object_output", "obj/" + File::createLegalFileName (config.getName().trim()));

        xml.createNewChildElement ("Option")->setAttribute ("type", getTypeIndex (project.getProjectType()));
        xml.createNewChildElement ("Option")->setAttribute ("compiler", "gcc");

        {
            XmlElement* const compiler = xml.createNewChildElement ("Compiler");

            {
                const StringArray compilerFlags (getCompilerFlags (config));

                for (int i = 0; i < compilerFlags.size(); ++i)
                    setAddOption (*compiler, "option", compilerFlags[i]);
            }

            {
                const StringArray includePaths (getIncludePaths (config));

                for (int i = 0; i < includePaths.size(); ++i)
                    setAddOption (*compiler, "directory", includePaths[i]);
            }
        }

        {
            XmlElement* const linker = xml.createNewChildElement ("Linker");

            const StringArray linkerFlags (getLinkerFlags (config));
            for (int i = 0; i < linkerFlags.size(); ++i)
                setAddOption (*linker, "option", linkerFlags[i]);

            const StringArray& libs = isWindows() ? mingwLibs : linuxLibs;

            for (int i = 0; i < libs.size(); ++i)
                setAddOption (*linker, "library", libs[i]);

            const StringArray librarySearchPaths (config.getLibrarySearchPaths());
            for (int i = 0; i < librarySearchPaths.size(); ++i)
                setAddOption (*linker, "directory", replacePreprocessorDefs (getAllPreprocessorDefs(), librarySearchPaths[i]));
        }
    }

    void addBuild (XmlElement& xml) const
    {
        XmlElement* const build = xml.createNewChildElement ("Build");

        for (ConstConfigIterator config (*this); config.next();)
            createBuildTarget (*build->createNewChildElement ("Target"), *config);
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

            if (! projectItem.shouldBeCompiled())
            {
                unit->createNewChildElement("Option")->setAttribute ("compile", 0);
                unit->createNewChildElement("Option")->setAttribute ("link", 0);
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
        addProjectCompilerOptions (xml);
        addProjectLinkerOptions (xml);
        addCompileUnits (xml);
    }

    void setAddOption (XmlElement& xml, const String& nm, const String& value) const
    {
        xml.createNewChildElement ("Add")->setAttribute (nm, value);
    }

    void initialiseDependencyPathValues()
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

    CodeBlocksOS os;

    JUCE_DECLARE_NON_COPYABLE (CodeBlocksProjectExporter)
};
