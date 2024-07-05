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

#include "../../Application/jucer_Headers.h"
#include "../../ProjectSaving/jucer_ProjectSaver.h"
#include "../../ProjectSaving/jucer_ProjectExport_Xcode.h"
#include "../../Application/jucer_Application.h"

//==============================================================================
LibraryModule::LibraryModule (const ModuleDescription& d)
    : moduleDescription (d)
{
}

void LibraryModule::writeIncludes (ProjectSaver& projectSaver, OutputStream& out)
{
    auto& project = projectSaver.getProject();
    auto& modules = project.getEnabledModules();

    auto moduleID = getID();

    if (modules.shouldCopyModuleFilesLocally (moduleID))
    {
        auto juceModuleFolder = moduleDescription.getFolder();

        auto localModuleFolder = project.getLocalModuleFolder (moduleID);
        localModuleFolder.createDirectory();
        projectSaver.copyFolder (juceModuleFolder, localModuleFolder);
    }

    out << "#include <" << moduleDescription.getModuleFolder().getFileName() << "/"
        << moduleDescription.getHeader().getFileName()
        << ">" << newLine;
}

void LibraryModule::addSearchPathsToExporter (ProjectExporter& exporter) const
{
    auto moduleRelativePath = exporter.getModuleFolderRelativeToProject (getID());

    exporter.addToExtraSearchPaths (moduleRelativePath.getParentDirectory());

    const auto libDirPlatform = [&]() -> String
    {
        if (exporter.isLinux())
            return "Linux";

        return exporter.getTypeInfoForExporter (exporter.getExporterIdentifier()).targetFolder;
    }();

    auto libSubdirPath = moduleRelativePath.toUnixStyle() + "/libs/" + libDirPlatform;
    auto moduleLibDir = exporter.getProject().resolveFilename (libSubdirPath);

    if (moduleLibDir.exists())
        exporter.addToModuleLibPaths ({ libSubdirPath, moduleRelativePath.getRoot() });

    auto extraInternalSearchPaths = moduleDescription.getExtraSearchPaths().trim();

    if (extraInternalSearchPaths.isNotEmpty())
    {
        auto paths = StringArray::fromTokens (extraInternalSearchPaths, true);

        for (auto& path : paths)
            exporter.addToExtraSearchPaths (moduleRelativePath.getChildFile (path.unquoted()));
    }
}

void LibraryModule::addDefinesToExporter (ProjectExporter& exporter) const
{
    auto extraDefs = moduleDescription.getPreprocessorDefs().trim();

    if (extraDefs.isNotEmpty())
        exporter.getExporterPreprocessorDefsValue() = exporter.getExporterPreprocessorDefsString() + "\n" + extraDefs;
}

void LibraryModule::addCompileUnitsToExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
{
    auto& project = exporter.getProject();
    auto& modules = project.getEnabledModules();

    auto moduleID = getID();

    auto localModuleFolder = modules.shouldCopyModuleFilesLocally (moduleID) ? project.getLocalModuleFolder (moduleID)
                                                                             : moduleDescription.getFolder();

    Array<File> compiled;
    findAndAddCompiledUnits (exporter, &projectSaver, compiled);

    if (modules.shouldShowAllModuleFilesInProject (moduleID))
        addBrowseableCode (exporter, compiled, localModuleFolder);
}

void LibraryModule::addLibsToExporter (ProjectExporter& exporter) const
{
    auto parseAndAddLibsToList = [] (StringArray& libList, const String& libs)
    {
        libList.addTokens (libs, ", ", {});
        libList.trim();
        libList.removeDuplicates (false);
    };

    auto& project = exporter.getProject();

    auto moduleInfo = moduleDescription.getModuleInfo();

    if (exporter.isXcode())
    {
        auto& xcodeExporter = dynamic_cast<XcodeProjectExporter&> (exporter);

        if (project.isAUPluginHost())
        {
            xcodeExporter.xcodeFrameworks.add ("CoreAudioKit");

            if (xcodeExporter.isOSX())
                xcodeExporter.xcodeFrameworks.add ("AudioUnit");
        }

        auto frameworks = moduleInfo[xcodeExporter.isOSX() ? "OSXFrameworks" : "iOSFrameworks"].toString();
        xcodeExporter.xcodeFrameworks.addTokens (frameworks, ", ", {});

        auto weakFrameworks = moduleInfo[xcodeExporter.isOSX() ? "WeakOSXFrameworks" : "WeakiOSFrameworks"].toString();
        xcodeExporter.xcodeWeakFrameworks.addTokens (weakFrameworks, ", ", {});

        parseAndAddLibsToList (xcodeExporter.xcodeLibs, moduleInfo[exporter.isOSX() ? "OSXLibs" : "iOSLibs"].toString());
    }
    else if (exporter.isLinux())
    {
        parseAndAddLibsToList (exporter.linuxLibs, moduleInfo["linuxLibs"].toString());
        parseAndAddLibsToList (exporter.linuxPackages, moduleInfo["linuxPackages"].toString());
    }
    else if (exporter.isWindows())
    {
        parseAndAddLibsToList (exporter.windowsLibs, moduleInfo["windowsLibs"].toString());
    }
    else if (exporter.isAndroid())
    {
        parseAndAddLibsToList (exporter.androidLibs, moduleInfo["androidLibs"].toString());
    }
}

