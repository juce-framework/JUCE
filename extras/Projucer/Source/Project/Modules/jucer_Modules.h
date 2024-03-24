/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

    bool isValid() const                    { return moduleDescription.isValid(); }
    String getID() const                    { return moduleDescription.getID(); }
    String getVendor() const                { return moduleDescription.getVendor(); }
    String getVersion() const               { return moduleDescription.getVersion(); }
    String getName() const                  { return moduleDescription.getName(); }
    String getDescription() const           { return moduleDescription.getDescription(); }
    String getLicense() const               { return moduleDescription.getLicense(); }
    String getMinimumCppStandard() const    { return moduleDescription.getMinimumCppStandard(); }

    File getFolder() const                  { return moduleDescription.getFolder(); }

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
    };

    Array<CompileUnit> getAllCompileUnits (build_tools::ProjectType::Target::Type forTarget =
                                               build_tools::ProjectType::Target::unspecified) const;
    void findAndAddCompiledUnits (ProjectExporter&, ProjectSaver*, Array<File>& result,
                                  build_tools::ProjectType::Target::Type forTarget =
                                      build_tools::ProjectType::Target::unspecified) const;

    ModuleDescription moduleDescription;

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

    CriticalSection stateLock;
    ValueTree state;

    std::unique_ptr<FileChooser> chooser;
    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModulesList)
};
