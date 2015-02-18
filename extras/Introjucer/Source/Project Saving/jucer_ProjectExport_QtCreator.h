/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

class QtCreatorProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameQtCreator()       { return "QtCreator";  }
    static const char* getValueTreeTypeName()   { return "QT_CREATOR"; }

    static QtCreatorProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new QtCreatorProjectExporter (project, settings);

        return nullptr;
    }


    //==============================================================================
    QtCreatorProjectExporter (Project& p, const ValueTree& t)   : ProjectExporter (p, t)
    {
        name = getNameQtCreator();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "QtCreator";
    }

    //==============================================================================
    bool canLaunchProject() override                    { return false; }
    bool launchProject() override                       { return false; }
    bool usesMMFiles() const override                   { return false; }
    bool isQtCreator() const override                   { return true; }
    bool canCopeWithDuplicateFiles() override           { return false; }

    void createExporterProperties (PropertyListBuilder&) override
    {
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        Array<RelativePath> sourceFiles;
        Array<RelativePath> headerFiles;
        for (int i = 0; i < getAllGroups().size(); ++i) {
            findAllSourceFiles (getAllGroups().getReference(i), sourceFiles);
            findAllHeaderFiles (getAllGroups().getReference(i), headerFiles);
        }
        MemoryOutputStream mo;
        writeMakefile (mo, sourceFiles, headerFiles);

        overwriteFileIfDifferentOrThrow (getTargetFolder()
                                         .getChildFile (project.getProjectFilenameRoot())
                                         .withFileExtension (".pro"), mo);
    }