void LibraryModule::addSettingsForModuleToExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
{
    addSearchPathsToExporter (exporter);
    addDefinesToExporter (exporter);
    addCompileUnitsToExporter (exporter, projectSaver);
    addLibsToExporter (exporter);
}

void LibraryModule::getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
{
    auto header = moduleDescription.getHeader();
    jassert (header.exists());

    StringArray lines;
    header.readLines (lines);

    for (int i = 0; i < lines.size(); ++i)
    {
        auto line = lines[i].trim();

        if (line.startsWith ("/**") && line.containsIgnoreCase ("Config:"))
        {
            auto config = std::make_unique<Project::ConfigFlag>();
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
                config->value = project.getConfigFlag (config->symbol);

                i += 2;

                if (lines[i].contains ("#define " + config->symbol))
                {
                    auto value = lines[i].fromFirstOccurrenceOf ("#define " + config->symbol, false, true).trim();
                    config->value.setDefault (value == "0" ? false : true);
                }

                auto currentValue = config->value.get().toString();

                if      (currentValue == "enabled")     config->value = true;
                else if (currentValue == "disabled")    config->value = false;

                flags.add (std::move (config));
            }
        }
    }
}

static void addFileWithGroups (Project::Item& group, const build_tools::RelativePath& file, const String& path)
{
    auto slash = path.indexOfChar (File::getSeparatorChar());

    if (slash >= 0)
    {
        auto topLevelGroup = path.substring (0, slash);
        auto remainingPath = path.substring (slash + 1);

        auto newGroup = group.getOrCreateSubGroup (topLevelGroup);
        addFileWithGroups (newGroup, file, remainingPath);
    }
    else
    {
        if (! group.containsChildForFile (file))
            group.addRelativeFile (file, -1, false);
    }
}

struct FileSorter
{
    static int compareElements (const File& f1, const File& f2)
    {
        return f1.getFileName().compareNatural (f2.getFileName());
    }
};

void LibraryModule::findBrowseableFiles (const File& folder, Array<File>& filesFound) const
{
    Array<File> tempList;
    FileSorter sorter;

    for (const auto& iter : RangedDirectoryIterator (folder, true, "*", File::findFiles))
        if (! iter.isHidden() && iter.getFile().hasFileExtension (browseableFileExtensions))
            tempList.addSorted (sorter, iter.getFile());

    filesFound.addArray (tempList);
}

bool LibraryModule::CompileUnit::isNeededForExporter (ProjectExporter& exporter) const
{
    const auto trimmedFileNameLowercase = file.getFileNameWithoutExtension().toLowerCase();

    const std::tuple<const char*, bool> shouldBuildForSuffix[] { { "_android", exporter.isAndroid() },
                                                                 { "_ios",     exporter.isiOS() },
                                                                 { "_linux",   exporter.isLinux() },
                                                                 { "_mac",     exporter.isOSX() },
                                                                 { "_osx",     exporter.isOSX() },
                                                                 { "_windows", exporter.isWindows() } };

    for (const auto& [suffix, shouldBuild] : shouldBuildForSuffix)
        if (trimmedFileNameLowercase.endsWith (suffix))
            return shouldBuild;

    const auto targetType = Project::getTargetTypeFromFilePath (file, false);

    if (targetType != build_tools::ProjectType::Target::unspecified && ! exporter.shouldBuildTargetType (targetType))
        return false;

    return exporter.usesMMFiles() ? isCompiledForObjC
                                  : isCompiledForNonObjC;
}

