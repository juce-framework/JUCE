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

#pragma once

#include "jucer_Project.h"
class ProjectExporter;
class ProjectSaver;

//==============================================================================
bool isJUCEModulesFolder (const File&);
bool isJUCEFolder (const File&);

//==============================================================================
struct ModuleDescription
{
    ModuleDescription() {}
    ModuleDescription (const File& folder);
    ModuleDescription (const var& info) : moduleInfo (info) {}

    bool isValid() const                    { return getID().isNotEmpty(); }

    String getID() const                    { return moduleInfo [Ids::ID_uppercase].toString(); }
    String getVendor() const                { return moduleInfo [Ids::vendor].toString(); }
    String getVersion() const               { return moduleInfo [Ids::version].toString(); }
    String getName() const                  { return moduleInfo [Ids::name].toString(); }
    String getDescription() const           { return moduleInfo [Ids::description].toString(); }
    String getLicense() const               { return moduleInfo [Ids::license].toString(); }
    String getMinimumCppStandard() const    { return moduleInfo [Ids::minimumCppStandard].toString(); }
    String getPreprocessorDefs() const      { return moduleInfo [Ids::defines].toString(); }
    String getExtraSearchPaths() const      { return moduleInfo [Ids::searchpaths].toString(); }
    StringArray getDependencies() const;

    File getFolder() const                  { jassert (moduleFolder != File()); return moduleFolder; }
    File getHeader() const;

    bool isPluginClient() const             { return getID() == "juce_audio_plugin_client"; }

    File moduleFolder;
    var moduleInfo;
    URL url;
};

//==============================================================================
class LibraryModule
{
public:
    LibraryModule (const ModuleDescription&);

    bool isValid() const                    { return moduleInfo.isValid(); }
    String getID() const                    { return moduleInfo.getID(); }
    String getVendor() const                { return moduleInfo.getVendor(); }
    String getVersion() const               { return moduleInfo.getVersion(); }
    String getName() const                  { return moduleInfo.getName(); }
    String getDescription() const           { return moduleInfo.getDescription(); }
    String getLicense() const               { return moduleInfo.getLicense(); }
    String getMinimumCppStandard() const    { return moduleInfo.getMinimumCppStandard(); }

    File getFolder() const                  { return moduleInfo.getFolder(); }

    void writeIncludes (ProjectSaver&, OutputStream&);
    void addSettingsForModuleToExporter (ProjectExporter&, ProjectSaver&) const;
    void getConfigFlags (Project&, OwnedArray<Project::ConfigFlag>& flags) const;
    void findBrowseableFiles (const File& localModuleFolder, Array<File>& files) const;

    struct CompileUnit
    {
        File file;
        bool isCompiledForObjC, isCompiledForNonObjC;

        void writeInclude (MemoryOutputStream&) const;
        bool isNeededForExporter (ProjectExporter&) const;
        String getFilenameForProxyFile() const;
        static bool hasSuffix (const File&, const char*);
    };

    Array<CompileUnit> getAllCompileUnits (ProjectType::Target::Type forTarget = ProjectType::Target::unspecified) const;
    void findAndAddCompiledUnits (ProjectExporter&, ProjectSaver*, Array<File>& result,
                                  ProjectType::Target::Type forTarget = ProjectType::Target::unspecified) const;

    ModuleDescription moduleInfo;

private:
    mutable Array<File> sourceFiles;
    OwnedArray<Project::ConfigFlag> configFlags;

    void addBrowseableCode (ProjectExporter&, const Array<File>& compiled, const File& localModuleFolder) const;
};

//==============================================================================
using ModuleIDAndFolder     = std::pair<String, File>;
using ModuleIDAndFolderList = std::vector<ModuleIDAndFolder>;

class AvailableModuleList
{
public:
    AvailableModuleList();

    void scanPaths      (const Array<File>&);
    void scanPathsAsync (const Array<File>&);

    ModuleIDAndFolderList getAllModules() const;
    ModuleIDAndFolder getModuleWithID (const String&) const;

    void removeDuplicates (const ModuleIDAndFolderList& other);

    //==============================================================================
    struct Listener
    {
        virtual ~Listener() {}

        virtual void availableModulesChanged() = 0;
    };

    void addListener (Listener* listenerToAdd)          { listeners.add (listenerToAdd); }
    void removeListener (Listener* listenerToRemove)    { listeners.remove (listenerToRemove); }

private:
    ThreadPoolJob* createScannerJob (const Array<File>&);
    void removePendingAndAddJob (ThreadPoolJob*);

    ThreadPool scanPool { 1 };

    ModuleIDAndFolderList moduleList;
    ListenerList<Listener> listeners;
    CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AvailableModuleList)
};

//==============================================================================
class EnabledModuleList
{
public:
    EnabledModuleList (Project&, const ValueTree&);

    static File findDefaultModulesFolder (Project&);

    bool isModuleEnabled (const String& moduleID) const;

    bool shouldUseGlobalPath (const String& moduleID) const;
    Value getShouldUseGlobalPathValue (const String& moduleID) const;

    Value shouldShowAllModuleFilesInProject (const String& moduleID);
    Value shouldCopyModuleFilesLocally (const String& moduleID) const;

    void removeModule (String moduleID);
    bool isAudioPluginModuleMissing() const;

    ModuleDescription getModuleInfo (const String& moduleID);

    void addModule (const File& moduleManifestFile, bool copyLocally, bool useGlobalPath, bool sendAnalyticsEvent);
    void addModuleInteractive (const String& moduleID);
    void addModuleFromUserSelectedFile();
    void addModuleOfferingToCopy (const File&, bool isFromUserSpecifiedFolder);

    StringArray getAllModules() const;
    StringArray getExtraDependenciesNeeded (const String& moduleID) const;
    bool doesModuleHaveHigherCppStandardThanProject (const String& moduleID);
    void createRequiredModules (OwnedArray<LibraryModule>& modules);

    int getNumModules() const               { return state.getNumChildren(); }
    String getModuleID (int index) const    { return state.getChild (index) [Ids::ID].toString(); }

    bool areMostModulesUsingGlobalPath() const;
    bool areMostModulesCopiedLocally() const;

    void setLocalCopyModeForAllModules (bool copyLocally);

    void sortAlphabetically();

    Project& project;
    ValueTree state;

private:
    UndoManager* getUndoManager() const     { return project.getUndoManagerFor (state); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModuleList)
};
