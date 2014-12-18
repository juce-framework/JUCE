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
File findDefaultModulesFolder (bool mustContainJuceCoreModule = true);
bool isJuceModulesFolder (const File&);
bool isJuceFolder (const File&);

//==============================================================================
struct ModuleDescription
{
    ModuleDescription() {}
    ModuleDescription (const File& manifest);
    ModuleDescription (const var& info)       : moduleInfo (info) {}

    bool isValid() const                { return getID().isNotEmpty(); }

    String getID() const                { return moduleInfo [Ids::ID].toString(); }
    String getVersion() const           { return moduleInfo [Ids::version].toString(); }
    String getName() const              { return moduleInfo [Ids::name].toString(); }
    String getDescription() const       { return moduleInfo [Ids::description].toString(); }
    String getLicense() const           { return moduleInfo [Ids::license].toString(); }
    String getHeaderName() const        { return moduleInfo [Ids::include].toString(); }
    String getPreprocessorDefs() const  { return moduleInfo [Ids::defines].toString(); }

    File getFolder() const              { jassert (manifestFile != File::nonexistent); return manifestFile.getParentDirectory(); }

    bool isPluginClient() const         { return getID() == "juce_audio_plugin_client"; }

    static const char* getManifestFileName()    { return "juce_module_info"; }

    var moduleInfo;
    File manifestFile;
    URL url;
};

//==============================================================================
struct ModuleList
{
    ModuleList();
    ModuleList (const ModuleList&);
    ModuleList& operator= (const ModuleList&);

    const ModuleDescription* getModuleWithID (const String& moduleID) const;
    StringArray getIDs() const;
    void sort();

    Result addAllModulesInFolder (const File&);
    Result scanAllKnownFolders (Project&);
    bool loadFromWebsite();

    OwnedArray<ModuleDescription> modules;
};

//==============================================================================
class LibraryModule
{
public:
    LibraryModule (const ModuleDescription&);

    bool isValid() const                { return moduleInfo.isValid(); }
    String getID() const                { return moduleInfo.getID(); }
    String getVersion() const           { return moduleInfo.getVersion(); }
    String getName() const              { return moduleInfo.getName(); }
    String getDescription() const       { return moduleInfo.getDescription(); }
    String getLicense() const           { return moduleInfo.getLicense(); }

    File getFolder() const              { return moduleInfo.getFolder(); }

    void writeIncludes (ProjectSaver&, OutputStream&);
    void prepareExporter (ProjectExporter&, ProjectSaver&) const;
    void createPropertyEditors (ProjectExporter&, PropertyListBuilder&) const;
    void getConfigFlags (Project&, OwnedArray<Project::ConfigFlag>& flags) const;
    void findBrowseableFiles (const File& localModuleFolder, Array<File>& files) const;

    ModuleDescription moduleInfo;

private:
    mutable Array<File> sourceFiles;

    File getModuleHeaderFile (const File& folder) const;

    void findWildcardMatches (const File& localModuleFolder, const String& wildcardPath, Array<File>& result) const;
    void findAndAddCompiledCode (ProjectExporter&, ProjectSaver&, const File& localModuleFolder, Array<File>& result) const;
    void addBrowsableCode (ProjectExporter&, ProjectSaver&, const Array<File>& compiled, const File& localModuleFolder) const;
    void createLocalHeaderWrapper (ProjectSaver&, const File& originalHeader, const File& localHeader) const;

    bool isAUPluginHost (const Project&) const;
    bool isVSTPluginHost (const Project&) const;
    bool isVST3PluginHost (const Project&) const;
};

//==============================================================================
class EnabledModuleList
{
public:
    EnabledModuleList (Project&, const ValueTree&);

    bool isModuleEnabled (const String& moduleID) const;
    Value shouldShowAllModuleFilesInProject (const String& moduleID);
    Value shouldCopyModuleFilesLocally (const String& moduleID) const;
    void removeModule (String moduleID);
    bool isAudioPluginModuleMissing() const;

    ModuleDescription getModuleInfo (const String& moduleID);

    File getModuleInfoFile (const String& moduleID);
    File getModuleFolder (const String& moduleID);

    void addModule (const File& moduleManifestFile, bool copyLocally);
    void addModuleInteractive (const String& moduleID);
    void addModuleFromUserSelectedFile();
    void addModuleOfferingToCopy (const File&);

    StringArray getAllModules() const;
    StringArray getExtraDependenciesNeeded (const String& moduleID) const;
    void createRequiredModules (OwnedArray<LibraryModule>& modules);

    int getNumModules() const               { return state.getNumChildren(); }
    String getModuleID (int index) const    { return state.getChild (index) [Ids::ID].toString(); }

    bool areMostModulesCopiedLocally() const;
    void setLocalCopyModeForAllModules (bool copyLocally);
    void sortAlphabetically();

    static File findDefaultModulesFolder (Project&);

    Project& project;
    ValueTree state;

private:
    UndoManager* getUndoManager() const     { return project.getUndoManagerFor (state); }

    File findLocalModuleInfoFile (const String& moduleID, bool useExportersForOtherOSes);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModuleList)
};


#endif   // __JUCER_MODULE_JUCEHEADER__