String LibraryModule::CompileUnit::getFilenameForProxyFile() const
{
    return "include_" + file.getFileName();
}

Array<LibraryModule::CompileUnit> LibraryModule::getAllCompileUnits (build_tools::ProjectType::Target::Type forTarget) const
{
    auto files = getFolder().findChildFiles (File::findFiles, false);

    FileSorter sorter;
    files.sort (sorter);

    Array<LibraryModule::CompileUnit> units;

    for (auto& file : files)
    {
        if (file.getFileName().startsWithIgnoreCase (getID())
              && file.hasFileExtension (sourceFileExtensions))
        {
            if (forTarget == build_tools::ProjectType::Target::unspecified
             || forTarget == Project::getTargetTypeFromFilePath (file, true))
            {
                CompileUnit cu;
                cu.file = file;
                units.add (std::move (cu));
            }
        }
    }

    for (auto& cu : units)
    {
        cu.isCompiledForObjC = true;
        cu.isCompiledForNonObjC = ! cu.file.hasFileExtension ("mm;m;metal");

        if (cu.isCompiledForNonObjC)
            if (cu.file.withFileExtension ("mm").existsAsFile())
                cu.isCompiledForObjC = false;

        jassert (cu.isCompiledForObjC || cu.isCompiledForNonObjC);
    }

    return units;
}

void LibraryModule::findAndAddCompiledUnits (ProjectExporter& exporter,
                                             ProjectSaver* projectSaver,
                                             Array<File>& result,
                                             build_tools::ProjectType::Target::Type forTarget) const
{
    for (auto& cu : getAllCompileUnits (forTarget))
    {
        if (cu.isNeededForExporter (exporter))
        {
            auto localFile = exporter.getProject().getGeneratedCodeFolder()
                                                  .getChildFile (cu.getFilenameForProxyFile());
            result.add (localFile);

            if (projectSaver != nullptr)
                projectSaver->addFileToGeneratedGroup (localFile);
        }
    }
}

void LibraryModule::addBrowseableCode (ProjectExporter& exporter, const Array<File>& compiled, const File& localModuleFolder) const
{
    if (sourceFiles.isEmpty())
        findBrowseableFiles (localModuleFolder, sourceFiles);

    auto sourceGroup       = Project::Item::createGroup (exporter.getProject(), getID(), "__mainsourcegroup" + getID(), false);
    auto moduleFromProject = exporter.getModuleFolderRelativeToProject (getID());
    auto moduleHeader      = moduleDescription.getHeader();

    auto& project = exporter.getProject();

    if (project.getEnabledModules().shouldCopyModuleFilesLocally (getID()))
        moduleHeader = project.getLocalModuleFolder (getID()).getChildFile (moduleHeader.getFileName());

    auto isModuleHeader = [&] (const File& f)  { return f.getFileName() == moduleHeader.getFileName(); };

    for (auto& sourceFile : sourceFiles)
    {
        auto pathWithinModule = build_tools::getRelativePathFrom (sourceFile, localModuleFolder);

        // (Note: in exporters like MSVC we have to avoid adding the same file twice, even if one of those instances
        // is flagged as being excluded from the build, because this overrides the other and it fails to compile)
        if ((exporter.canCopeWithDuplicateFiles() || ! compiled.contains (sourceFile)) && ! isModuleHeader (sourceFile))
            addFileWithGroups (sourceGroup, moduleFromProject.getChildFile (pathWithinModule), pathWithinModule);
    }

    sourceGroup.sortAlphabetically (true, true);
    sourceGroup.addFileAtIndex (moduleHeader, -1, false);

    exporter.getModulesGroup().state.appendChild (sourceGroup.state.createCopy(), nullptr);
}

//==============================================================================
EnabledModulesList::EnabledModulesList (Project& p, const ValueTree& s)
    : project (p), state (s)
{
}

StringArray EnabledModulesList::getAllModules() const
{
    StringArray moduleIDs;

    for (int i = 0; i < getNumModules(); ++i)
        moduleIDs.add (getModuleID (i));

    return moduleIDs;
}

void EnabledModulesList::createRequiredModules (OwnedArray<LibraryModule>& modules)
{
    for (int i = 0; i < getNumModules(); ++i)
        modules.add (new LibraryModule (getModuleInfo (getModuleID (i))));
}

