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

#ifndef __JUCER_JUCELIBRARYMODULE_JUCEHEADER__
#define __JUCER_JUCELIBRARYMODULE_JUCEHEADER__


//==============================================================================
class JuceLibraryModule  : public LibraryModule
{
public:
    JuceLibraryModule (const File& file)
        : moduleInfo (JSON::parse (file)),
          moduleFile (file),
          moduleFolder (file.getParentDirectory())
    {
        jassert (isValid());
    }

    String getID() const    { return moduleInfo ["id"].toString(); };
    bool isValid() const    { return getID().isNotEmpty(); }

    void writeIncludes (Project& project, OutputStream& out)
    {
        File header (getInclude());

        StringArray paths, guards;
        createMultipleIncludes (project, getPathToModuleFile (header),
                                paths, guards);

        ProjectSaver::writeGuardedInclude (out, paths, guards);
    }

    void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
    {
        Project& project = exporter.getProject();

        {
            Array<File> compiled;
            findAndAddCompiledCode (exporter, projectSaver, compiled);

            if (project.shouldShowAllModuleFilesInProject (getID()).getValue())
                addIncludedCode (exporter, compiled);
        }

        if (isVSTPluginHost (project))
            VSTHelpers::addVSTFolderToPath (exporter, exporter.extraSearchPaths);

        if (isAUPluginHost (project))
            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

        if (isPluginClient())
        {
            if (shouldBuildVST (project).getValue())   VSTHelpers::prepareExporter (exporter, projectSaver);
            if (shouldBuildRTAS (project).getValue())  RTASHelpers::prepareExporter (exporter, projectSaver, moduleFolder);
            if (shouldBuildAU (project).getValue())    AUHelpers::prepareExporter (exporter, projectSaver);
        }
    }

    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const
    {
        if (isVSTPluginHost (exporter.getProject()))
            VSTHelpers::createVSTPathEditor (exporter, props);

        if (isPluginClient())
        {
            if (shouldBuildVST (exporter.getProject()).getValue())   VSTHelpers::createPropertyEditors (exporter, props);
            if (shouldBuildRTAS (exporter.getProject()).getValue())  RTASHelpers::createPropertyEditors (exporter, props);
        }
    }

    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
    {
        const File header (getInclude());
        jassert (header.exists());

        StringArray lines;
        header.readLines (lines);

        for (int i = 0; i < lines.size(); ++i)
        {
            String line (lines[i].trim());

            if (line.startsWith ("/**") && line.containsIgnoreCase ("Config:"))
            {
                ScopedPointer <Project::ConfigFlag> config (new Project::ConfigFlag());
                config->sourceModuleID = getID();
                config->symbol = line.fromFirstOccurrenceOf (":", false, false).trim();

                if (config->symbol.length() > 2)
                {
                    ++i;

                    while (! (lines[i].contains ("*/") || lines[i].contains ("@see")))
                    {
                        if (lines[i].trim().isNotEmpty())
                            config->description = config->description.trim() + " " + lines[i].trim();

                        ++i;
                    }

                    config->description = config->description.upToFirstOccurrenceOf ("*/", false, false);
                    config->value.referTo (project.getConfigFlag (config->symbol));
                    flags.add (config.release());
                }
            }
        }
    }

    var moduleInfo;
    File moduleFile, moduleFolder;

private:
    mutable Array<File> sourceFiles;

    File getInclude() const
    {
        return moduleFolder.getChildFile (moduleInfo ["include"]);
    }

    String getPathToModuleFile (const File& file) const
    {
        return file.getRelativePathFrom (moduleFolder.getParentDirectory().getParentDirectory());
    }

    static bool fileTargetMatches (ProjectExporter& exporter, const String& target)
    {
        if (target == "xcode" && ! exporter.isXcode())
            return false;

        if (target == "msvc" && ! exporter.isVisualStudio())
            return false;

        return true;
    }

    struct FileSorter
    {
        static int compareElements (const File& f1, const File& f2)
        {
            return f1.getFileName().compareIgnoreCase (f2.getFileName());
        }
    };

    void findWildcardMatches (const String& wildcardPath, Array<File>& result) const
    {
        String path (wildcardPath.upToLastOccurrenceOf ("/", false, false));
        String wildCard (wildcardPath.fromLastOccurrenceOf ("/", false, false));

        DirectoryIterator iter (moduleFolder.getChildFile (path), false, wildCard);

        Array<File> tempList;

        FileSorter sorter;
        while (iter.next())
            tempList.addSorted (sorter, iter.getFile());

        result.addArray (tempList);
    }

    void getAllSourceFiles (Array<File>& result) const
    {
        const var filesArray (moduleInfo ["browse"]);
        const Array<var>* const files = filesArray.getArray();

        for (int i = 0; i < files->size(); ++i)
            findWildcardMatches (files->getReference(i), result);
    }

    void addFileWithGroups (Project::Item group, const File& file, const String& path) const
    {
        const int slash = path.indexOfChar ('/');

        if (slash >= 0)
        {
            const String topLevelGroup (path.substring (0, slash));
            const String remainingPath (path.substring (slash + 1));

            addFileWithGroups (group.getOrCreateSubGroup (topLevelGroup), file, remainingPath);
        }
        else
        {
            if (! group.findItemForFile (file).isValid())
                group.addFileUnchecked (file, -1, false);
        }
    }

