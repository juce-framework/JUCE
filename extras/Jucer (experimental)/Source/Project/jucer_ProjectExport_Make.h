/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

        return 0;
    }


    //==============================================================================
    MakefileProjectExporter (Project& project_, const ValueTree& settings_)
        : ProjectExporter (project_, settings_)
    {
        name = getNameLinux();

        if (getTargetLocation().toString().isEmpty())
            getTargetLocation() = getDefaultBuildsRootFolder() + "Linux";

        if (getVSTFolder().toString().isEmpty())
            getVSTFolder() = "~/SDKs/vstsdk2.4";
    }

    ~MakefileProjectExporter()
    {
    }

    //==============================================================================
    bool isDefaultFormatForCurrentOS()
    {
      #if JUCE_LINUX
        return true;
      #else
        return false;
      #endif
    }

    bool isPossibleForCurrentProject()          { return true; }
    bool usesMMFiles() const                    { return false; }

    void launchProject()
    {
        // what to do on linux?
    }

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);
    }

    //==============================================================================
    const String create()
    {
        Array<RelativePath> files;
        findAllFilesToCompile (project.getMainGroup(), files);

        for (int i = 0; i < juceWrapperFiles.size(); ++i)
            if (shouldFileBeCompiledByDefault (juceWrapperFiles.getReference(i)))
                files.add (juceWrapperFiles.getReference(i));

        const Array<RelativePath> vstFiles (getVSTFilesRequired());
        for (int i = 0; i < vstFiles.size(); i++)
            files.add (vstFiles.getReference(i));

        MemoryOutputStream mo;
        writeMakefile (mo, files);

        const File makefile (getTargetFolder().getChildFile ("Makefile"));
        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (makefile, mo))
            return "Can't write to the Makefile: " + makefile.getFullPathName();

        return String::empty;
    }