void EnabledModulesList::sortAlphabetically()
{
    struct ModuleTreeSorter
    {
        static int compareElements (const ValueTree& m1, const ValueTree& m2)
        {
            return m1[Ids::ID].toString().compareIgnoreCase (m2[Ids::ID]);
        }
    };

    ModuleTreeSorter sorter;

    const ScopedLock sl (stateLock);
    state.sort (sorter, getUndoManager(), false);
}

File EnabledModulesList::getDefaultModulesFolder() const
{
    File globalPath (getAppSettings().getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS()).get().toString());

    if (globalPath.exists())
        return globalPath;

    for (auto& exporterPathModule : project.getExporterPathsModulesList().getAllModules())
    {
        auto f = exporterPathModule.second;

        if (f.isDirectory())
            return f.getParentDirectory();
    }

    return File::getCurrentWorkingDirectory();
}

ModuleDescription EnabledModulesList::getModuleInfo (const String& moduleID) const
{
    return ModuleDescription (project.getModuleWithID (moduleID).second);
}

bool EnabledModulesList::isModuleEnabled (const String& moduleID) const
{
    const ScopedLock sl (stateLock);
    return state.getChildWithProperty (Ids::ID, moduleID).isValid();
}

static void getDependencies (Project& project, const String& moduleID, StringArray& dependencies)
{
    auto info = project.getEnabledModules().getModuleInfo (moduleID);

    for (auto uid : info.getDependencies())
    {
        if (! dependencies.contains (uid, true))
        {
            dependencies.add (uid);
            getDependencies (project, uid, dependencies);
        }
    }
}

StringArray EnabledModulesList::getExtraDependenciesNeeded (const String& moduleID) const
{
    StringArray dependencies, extraDepsNeeded;
    getDependencies (project, moduleID, dependencies);

    for (auto dep : dependencies)
        if (dep != moduleID && ! isModuleEnabled (dep))
            extraDepsNeeded.add (dep);

    return extraDepsNeeded;
}

bool EnabledModulesList::tryToFixMissingDependencies (const String& moduleID)
{
    auto copyLocally = areMostModulesCopiedLocally();
    auto useGlobalPath = areMostModulesUsingGlobalPath();

    StringArray missing;

    for (auto missingModule : getExtraDependenciesNeeded (moduleID))
    {
        auto mod = project.getModuleWithID (missingModule);

        if (mod.second != File())
            addModule (mod.second, copyLocally, useGlobalPath);
        else
            missing.add (missingModule);
    }

    return (missing.size() == 0);
}

bool EnabledModulesList::doesModuleHaveHigherCppStandardThanProject (const String& moduleID) const
{
    auto projectCppStandard = project.getCppStandardString();

    if (projectCppStandard == Project::getCppStandardVars().getLast().toString())
        return false;

    auto moduleCppStandard = getModuleInfo (moduleID).getMinimumCppStandard();

    return (moduleCppStandard.getIntValue() > projectCppStandard.getIntValue());
}

bool EnabledModulesList::shouldUseGlobalPath (const String& moduleID) const
{
    const ScopedLock sl (stateLock);
    return (bool) shouldUseGlobalPathValue (moduleID).getValue();
}

Value EnabledModulesList::shouldUseGlobalPathValue (const String& moduleID) const
{
    const ScopedLock sl (stateLock);
    return state.getChildWithProperty (Ids::ID, moduleID)
                .getPropertyAsValue (Ids::useGlobalPath, getUndoManager());
}

bool EnabledModulesList::shouldShowAllModuleFilesInProject (const String& moduleID) const
{
    return (bool) shouldShowAllModuleFilesInProjectValue (moduleID).getValue();
}

Value EnabledModulesList::shouldShowAllModuleFilesInProjectValue (const String& moduleID) const
{
    const ScopedLock sl (stateLock);
    return state.getChildWithProperty (Ids::ID, moduleID)
                .getPropertyAsValue (Ids::showAllCode, getUndoManager());
}

bool EnabledModulesList::shouldCopyModuleFilesLocally (const String& moduleID) const
{
    return (bool) shouldCopyModuleFilesLocallyValue (moduleID).getValue();
}

Value EnabledModulesList::shouldCopyModuleFilesLocallyValue (const String& moduleID) const
{
    const ScopedLock sl (stateLock);
    return state.getChildWithProperty (Ids::ID, moduleID)
                .getPropertyAsValue (Ids::useLocalCopy, getUndoManager());
}

bool EnabledModulesList::areMostModulesUsingGlobalPath() const
{
    int numYes = 0, numNo = 0;

    for (auto i = getNumModules(); --i >= 0;)
    {
        if (shouldUseGlobalPath (getModuleID (i)))
            ++numYes;
        else
            ++numNo;
    }

    return numYes > numNo;
}