    Project::Item addCompiledFile (const File& compiledFile, ProjectExporter& exporter, ProjectSaver& projectSaver) const
    {
        if (compiledFile.hasFileExtension ("cpp;cc;cxx;mm;m"))
        {
            MemoryOutputStream mem;
            writeSourceWrapper (mem, exporter.getProject(),
                                getPathToModuleFile (compiledFile));

            String wrapperName (compiledFile.getFileNameWithoutExtension());
            wrapperName << "_wrapper" << (exporter.usesMMFiles() ? ".mm" : ".cpp");

            return projectSaver.saveGeneratedFile (wrapperName, mem);
        }

        return projectSaver.addFileToGeneratedGroup (compiledFile);
    }

    void findAndAddCompiledCode (ProjectExporter& exporter, ProjectSaver& projectSaver, Array<File>& result) const
    {
        const var compileArray (moduleInfo ["compile"]); // careful to keep this alive while the array is in use!
        const Array<var>* const files = compileArray.getArray();

        if (files != nullptr)
        {
            for (int i = 0; i < files->size(); ++i)
            {
                const var& file = files->getReference(i);
                const String filename (file ["file"].toString());

                if (filename.isNotEmpty()
                     && fileTargetMatches (exporter, file ["target"].toString()))
                {
                    const File compiledFile (moduleFolder.getChildFile (filename));
                    result.add (compiledFile);

                    Project::Item item (addCompiledFile (compiledFile, exporter, projectSaver));

                    if (file ["warnings"].toString().equalsIgnoreCase ("disabled"))
                        item.getShouldInhibitWarningsValue() = true;

                    if (file ["stdcall"])
                        item.getShouldUseStdCallValue() = true;
                }
            }
        }
    }

    void addIncludedCode (ProjectExporter& exporter, const Array<File>& compiled) const
    {
        if (sourceFiles.size() == 0)
            getAllSourceFiles (sourceFiles);

        Project::Item sourceGroup (Project::Item::createGroup (exporter.getProject(), getID(), "__mainsourcegroup" + getID()));

        int i;
        for (i = 0; i < sourceFiles.size(); ++i)
            addFileWithGroups (sourceGroup, sourceFiles.getReference(i),
                               sourceFiles.getReference(i).getRelativePathFrom (moduleFolder));

        sourceGroup.addFile (moduleFile, -1, false);
        sourceGroup.addFile (getInclude(), -1, false);

        for (i = 0; i < compiled.size(); ++i)
            addFileWithGroups (sourceGroup, compiled.getReference(i),
                               compiled.getReference(i).getRelativePathFrom (moduleFolder));

        exporter.getModulesGroup().getNode().addChild (sourceGroup.getNode().createCopy(), -1, nullptr);
    }

    static void writeSourceWrapper (OutputStream& out, Project& project, const String& pathFromJuceFolder)
    {
        const String appConfigFileName (project.getAppConfigFilename());

        ProjectSaver::writeAutoGenWarningComment (out);

        out << "    This file pulls in a module's source code, and builds it using the settings" << newLine
            << "    defined in " << appConfigFileName << "." << newLine
            << newLine
            << "*/"
            << newLine
            << newLine
            << "#define JUCE_WRAPPED_FILE 1" << newLine
            << newLine
            << CodeHelpers::createIncludeStatement (appConfigFileName) << newLine;

        writeInclude (project, out, pathFromJuceFolder);
    }

    static void createMultipleIncludes (Project& project, const String& pathFromLibraryFolder,
                                        StringArray& paths, StringArray& guards)
    {
        for (int i = project.getNumExporters(); --i >= 0;)
        {
            ScopedPointer <ProjectExporter> exporter (project.createExporter (i));

            if (exporter != nullptr)
            {
                paths.add (exporter->getIncludePathForFileInJuceFolder (pathFromLibraryFolder, project.getAppIncludeFile()));
                guards.add ("defined (" + exporter->getExporterIdentifierMacro() + ")");
            }
        }
    }

    static void writeInclude (Project& project, OutputStream& out, const String& pathFromJuceFolder)
    {
        StringArray paths, guards;
        createMultipleIncludes (project, pathFromJuceFolder, paths, guards);

        StringArray uniquePaths (paths);
        uniquePaths.removeDuplicates (false);

        if (uniquePaths.size() == 1)
        {
            out << "#include " << paths[0] << newLine;
        }
        else
        {
            int i = paths.size();
            for (; --i >= 0;)
            {
                for (int j = i; --j >= 0;)
                {
                    if (paths[i] == paths[j] && guards[i] == guards[j])
                    {
                        paths.remove (i);
                        guards.remove (i);
                    }
                }
            }

            for (i = 0; i < paths.size(); ++i)
            {
                out << (i == 0 ? "#if " : "#elif ") << guards[i] << newLine
                    << " #include " << paths[i] << newLine;
            }

            out << "#endif" << newLine;
        }
    }

    bool isPluginClient() const                          { return getID() == "juce_audio_plugin_client"; }
    bool isAUPluginHost (const Project& project) const   { return getID() == "juce_audio_processors" && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_AU"); }
    bool isVSTPluginHost (const Project& project) const  { return getID() == "juce_audio_processors" && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_VST"); }
};

#endif   // __JUCER_JUCELIBRARYMODULE_JUCEHEADER__
