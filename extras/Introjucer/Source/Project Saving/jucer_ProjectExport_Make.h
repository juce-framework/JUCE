/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_PROJECTEXPORT_MAKE_JUCEHEADER__
#define __JUCER_PROJECTEXPORT_MAKE_JUCEHEADER__

#include "jucer_ProjectExporter.h"


//==============================================================================
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
    MakefileProjectExporter (Project& project_, const ValueTree& settings_)
        : ProjectExporter (project_, settings_)
    {
        name = getNameLinux();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + "Linux";
    }

    //==============================================================================
    int getLaunchPreferenceOrderForCurrentOS()
    {
       #if JUCE_LINUX
        return 1;
       #else
        return 0;
       #endif
    }

    bool isPossibleForCurrentProject()          { return true; }
    bool usesMMFiles() const                    { return false; }
    bool isLinux() const                        { return true; }
    bool canCopeWithDuplicateFiles()            { return false; }

    void launchProject()
    {
        // what to do on linux?
    }

    void createPropertyEditors (PropertyListBuilder& props)
    {
        ProjectExporter::createPropertyEditors (props);
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const
    {
        Array<RelativePath> files;
        for (int i = 0; i < groups.size(); ++i)
            findAllFilesToCompile (groups.getReference(i), files);

        MemoryOutputStream mo;
        writeMakefile (mo, files);

        overwriteFileIfDifferentOrThrow (getTargetFolder().getChildFile ("Makefile"), mo);
    }

protected:
    //==============================================================================
    class MakeBuildConfiguration  : public BuildConfiguration
    {
    public:
        MakeBuildConfiguration (Project& project, const ValueTree& settings)
            : BuildConfiguration (project, settings)
        {
            setValueIfVoid (getLibrarySearchPathValue(), "/usr/X11R6/lib/");
        }

        void createPropertyEditors (PropertyListBuilder& props)
        {
            createBasicPropertyEditors (props);
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& settings) const
    {
        return new MakeBuildConfiguration (project, settings);
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

        searchPaths.removeDuplicates (false);

        for (int i = 0; i < searchPaths.size(); ++i)
            out << " -I " << addQuotesIfContainsSpaces (FileHelpers::unixStylePath (replacePreprocessorTokens (config, searchPaths[i])));
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
        out << "  LDFLAGS += -L$(BINDIR) -L$(LIBDIR)";

        if (makefileIsDLL)
            out << " -shared";

        out << config.getGCCLibraryPathFlags();

        for (int i = 0; i < linuxLibs.size(); ++i)
            out << " -l" << linuxLibs[i];

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
            << "  OUTDIR := " << escapeSpaces (outputDir) << newLine;

        writeCppFlags (out, config);

        out << "  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH)";

        if (config.isDebug())
            out << " -g -ggdb";

        if (makefileIsDLL)
            out << " -fPIC";

        out << " -O" << config.getGCCOptimisationFlag() << newLine;

        out << "  CXXFLAGS += $(CFLAGS) " << replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim() << newLine;

        writeLinkerFlags (out, config);

        out << "  LDDEPS :=" << newLine
            << "  RESFLAGS := ";
        writeDefineFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << newLine;

        String targetName (config.getTargetBinaryNameString());

        if (projectType.isLibrary())
            targetName = getLibbedFilename (targetName);
        else
            targetName = targetName.upToLastOccurrenceOf (".", false, false) + makefileTargetSuffix;

        out << "  TARGET := " << escapeSpaces (targetName) << newLine;

        if (projectType.isLibrary())
            out << "  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS) $(TARGET_ARCH)" << newLine;
        else
            out << "  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)" << newLine;

        out << "endif" << newLine << newLine;
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
        out << "# Automatically generated makefile, created by the Introjucer" << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!" << newLine
            << newLine;

        out << "ifndef CONFIG" << newLine
            << "  CONFIG=" << escapeSpaces (getConfiguration(0)->getName()) << newLine
            << "endif" << newLine
            << newLine;

        if (! projectType.isLibrary())
            out << "ifeq ($(TARGET_ARCH),)" << newLine
                << "  TARGET_ARCH := -march=native" << newLine
                << "endif"  << newLine << newLine;

        out << "# (this disables dependency generation if multiple architectures are set)" << newLine
            << "DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)" << newLine
            << newLine;

        for (ConstConfigIterator config (*this); config.next();)
            writeConfig (out, *config);

        writeObjects (out, files);

        out << ".PHONY: clean" << newLine
            << newLine;

        out << "$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)" << newLine
            << "\t@echo Linking " << projectName << newLine
            << "\t-@mkdir -p $(BINDIR)" << newLine
            << "\t-@mkdir -p $(LIBDIR)" << newLine
            << "\t-@mkdir -p $(OUTDIR)" << newLine
            << "\t@$(BLDCMD)" << newLine
            << newLine;

        out << "clean:" << newLine
            << "\t@echo Cleaning " << projectName << newLine
            << "\t-@rm -f $(OUTDIR)/$(TARGET)" << newLine
            << "\t-@rm -rf $(OBJDIR)/*" << newLine
            << "\t-@rm -rf $(OBJDIR)" << newLine
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
                    << (files.getReference(i).hasFileExtension (".c") ? "\t@$(CC) $(CFLAGS) -o \"$@\" -c \"$<\""
                                                                      : "\t@$(CXX) $(CXXFLAGS) -o \"$@\" -c \"$<\"")
                    << newLine << newLine;
            }
        }

        out << "-include $(OBJECTS:%.o=%.d)" << newLine;
    }

    String getObjectFileFor (const RelativePath& file) const
    {
        return file.getFileNameWithoutExtension()
                + "_" + String::toHexString (file.toUnixStyle().hashCode()) + ".o";
    }

    JUCE_DECLARE_NON_COPYABLE (MakefileProjectExporter);
};


#endif   // __JUCER_PROJECTEXPORT_MAKE_JUCEHEADER__
