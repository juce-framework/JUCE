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
    LibraryModule (const var& moduleInfo);

    bool isValid() const;

    String getID() const                { return moduleInfo ["id"].toString(); }
    String getVersion() const           { return moduleInfo ["version"].toString(); }
    String getName() const              { return moduleInfo ["name"].toString(); }
    String getDescription() const       { return moduleInfo ["description"].toString(); }
    const File& getFolder() const       { return moduleFolder; }

    void writeIncludes (ProjectSaver&, OutputStream&);
    void prepareExporter (ProjectExporter&, ProjectSaver&) const;
    void createPropertyEditors (ProjectExporter&, PropertyListBuilder&) const;
    void getConfigFlags (Project&, OwnedArray<Project::ConfigFlag>& flags) const;
    void getLocalCompiledFiles (const File& localModuleFolder, Array<File>& files) const;
    void findBrowseableFiles (const File& localModuleFolder, Array<File>& files) const;
    File getLocalFolderFor (Project&) const;

    static String getInfoFileName()     { return "juce_module_info"; }

    var moduleInfo;

private:
    File moduleFile, moduleFolder;
    mutable Array<File> sourceFiles;

    File getInclude (const File& folder) const;
    static bool fileTargetMatches (ProjectExporter& exporter, const String& target);

    struct FileSorter
    {
        static int compareElements (const File& f1, const File& f2)
        {
            return f1.getFileName().compareIgnoreCase (f2.getFileName());
        }
    };

    void findWildcardMatches (const File& localModuleFolder, const String& wildcardPath, Array<File>& result) const;
    void findAndAddCompiledCode (ProjectExporter&, ProjectSaver&, const File& localModuleFolder, Array<File>& result) const;
    void addBrowsableCode (ProjectExporter&, const Array<File>& compiled, const File& localModuleFolder) const;
    void createLocalHeaderWrapper (ProjectSaver&, const File& originalHeader, const File& localHeader) const;
    RelativePath getModuleRelativeToProject (ProjectExporter&) const;
    RelativePath getModuleOrLocalCopyRelativeToProject (ProjectExporter&, const File& localModuleFolder) const;

    bool isPluginClient() const;
    bool isAUPluginHost (const Project&) const;
    bool isVSTPluginHost (const Project&) const;
};

//==============================================================================
class ModuleList
{
public:
    ModuleList();
    ModuleList (const ModuleList&);
    ModuleList& operator= (const ModuleList&);

    //==============================================================================
    Result rescan (const File& newModulesFolder);
    void rescan();
    File getModulesFolder() const     { return moduleFolder; }

    bool loadFromWebsite();

    LibraryModule* loadModule (const String& uid) const;

    void getDependencies (const String& moduleID, StringArray& dependencies) const;
    void createDependencies (const String& moduleID, OwnedArray<LibraryModule>& modules) const;

    //==============================================================================
    struct Module
    {
        LibraryModule* create() const;

        String uid, version, name, description;
        File file;
        URL url;

        bool operator== (const Module&) const;
        bool operator!= (const Module&) const;
    };

    const Module* findModuleInfo (const String& uid) const;

    bool operator== (const ModuleList&) const;

    //==============================================================================
    static bool isJuceFolder (const File& folder);
    static bool isModulesFolder (const File& folder);
    static bool isJuceOrModulesFolder (const File& folder);

    static File getDefaultModulesFolder (Project*);
    static bool isLocalModulesFolderValid();
    static bool isLibraryNewerThanIntrojucer();

    static File getLocalModulesFolder (Project*);
    static void setLocalModulesFolder (const File& newFile);

    static File getModulesFolderForJuceOrModulesFolder (const File& f);
    static File getModulesFolderForExporter (const ProjectExporter&);

    StringArray getExtraDependenciesNeeded (Project&, const Module&);

    //==============================================================================
    OwnedArray<Module> modules;

private:
    File moduleFolder;

    void sort();
};


#endif   // __JUCER_MODULE_JUCEHEADER__
