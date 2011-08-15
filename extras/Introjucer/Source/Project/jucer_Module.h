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

#ifndef __JUCER_MODULE_JUCEHEADER__
#define __JUCER_MODULE_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"
class ProjectExporter;
class ProjectSaver;

//==============================================================================
class LibraryModule
{
public:
    LibraryModule (const File& file);

    String getID() const; 
    bool isValid() const;

    void writeIncludes (ProjectSaver& projectSaver, OutputStream& out);
    void prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const;
    void createPropertyEditors (const ProjectExporter& exporter, Array <PropertyComponent*>& props) const;
    void getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const;

    var moduleInfo;

private:
    File moduleFile, moduleFolder;
    mutable Array<File> sourceFiles;

    File getInclude (const File& folder) const;
    File getModuleTargetFolder (ProjectSaver& projectSaver) const;
    String getPathToModuleFile (ProjectSaver& projectSaver, const File& file) const;
    static bool fileTargetMatches (ProjectExporter& exporter, const String& target);

    struct FileSorter
    {
        static int compareElements (const File& f1, const File& f2)
        {
            return f1.getFileName().compareIgnoreCase (f2.getFileName());
        }
    };

    void findWildcardMatches (const File& localModuleFolder, const String& wildcardPath, Array<File>& result) const;
    void addFileWithGroups (Project::Item& group, const File& file, const String& path) const;
    void findAndAddCompiledCode (ProjectExporter& exporter, ProjectSaver& projectSaver, const File& localModuleFolder, Array<File>& result) const;
    void addBrowsableCode (ProjectExporter& exporter, const Array<File>& compiled, const File& localModuleFolder) const;

    static void writeSourceWrapper (OutputStream& out, Project& project, const String& pathFromJuceFolder);
    static void createMultipleIncludes (Project& project, const String& pathFromLibraryFolder, StringArray& paths, StringArray& guards);
    static void writeInclude (Project& project, OutputStream& out, const String& pathFromJuceFolder);

    bool isPluginClient() const;
    bool isAUPluginHost (const Project& project) const;
    bool isVSTPluginHost (const Project& project) const;
};

//==============================================================================
class ModuleList
{
public:
    ModuleList();

    static ModuleList& getInstance();

    //==============================================================================
    void rescan();
    LibraryModule* loadModule (const String& uid) const;

    void getDependencies (const String& moduleID, StringArray& dependencies) const;
    void createDependencies (const String& moduleID, OwnedArray<LibraryModule>& modules) const;

    //==============================================================================
    struct Module
    {
        LibraryModule* create() const;

        String uid, name, description;
        File file;
    };

    const Module* findModuleInfo (const String& uid) const;

    OwnedArray<Module> modules;

private:
    File moduleFolder;
};


#endif   // __JUCER_MODULE_JUCEHEADER__
