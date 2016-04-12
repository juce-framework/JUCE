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

class MakefileProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameLinux()           { return "Linux Makefile"; }
    static const char* getValueTreeTypeName()   { return "LINUX_MAKE"; }

    static MakefileProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new MakefileProjectExporter (project, settings);

        return nullptr;
    }


    //==============================================================================
    MakefileProjectExporter (Project& p, const ValueTree& t)   : ProjectExporter (p, t)
    {
        name = getNameLinux();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "LinuxMakefile";

        initialiseDependencyPathValues();
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
    bool isAndroidAnt() const override                  { return false; }

    bool isAndroid() const override                     { return false; }
    bool isWindows() const override                     { return false; }
    bool isLinux() const override                       { return true; }
    bool isOSX() const override                         { return false; }
    bool isiOS() const override                         { return false; }

    bool supportsVST() const override                   { return true; }
    bool supportsVST3() const override                  { return false; }
    bool supportsAAX() const override                   { return false; }
    bool supportsRTAS() const override                  { return false; }
    bool supportsAU()   const override                  { return false; }
    bool supportsAUv3() const override                  { return false; }
    bool supportsStandalone() const override            { return false;  }

    Value getCppStandardValue()                         { return getSetting (Ids::cppLanguageStandard); }
    String getCppStandardString() const                 { return settings[Ids::cppLanguageStandard]; }

    void createExporterProperties (PropertyListBuilder& properties) override
    {
        static const char* cppStandardNames[]  = { "C++03",       "C++11",       "C++14",        nullptr };
        static const char* cppStandardValues[] = { "-std=c++03",  "-std=c++11",  "-std=c++14",   nullptr };

        properties.add (new ChoicePropertyComponent (getCppStandardValue(),
                                                     "C++ standard to use",
                                                     StringArray (cppStandardNames),
                                                     Array<var>  (cppStandardValues)),
                        "The C++ standard to specify in the makefile");
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        Array<RelativePath> files;
        for (int i = 0; i < getAllGroups().size(); ++i)
            findAllFilesToCompile (getAllGroups().getReference(i), files);

        MemoryOutputStream mo;
        writeMakefile (mo, files);

        overwriteFileIfDifferentOrThrow (getTargetFolder().getChildFile ("Makefile"), mo);
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType& type) override
    {
        if (type.isStaticLibrary())
            makefileTargetSuffix = ".a";

        else if (type.isDynamicLibrary())
            makefileTargetSuffix = ".so";

        else if (type.isAudioPlugin())
            makefileIsDLL = true;
    }

protected:
    //==============================================================================
    class MakeBuildConfiguration  : public BuildConfiguration
    {
    public:
        MakeBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
            setValueIfVoid (getLibrarySearchPathValue(), "/usr/X11R6/lib/");
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
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return new MakeBuildConfiguration (project, tree, *this);
    }

private:
    //==============================================================================
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
                results.add (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder));
        }
    }

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

        out << createGCCPreprocessorFlags (mergePreprocessorDefs (defines, getAllPreprocessorDefs (config)));
    }

    void writeHeaderPathFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());

        searchPaths.insert (0, "/usr/include/freetype2");
        searchPaths.insert (0, "/usr/include");

        searchPaths = getCleanedStringArray (searchPaths);

        for (int i = 0; i < searchPaths.size(); ++i)
            out << " -I " << escapeSpaces (FileHelpers::unixStylePath (replacePreprocessorTokens (config, searchPaths[i])));
    }

    void writeCppFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        out << "  CPPFLAGS := $(DEPFLAGS)";
        writeDefineFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << newLine;
    }

    void writeLinkerFlags (OutputStream& out, const BuildConfiguration& config) const
    {
        out << "  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR)";

        {
            StringArray flags (makefileExtraLinkerFlags);

            if (makefileIsDLL)
                flags.add ("-shared");

            if (! config.isDebug())
                flags.add ("-fvisibility=hidden");

            if (flags.size() > 0)
                out << " " << getCleanedStringArray (flags).joinIntoString (" ");
        }

        out << config.getGCCLibraryPathFlags();

        for (int i = 0; i < linuxLibs.size(); ++i)
            out << " -l" << linuxLibs[i];

        if (getProject().isConfigFlagEnabled ("JUCE_USE_CURL"))
            out << " -lcurl";

        StringArray libraries;
        libraries.addTokens (getExternalLibrariesString(), ";", "\"'");
        libraries.removeEmptyStrings();

        if (libraries.size() != 0)
            out << " -l" << replacePreprocessorTokens (config, libraries.joinIntoString (" -l")).trim();

        out << " " << replacePreprocessorTokens (config, getExtraLinkerFlagsString()).trim()
            << newLine;
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
        out << "  BINDIR := " << escapeSpaces (buildDirName) << newLine
            << "  LIBDIR := " << escapeSpaces (buildDirName) << newLine
            << "  OBJDIR := " << escapeSpaces (intermediatesDirName) << newLine
            << "  OUTDIR := " << escapeSpaces (outputDir) << newLine
            << newLine
            << "  ifeq ($(TARGET_ARCH),)" << newLine
            << "    TARGET_ARCH := " << getArchFlags (config) << newLine
            << "  endif"  << newLine
            << newLine;

        writeCppFlags (out, config);

        out << "  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH)";

        if (config.isDebug())
            out << " -g -ggdb";

        if (makefileIsDLL)
            out << " -fPIC";

        out << " -O" << config.getGCCOptimisationFlag()
            << (" "  + replacePreprocessorTokens (config, getExtraCompilerFlagsString())).trimEnd()
            << newLine;

        String cppStandardToUse (getCppStandardString());

        if (cppStandardToUse.isEmpty())
            cppStandardToUse = "-std=c++11";

        out << "  CXXFLAGS += $(CFLAGS) "
            << cppStandardToUse
            << newLine;

        writeLinkerFlags (out, config);

        out << newLine;

        String targetName (replacePreprocessorTokens (config, config.getTargetBinaryNameString()));

        if (projectType.isStaticLibrary() || projectType.isDynamicLibrary())
            targetName = getLibbedFilename (targetName);
        else
            targetName = targetName.upToLastOccurrenceOf (".", false, false) + makefileTargetSuffix;

        out << "  TARGET := " << escapeSpaces (targetName) << newLine;

        if (projectType.isStaticLibrary())
            out << "  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS)" << newLine;
        else
            out << "  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)" << newLine;

        out << "  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)" << newLine
            << "endif" << newLine
            << newLine;
    }

    void writeObjects (OutputStream& out, const Array<RelativePath>& files) const
    {
        out << "OBJECTS := \\" << newLine;

        for (int i = 0; i < files.size(); ++i)
            if (shouldFileBeCompiledByDefault (files.getReference(i)))
                out << "  $(OBJDIR)/" << escapeSpaces (getObjectFileFor (files.getReference(i))) << " \\" << newLine;

        out << newLine;
    }

    void writeMakefile (OutputStream& out, const Array<RelativePath>& files) const
    {
        out << "# Automatically generated makefile, created by the Projucer" << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
            << newLine;

        out << "# (this disables dependency generation if multiple architectures are set)" << newLine
            << "DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)" << newLine
            << newLine;

        out << "ifndef CONFIG" << newLine
            << "  CONFIG=" << escapeSpaces (getConfiguration(0)->getName()) << newLine
            << "endif" << newLine
            << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            writeConfig (out, *config);

        writeObjects (out, files);

        out << ".PHONY: clean" << newLine
            << newLine;

        out << "$(OUTDIR)/$(TARGET): $(OBJECTS) $(RESOURCES)" << newLine
            << "\t@echo Linking " << projectName << newLine
            << "\t-@mkdir -p $(BINDIR)" << newLine
            << "\t-@mkdir -p $(LIBDIR)" << newLine
            << "\t-@mkdir -p $(OUTDIR)" << newLine
            << "\t@$(BLDCMD)" << newLine
            << newLine;

        out << "clean:" << newLine
            << "\t@echo Cleaning " << projectName << newLine
            << "\t@$(CLEANCMD)" << newLine
            << newLine;

        out << "strip:" << newLine
            << "\t@echo Stripping " << projectName << newLine
            << "\t-@strip --strip-unneeded $(OUTDIR)/$(TARGET)" << newLine
            << newLine;

        for (int i = 0; i < files.size(); ++i)
        {
            if (shouldFileBeCompiledByDefault (files.getReference(i)))
            {
                jassert (files.getReference(i).getRoot() == RelativePath::buildTargetFolder);

                out << "$(OBJDIR)/" << escapeSpaces (getObjectFileFor (files.getReference(i)))
                    << ": " << escapeSpaces (files.getReference(i).toUnixStyle()) << newLine
                    << "\t-@mkdir -p $(OBJDIR)" << newLine
                    << "\t@echo \"Compiling " << files.getReference(i).getFileName() << "\"" << newLine
                    << (files.getReference(i).hasFileExtension ("c;s;S") ? "\t@$(CC) $(CFLAGS) -o \"$@\" -c \"$<\""
                                                                         : "\t@$(CXX) $(CXXFLAGS) -o \"$@\" -c \"$<\"")
                    << newLine << newLine;
            }
        }

        out << "-include $(OBJECTS:%.o=%.d)" << newLine;
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

    void initialiseDependencyPathValues()
    {
        vst2Path.referTo (Value (new DependencyPathValueSource (getSetting (Ids::vstFolder),
                                                                Ids::vst2Path,
                                                                TargetOS::linux)));

        vst3Path.referTo (Value (new DependencyPathValueSource (getSetting (Ids::vst3Folder),
                                                                Ids::vst3Path,
                                                                TargetOS::linux)));
    }

    JUCE_DECLARE_NON_COPYABLE (MakefileProjectExporter)
};
