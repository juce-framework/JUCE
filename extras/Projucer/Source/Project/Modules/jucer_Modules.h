/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../jucer_Project.h"

class ProjectExporter;
class ProjectSaver;

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
        bool isCompiledForObjC = false, isCompiledForNonObjC = false;

        bool isNeededForExporter (ProjectExporter&) const;
        String getFilenameForProxyFile() const;
        static bool hasSuffix (const File&, const char*);
    };

    Array<CompileUnit> getAllCompileUnits (build_tools::ProjectType::Target::Type forTarget =
                                               build_tools::ProjectType::Target::unspecified) const;
    void findAndAddCompiledUnits (ProjectExporter&, ProjectSaver*, Array<File>& result,
                                  build_tools::ProjectType::Target::Type forTarget =
                                      build_tools::ProjectType::Target::unspecified) const;

    ModuleDescription moduleInfo;

private:
    void addSearchPathsToExporter (ProjectExporter&) const;
    void addDefinesToExporter (ProjectExporter&) const;
    void addCompileUnitsToExporter (ProjectExporter&, ProjectSaver&) const;
    void addLibsToExporter (ProjectExporter&) const;

    void addBrowseableCode (ProjectExporter&, const Array<File>& compiled, const File& localModuleFolder) const;

    mutable Array<File> sourceFiles;
    OwnedArray<Project::ConfigFlag> configFlags;
};

//==============================================================================
class EnabledModulesList
{
public:
    EnabledModulesList (Project&, const ValueTree&);

    //==============================================================================
    ValueTree getState() const              { return state; }

    StringArray getAllModules() const;
    void createRequiredModules (OwnedArray<LibraryModule>& modules);
    void sortAlphabetically();

    File getDefaultModulesFolder() const;

    int getNumModules() const               { return state.getNumChildren(); }
    String getModuleID (int index) const    { return state.getChild (index) [Ids::ID].toString(); }

    ModuleDescription getModuleInfo (const String& moduleID) const;

    bool isModuleEnabled (const String& moduleID) const;

    StringArray getExtraDependenciesNeeded (const String& moduleID) const;
    bool tryToFixMissingDependencies (const String& moduleID);

    bool doesModuleHaveHigherCppStandardThanProject (const String& moduleID) const;

    bool shouldUseGlobalPath (const String& moduleID) const;
    Value shouldUseGlobalPathValue (const String& moduleID) const;

    bool shouldShowAllModuleFilesInProject (const String& moduleID) const;
    Value shouldShowAllModuleFilesInProjectValue (const String& moduleID) const;

    bool shouldCopyModuleFilesLocally (const String& moduleID) const;
    Value shouldCopyModuleFilesLocallyValue (const String& moduleID) const;

    bool areMostModulesUsingGlobalPath() const;
    bool areMostModulesCopiedLocally() const;

    StringArray getModulesWithHigherCppStandardThanProject() const;
    StringArray getModulesWithMissingDependencies() const;

    String getHighestModuleCppStandard() const;

    //==============================================================================
    void addModule (const File& moduleManifestFile, bool copyLocally, bool useGlobalPath);
    void addModuleInteractive (const String& moduleID);
    void addModuleFromUserSelectedFile();
    void addModuleOfferingToCopy (const File&, bool isFromUserSpecifiedFolder);

    void removeModule (String moduleID);

private:
    UndoManager* getUndoManager() const     { return project.getUndoManagerFor (state); }

    Project& project;
    ValueTree state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModulesList)
};