private:
    //==============================================================================
    void findAllFilesToCompile (const Project::Item& projectItem, Array<RelativePath>& results)
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

    void writeDefineFlags (OutputStream& out, const Project::BuildConfiguration& config)
    {
        StringPairArray defines;
        defines.set ("LINUX", "1");

        if (config.isDebug().getValue())
        {
            defines.set ("DEBUG", "1");
            defines.set ("_DEBUG", "1");
        }
        else
        {
            defines.set ("NDEBUG", "1");
        }

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

        for (int i = 0; i < defines.size(); ++i)
        {
            String def (defines.getAllKeys()[i]);
            const String value (defines.getAllValues()[i]);
            if (value.isNotEmpty())
                def << "=" << value;

            out << " -D " << def.quoted();
        }
    }

    void writeHeaderPathFlags (OutputStream& out, const Project::BuildConfiguration& config)
    {
        StringArray headerPaths (config.getHeaderSearchPaths());
        headerPaths.insert (0, "/usr/include/freetype2");
        headerPaths.insert (0, "/usr/include");

        if (project.shouldAddVSTFolderToPath() && getVSTFolder().toString().isNotEmpty())
            headerPaths.insert (0, rebaseFromProjectFolderToBuildTarget (RelativePath (getVSTFolder().toString(), RelativePath::projectFolder)).toUnixStyle());

        if (isVST())
            headerPaths.insert (0, juceWrapperFolder.toUnixStyle());

        for (int i = 0; i < headerPaths.size(); ++i)
            out << " -I " << FileHelpers::unixStylePath (replacePreprocessorTokens (config, headerPaths[i])).quoted();
    }

    void writeCppFlags (OutputStream& out, const Project::BuildConfiguration& config)
    {
        out << "  CPPFLAGS := $(DEPFLAGS)";
        writeDefineFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << newLine;
    }

    void writeLinkerFlags (OutputStream& out, const Project::BuildConfiguration& config)
    {
        out << "  LDFLAGS += -L$(BINDIR) -L$(LIBDIR)";

        if (project.isAudioPlugin())
            out << " -shared";

        {
            Array<RelativePath> libraryPaths;
            libraryPaths.add (RelativePath ("/usr/X11R6/lib/", RelativePath::unknown));
            libraryPaths.add (getJucePathFromTargetFolder().getChildFile ("bin"));

            for (int i = 0; i < libraryPaths.size(); ++i)
                out << " -L" << libraryPaths.getReference(i).toUnixStyle().quoted();
        }

        const char* defaultLibs[] = { "freetype", "pthread", "rt", "X11", "GL", "GLU", "Xinerama", "asound", 0 };
        StringArray libs (defaultLibs);

        if (project.getJuceLinkageMode() == Project::useLinkedJuce)
            libs.add ("juce");

        for (int i = 0; i < libs.size(); ++i)
            out << " -l" << libs[i];

        out << " " << replacePreprocessorTokens (config, getExtraLinkerFlags().toString()).trim()
            << newLine;
    }

    void writeConfig (OutputStream& out, const Project::BuildConfiguration& config)
    {
        const String buildDirName ("build");
        const String intermediatesDirName (buildDirName + "/intermediate/" + config.getName().toString());
        String outputDir (buildDirName);

        if (config.getTargetBinaryRelativePath().toString().isNotEmpty())
        {
            RelativePath binaryPath (config.getTargetBinaryRelativePath().toString(), RelativePath::projectFolder);
            outputDir = binaryPath.rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder).toUnixStyle();
        }

        out << "ifeq ($(CONFIG)," << escapeSpaces (config.getName().toString()) << ")" << newLine;
        out << "  BINDIR := " << escapeSpaces (buildDirName) << newLine
            << "  LIBDIR := " << escapeSpaces (buildDirName) << newLine
            << "  OBJDIR := " << escapeSpaces (intermediatesDirName) << newLine
            << "  OUTDIR := " << escapeSpaces (outputDir) << newLine;

        writeCppFlags (out, config);

        out << "  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH)";

        if (config.isDebug().getValue())
            out << " -g -ggdb";

        if (project.isAudioPlugin())
            out << " -fPIC";

        out << " -O" << config.getGCCOptimisationFlag() << newLine;

        out << "  CXXFLAGS += $(CFLAGS) " << replacePreprocessorTokens (config, getExtraCompilerFlags().toString()).trim() << newLine;

        writeLinkerFlags (out, config);

        out << "  LDDEPS :=" << newLine
            << "  RESFLAGS := ";
        writeDefineFlags (out, config);
        writeHeaderPathFlags (out, config);
        out << newLine;

        String targetName (config.getTargetBinaryName().getValue().toString());

        if (project.isLibrary())
            targetName = getLibbedFilename (targetName);
        else if (isVST())
            targetName = targetName.upToLastOccurrenceOf (".", false, false) + ".so";

        out << "  TARGET := " << escapeSpaces (targetName) << newLine;

        if (project.isLibrary())
            out << "  BLDCMD = ar -rcs $(OUTDIR)/$(TARGET) $(OBJECTS) $(TARGET_ARCH)" << newLine;
        else
            out << "  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)" << newLine;

        out << "endif" << newLine << newLine;
    }

    void writeObjects (OutputStream& out, const Array<RelativePath>& files)
    {
        out << "OBJECTS := \\" << newLine;

        for (int i = 0; i < files.size(); ++i)
            if (shouldFileBeCompiledByDefault (files.getReference(i)))
                out << "  $(OBJDIR)/" << escapeSpaces (getObjectFileFor (files.getReference(i))) << " \\" << newLine;

        out << newLine;
    }

    void writeMakefile (OutputStream& out, const Array<RelativePath>& files)
    {
        out << "# Automatically generated makefile, created by the Jucer" << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Jucer project!" << newLine
            << newLine;

        out << "ifndef CONFIG" << newLine
            << "  CONFIG=" << escapeSpaces (project.getConfiguration(0).getName().toString()) << newLine
            << "endif" << newLine
            << newLine;

        if (! project.isLibrary())
            out << "ifeq ($(TARGET_ARCH),)" << newLine
                << "  TARGET_ARCH := -march=native" << newLine
                << "endif"  << newLine << newLine;

        out << "# (this disables dependency generation if multiple architectures are set)" << newLine
            << "DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)" << newLine
            << newLine;

        int i;
        for (i = 0; i < project.getNumConfigurations(); ++i)
            writeConfig (out, project.getConfiguration(i));

        writeObjects (out, files);

        out << ".PHONY: clean" << newLine
            << newLine;

        out << "$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)" << newLine
            << "\t@echo Linking " << project.getProjectName() << newLine
            << "\t-@mkdir -p $(BINDIR)" << newLine
            << "\t-@mkdir -p $(LIBDIR)" << newLine
            << "\t-@mkdir -p $(OUTDIR)" << newLine
            << "\t@$(BLDCMD)" << newLine
            << newLine;

        out << "clean:" << newLine
            << "\t@echo Cleaning " << project.getProjectName() << newLine
            << "\t-@rm -f $(OUTDIR)/$(TARGET)" << newLine
            << "\t-@rm -rf $(OBJDIR)/*" << newLine
            << "\t-@rm -rf $(OBJDIR)" << newLine
            << newLine;

        for (i = 0; i < files.size(); ++i)
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

    static const String escapeSpaces (const String& s)
    {
        return s.replace (" ", "\\ ");
    }

    const String getObjectFileFor (const RelativePath& file) const
    {
        return file.getFileNameWithoutExtension()
                + "_" + String::toHexString (file.toUnixStyle().hashCode()) + ".o";
    }

    JUCE_DECLARE_NON_COPYABLE (MakefileProjectExporter);
};


#endif   // __JUCER_PROJECTEXPORT_MAKE_JUCEHEADER__