bool EnabledModulesList::areMostModulesCopiedLocally() const
{
    int numYes = 0, numNo = 0;

    for (auto i = getNumModules(); --i >= 0;)
    {
        if (shouldCopyModuleFilesLocally (getModuleID (i)))
            ++numYes;
        else
            ++numNo;
    }

    return numYes > numNo;
}

StringArray EnabledModulesList::getModulesWithHigherCppStandardThanProject() const
{
    StringArray list;

    for (auto& module : getAllModules())
        if (doesModuleHaveHigherCppStandardThanProject (module))
            list.add (module);

    return list;
}

StringArray EnabledModulesList::getModulesWithMissingDependencies() const
{
    StringArray list;

    for (auto& module : getAllModules())
        if (getExtraDependenciesNeeded (module).size() > 0)
            list.add (module);

    return list;
}

String EnabledModulesList::getHighestModuleCppStandard() const
{
    auto highestCppStandard = Project::getCppStandardVars()[0].toString();

    for (auto& mod : getAllModules())
    {
        auto moduleCppStandard = getModuleInfo (mod).getMinimumCppStandard();

        if (moduleCppStandard == "latest")
            return moduleCppStandard;

        if (moduleCppStandard.getIntValue() > highestCppStandard.getIntValue())
            highestCppStandard = moduleCppStandard;
    }

    return highestCppStandard;
}

void EnabledModulesList::addModule (const File& moduleFolder, bool copyLocally, bool useGlobalPath)
{
    ModuleDescription info (moduleFolder);

    if (info.isValid())
    {
        auto moduleID = info.getID();

        if (! isModuleEnabled (moduleID))
        {
            ValueTree module (Ids::MODULE);
            module.setProperty (Ids::ID, moduleID, getUndoManager());

            {
                const ScopedLock sl (stateLock);
                state.appendChild (module, getUndoManager());
            }

            sortAlphabetically();

            shouldShowAllModuleFilesInProjectValue (moduleID) = true;
            shouldCopyModuleFilesLocallyValue (moduleID) = copyLocally;
            shouldUseGlobalPathValue (moduleID) = useGlobalPath;

            build_tools::RelativePath path (moduleFolder.getParentDirectory(),
                                            project.getProjectFolder(),
                                            build_tools::RelativePath::projectFolder);

            for (Project::ExporterIterator exporter (project); exporter.next();)
                exporter->getPathForModuleValue (moduleID) = path.toUnixStyle();

            if (! useGlobalPath)
                project.rescanExporterPathModules (false);
        }
    }
}

void EnabledModulesList::addModuleInteractive (const String& moduleID)
{
    auto f = project.getModuleWithID (moduleID).second;

    if (f != File())
    {
        addModule (f, areMostModulesCopiedLocally(), areMostModulesUsingGlobalPath());
        return;
    }

    addModuleFromUserSelectedFile();
}

void EnabledModulesList::addModuleFromUserSelectedFile()
{
    chooser = std::make_unique<FileChooser> ("Select a module to add...", getDefaultModulesFolder(), "");
    auto flags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (flags, [this] (const FileChooser& fc)
    {
        if (fc.getResult() == File{})
            return;

        addModuleOfferingToCopy (fc.getResult(), true);
    });
}

void EnabledModulesList::addModuleOfferingToCopy (const File& f, bool isFromUserSpecifiedFolder)
{
    ModuleDescription m (f);

    if (! m.isValid())
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                         "Add Module",
                                                         "This wasn't a valid module folder!");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
        return;
    }

    if (isModuleEnabled (m.getID()))
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::InfoIcon,
                                                         "Add Module",
                                                         "The project already contains this module!");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);
        return;
    }

    addModule (m.getModuleFolder(),
               areMostModulesCopiedLocally(),
               isFromUserSpecifiedFolder ? false : areMostModulesUsingGlobalPath());
}

void EnabledModulesList::removeModule (String moduleID) // must be pass-by-value, and not a const ref!
{
    {
        const ScopedLock sl (stateLock);

        for (auto i = state.getNumChildren(); --i >= 0;)
            if (state.getChild (i) [Ids::ID] == moduleID)
                state.removeChild (i, getUndoManager());
    }

    for (Project::ExporterIterator exporter (project); exporter.next();)
        exporter->removePathForModule (moduleID);
}