protected:
    //==============================================================================
    class QtCreatorBuildConfiguration  : public BuildConfiguration
    {
    public:
        QtCreatorBuildConfiguration (Project& p, const ValueTree& settings)
            : BuildConfiguration (p, settings)
        {
            setValueIfVoid (getLibrarySearchPathValue(), "/usr/X11R6/lib/");
        }

        Value getArchitectureType()                 { return getValue (Ids::linuxArchitecture); }
        String getArchitectureTypeString() const    { return config [Ids::linuxArchitecture]; }

        void createConfigProperties (PropertyListBuilder& props) override
        {
            /*static const char* const archNames[] = { "(Default)", "32-bit (-m32)", "64-bit (-m64)", "ARM v6", "ARM v7" };
            const var archFlags[] = { var(), "-m32", "-m64", "-march=armv6", "-march=armv7" };

            props.add (new ChoicePropertyComponent (getArchitectureType(), "Architecture",
                                                    StringArray (archNames, numElementsInArray (archNames)),
                                                    Array<var> (archFlags, numElementsInArray (archFlags)))); */
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return new QtCreatorBuildConfiguration (project, tree);
    }

private:
    //==============================================================================
    void findAllSourceFiles (const Project::Item& projectItem, Array<RelativePath>& results) const
    {
        if (projectItem.isGroup())
        {
            String sName = projectItem.getName();
            int inumChild = projectItem.getNumChildren();
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                findAllSourceFiles(projectItem.getChild(i), results);
        }
        else
        {
            String sName = projectItem.getName();
            if (projectItem.shouldBeCompiled())
                results.add (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder));
        }
    }

    void findAllHeaderFiles (const Project::Item& projectItem, Array<RelativePath>& results) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                findAllHeaderFiles(projectItem.getChild(i), results);
        }
        else
        {
            if (projectItem.getFile().hasFileExtension (headerFileExtensions))
                results.add (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder));
        }
    }

    String createDefineFlags (const StringPairArray& defs) const
    {
        String s = "    DEFINES += \\" + newLine ;

        for (int i = 0; i < defs.size(); ++i)
        {
            String def (defs.getAllKeys()[i]);
            const String value (defs.getAllValues()[i]);
            if (value.isNotEmpty()) {
                def << "=" << value;
                def = def.quoted();
            }
            s += "        " + def + " \\" + newLine;
        }

        return s;
    }

    void writeMakefile (OutputStream& out, const Array<RelativePath>& sourceFiles, const Array<RelativePath>& headerFiles) const
    {
        out << "# Automatically generated qmake file, created by the Introjucer" << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!" << newLine
            << newLine;

        if (projectType.isStaticLibrary()) {
            out << "TEMPLATE = lib" << newLine;
            out << "CONFIG  += static" << newLine;
        } else if (projectType.isDynamicLibrary()) {
            out << "TEMPLATE = lib" << newLine;
        } else if  (projectType.isAudioPlugin()) {
            out << "TEMPLATE = lib" << newLine;
            out << "CONFIG  += plugin no_plugin_name_prefix" << newLine;
        } else {
            out << "TEMPLATE = app" << newLine;
        }

        out << "CONFIG  -= qt" << newLine;
        out << "CONFIG  += warn_off" << newLine;
        out << newLine;
        out << "CONFIG(release, debug|release){" << newLine
            << "    DESTDIR     = build/release/" << newLine
            << "    OBJECTS_DIR = build/release/intermediate/" << newLine;
        for (ConstConfigIterator config (*this); config.next();)
            if (!config->isDebug())
                out << "    TARGET = " << config->getTargetBinaryNameString() << newLine;
        out << "}" << newLine;

        out << "CONFIG(debug, debug|release){" << newLine
            << "    DESTDIR     = build/debug/" << newLine
            << "    OBJECTS_DIR = build/debug/intermediate/" << newLine;
        for (ConstConfigIterator config (*this); config.next();)
            if (config->isDebug())
               out << "    TARGET = " << config->getTargetBinaryNameString() << newLine;
        out << "}" << newLine;
        out << newLine;

        out << "# Compiler flags" << newLine;

        StringPairArray defines;
        // general options
        out << "QMAKE_CXXFLAGS = -std=c++11 -Wall" << newLine;
        // Linux specific options
        defines.clear();
        out << "unix:  QMAKE_CXXFLAGS += -I/usr/include/freetype2 -I/usr/include";
        if (makefileIsDLL)
            out << " -fPIC";
        defines.set ("LINUX", "1");
        out << createGCCPreprocessorFlags (defines)
            << newLine;

        // Windows specific options
        defines.clear();
        out << "win32: QMAKE_CXXFLAGS += -mstackrealign -D__MINGW__=1 -D__MINGW_EXTENSION="
            << createGCCPreprocessorFlags (defines)
            << newLine;

        out << newLine;

        for (ConstConfigIterator config (*this); config.next();) {
            if (!config->isDebug()) {
                out << "QMAKE_CXXFLAGS_RELEASE = ";
                out << " -O" << config->getGCCOptimisationFlag();
                out << (" "  + replacePreprocessorTokens (*config, getExtraCompilerFlagsString())).trimEnd()
                    << newLine;

                // include paths
                StringArray searchPaths (extraSearchPaths);
                searchPaths.addArray (config->getHeaderSearchPaths());
                searchPaths.removeDuplicates (false);
                out << "CONFIG(release, debug|release){" << newLine
                    << "    INCLUDEPATH = \\" << newLine;
                for (int i = 0; i < searchPaths.size(); ++i)
                    out << "        " << addQuotesIfContainsSpaces (FileHelpers::unixStylePath (replacePreprocessorTokens (*config, searchPaths[i]))) << " \\" << newLine;
                out << newLine;

                defines.clear();
                defines.set ("NDEBUG", "1");
                out << createDefineFlags (mergePreprocessorDefs (defines, getAllPreprocessorDefs (*config)))
                    << newLine;
                out << "}" << newLine;
                out << newLine;

            } else {

                out << "QMAKE_CXXFLAGS_DEBUG   = -g -ggdb ";
                out << " -O" << config->getGCCOptimisationFlag();
                out << (" "  + replacePreprocessorTokens (*config, getExtraCompilerFlagsString())).trimEnd()
                    << newLine;

                // include paths
                StringArray searchPaths (extraSearchPaths);
                searchPaths.addArray (config->getHeaderSearchPaths());
                searchPaths.removeDuplicates (false);
                out << "CONFIG(debug, debug|release){" << newLine
                    << "    INCLUDEPATH = \\" << newLine;
                for (int i = 0; i < searchPaths.size(); ++i)
                    out << "        " << addQuotesIfContainsSpaces (FileHelpers::unixStylePath (replacePreprocessorTokens (*config, searchPaths[i]))) << " \\" << newLine;
                out << newLine;

                defines.clear();
                defines.set ("DEBUG", "1");
                defines.set ("_DEBUG", "1");
                out << createDefineFlags (mergePreprocessorDefs (defines, getAllPreprocessorDefs (*config)))
                    << newLine;
                out << "}" << newLine;
                out << newLine;
            }
        }

        out << newLine;

        // Copy flags from C to CXX
        out << "QMAKE_CFLAGS         = $$QMAKE_CXXFLAGS"         << newLine
            << "QMAKE_CFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE" << newLine
            << "QMAKE_CFLAGS_DEBUG   = $$QMAKE_CXXFLAGS_DEBUG"   << newLine;

        out << newLine  << newLine;

        // Linker flags
        out << "# Linker flags" << newLine;
        out << "LIBS = -L$$DESTDIR " << getExternalLibrariesString();
        if (makefileIsDLL)
            out << " -shared";
         out << newLine;

        // Linux specific linker flags
        out << "unix:  LIBS += -L/usr/X11R6/lib/";
        for (int i = 0; i < linuxLibs.size(); ++i)
            out << " -l" << linuxLibs[i];
        out << newLine;

        // Windows specific linker flags
        out << "win32: LIBS += -lgdi32 -luser32 -lkernel32 -lcomctl32";
        for (int i = 0; i < mingwLibs.size(); ++i)
            out << " -l" << mingwLibs[i];
        // statically link pthread as it usually is not in the path on windows
        out << " -static -lpthread" << newLine;

        // statically link some libraries on windows, so we can easily
        // run the program even if the compiler libraries are not
        // in the path
        out << "win32: QMAKE_LFLAGS += -static-libstdc++ -static-libgcc" << newLine;

        out << "QMAKE_LFLAGS += "
            << getExtraLinkerFlagsString()
            << newLine;

        // Debug specific linker flags
        out << "QMAKE_LFLAGS_DEBUG += -fvisibility=hidden" << newLine;
        out << newLine;

        out << "# Source and header files" << newLine;
        // Collect all source files
        out << "SOURCES = \\" << newLine;
        for (int i = 0; i < sourceFiles.size(); ++i)
        {
            //if (shouldFileBeCompiledByDefault (sourceFiles.getReference(i)))
            //{
                jassert (sourceFiles.getReference(i).getRoot() == RelativePath::buildTargetFolder);
                out << "\t\"" << (sourceFiles.getReference(i).toUnixStyle()) << "\" \\" << newLine;
            //}
        }
        out << newLine;

        // Collect all header files
        out << newLine << "HEADERS = \\" << newLine;
        for (int i = 0; i < headerFiles.size(); ++i)
        {
            jassert (headerFiles.getReference(i).getRoot() == RelativePath::buildTargetFolder);
            out << "\t\"" << (headerFiles.getReference(i).toUnixStyle()) << "\" \\" << newLine;
        }
        out << newLine;

    }

    /*String getArchFlags (const BuildConfiguration& config) const
    {
        if (const MakeBuildConfiguration* makeConfig = dynamic_cast<const MakeBuildConfiguration*> (&config))
            if (makeConfig->getArchitectureTypeString().isNotEmpty())
                return makeConfig->getArchitectureTypeString();

        return "-march=native";
    }*/
    JUCE_DECLARE_NON_COPYABLE (QtCreatorProjectExporter)
};
