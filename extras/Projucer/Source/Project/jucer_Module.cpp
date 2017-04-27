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

#include "../jucer_Headers.h"
#include "jucer_Module.h"
#include "jucer_ProjectType.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Project Saving/jucer_ProjectSaver.h"
#include "../Project Saving/jucer_ProjectExport_XCode.h"


static String trimCommentCharsFromStartOfLine (const String& line)
{
    return line.trimStart().trimCharactersAtStart ("*/").trimStart();
}

static var parseModuleDesc (const StringArray& lines)
{
    DynamicObject* o = new DynamicObject();
    var result (o);

    for (int i = 0; i < lines.size(); ++i)
    {
        String line = trimCommentCharsFromStartOfLine (lines[i]);

        int colon = line.indexOfChar (':');

        if (colon >= 0)
        {
            String key = line.substring (0, colon).trim();
            String value = line.substring (colon + 1).trim();

            o->setProperty (key, value);
        }
    }

    return result;
}

static var parseModuleDesc (const File& header)
{
    StringArray lines;
    header.readLines (lines);

    for (int i = 0; i < lines.size(); ++i)
    {
        if (trimCommentCharsFromStartOfLine (lines[i]).startsWith ("BEGIN_JUCE_MODULE_DECLARATION"))
        {
            StringArray desc;

            for (int j = i + 1; j < lines.size(); ++j)
            {
                if (trimCommentCharsFromStartOfLine (lines[j]).startsWith ("END_JUCE_MODULE_DECLARATION"))
                    return parseModuleDesc (desc);

                desc.add (lines[j]);
            }

            break;
        }
    }

    return {};
}

ModuleDescription::ModuleDescription (const File& folder)
   : moduleFolder (folder),
     moduleInfo (parseModuleDesc (getHeader()))
{
}

File ModuleDescription::getHeader() const
{
    if (moduleFolder != File())
    {
        const char* extensions[] = { ".h", ".hpp", ".hxx" };

        for (auto e : extensions)
        {
            File header (moduleFolder.getChildFile (moduleFolder.getFileName() + e));

            if (header.existsAsFile())
                return header;
        }
    }

    return {};
}

StringArray ModuleDescription::getDependencies() const
{
    auto deps = StringArray::fromTokens (moduleInfo ["dependencies"].toString(), " \t;,", "\"'");
    deps.trim();
    deps.removeEmptyStrings();
    return deps;
}

//==============================================================================
ModuleList::ModuleList()
{
}

ModuleList::ModuleList (const ModuleList& other)
{
    operator= (other);
}

ModuleList& ModuleList::operator= (const ModuleList& other)
{
    modules.clear();
    modules.addCopiesOf (other.modules);
    return *this;
}

const ModuleDescription* ModuleList::getModuleWithID (const String& moduleID) const
{
    for (auto* m : modules)
        if (m->getID() == moduleID)
            return m;

    return nullptr;
}

struct ModuleSorter
{
    static int compareElements (const ModuleDescription* m1, const ModuleDescription* m2)
    {
        return m1->getID().compareIgnoreCase (m2->getID());
    }
};

void ModuleList::sort()
{
    ModuleSorter sorter;
    modules.sort (sorter);
}

StringArray ModuleList::getIDs() const
{
    StringArray results;

    for (auto* m : modules)
        results.add (m->getID());

    results.sort (true);
    return results;
}

Result ModuleList::tryToAddModuleFromFolder (const File& path)
{
    ModuleDescription m (path);

    if (m.isValid())
    {
        modules.add (new ModuleDescription (m));
        return Result::ok();
    }

    return Result::fail (path.getFullPathName() + " is not a valid module");
}

Result ModuleList::addAllModulesInFolder (const File& path)
{
    if (! tryToAddModuleFromFolder (path))
    {
        const int subfolders = 2;
        return addAllModulesInSubfoldersRecursively (path, subfolders);
    }

    return Result::ok();
}

Result ModuleList::addAllModulesInSubfoldersRecursively (const File& path, int depth)
{
    if (depth > 0)
    {
        for (DirectoryIterator iter (path, false, "*", File::findDirectories); iter.next();)
        {
            auto childPath = iter.getFile().getLinkedTarget();

            if (! tryToAddModuleFromFolder (childPath))
                addAllModulesInSubfoldersRecursively (childPath, depth - 1);
        }
    }

    return Result::ok();
}

static Array<File> getAllPossibleModulePaths (Project& project)
{
    StringArray paths;

    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        for (int i = 0; i < project.getModules().getNumModules(); ++i)
        {
            const String path (exporter->getPathForModuleString (project.getModules().getModuleID (i)));

            if (path.isNotEmpty())
                paths.addIfNotAlreadyThere (path);
        }

        String oldPath (exporter->getLegacyModulePath());

        if (oldPath.isNotEmpty())
            paths.addIfNotAlreadyThere (oldPath);
    }

    Array<File> files;

    for (auto& path : paths)
    {
        auto f = project.resolveFilename (path);

        if (f.isDirectory())
        {
            files.add (f);

            if (f.getChildFile ("modules").isDirectory())
                files.addIfNotAlreadyThere (f.getChildFile ("modules"));
        }
    }

    return files;
}

Result ModuleList::scanAllKnownFolders (Project& project)
{
    modules.clear();
    Result result (Result::ok());

    for (auto& m : getAllPossibleModulePaths (project))
    {
        result = addAllModulesInFolder (m);

        if (result.failed())
            break;
    }

    sort();
    return result;
}

//==============================================================================
LibraryModule::LibraryModule (const ModuleDescription& d)
    : moduleInfo (d)
{
}

//==============================================================================
void LibraryModule::writeIncludes (ProjectSaver& projectSaver, OutputStream& out)
{
    Project& project = projectSaver.project;
    EnabledModuleList& modules = project.getModules();

    const String id (getID());

    if (modules.shouldCopyModuleFilesLocally (id).getValue())
    {
        const File juceModuleFolder (moduleInfo.getFolder());

        const File localModuleFolder (project.getLocalModuleFolder (id));
        localModuleFolder.createDirectory();
        projectSaver.copyFolder (juceModuleFolder, localModuleFolder);
    }

    out << "#include <" << moduleInfo.moduleFolder.getFileName() << "/"
        << moduleInfo.getHeader().getFileName()
        << ">" << newLine;
}

//==============================================================================
static void parseAndAddLibs (StringArray& libList, const String& libs)
{
    libList.addTokens (libs, ", ", {});
    libList.trim();
    libList.sort (false);
    libList.removeDuplicates (false);
}

void LibraryModule::addSettingsForModuleToExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
{
    auto& project = exporter.getProject();

    const auto moduleRelativePath = exporter.getModuleFolderRelativeToProject (getID());

    exporter.addToExtraSearchPaths (moduleRelativePath.getParentDirectory());

    String libDirPlatform;
    if (exporter.isLinux())
        libDirPlatform = "Linux";
    else if (exporter.isCodeBlocks() && exporter.isWindows())
        libDirPlatform = "MinGW";
    else
        libDirPlatform = exporter.getTargetFolder().getFileName();

    const auto libSubdirPath = String (moduleRelativePath.toUnixStyle() + "/libs/") + libDirPlatform;

    const auto moduleLibDir = File (project.getProjectFolder().getFullPathName() + "/" + libSubdirPath);
    if (moduleLibDir.exists())
        exporter.addToModuleLibPaths (RelativePath (libSubdirPath, moduleRelativePath.getRoot()));

    const auto extraInternalSearchPaths = moduleInfo.getExtraSearchPaths().trim();
    if (extraInternalSearchPaths.isNotEmpty())
    {
        StringArray paths;
        paths.addTokens (extraInternalSearchPaths, true);

        for (int i = 0; i < paths.size(); ++i)
            exporter.addToExtraSearchPaths (moduleRelativePath.getChildFile (paths.getReference(i)));
    }

    {
        const String extraDefs (moduleInfo.getPreprocessorDefs().trim());

        if (extraDefs.isNotEmpty())
            exporter.getExporterPreprocessorDefs() = exporter.getExporterPreprocessorDefsString() + "\n" + extraDefs;
    }

    {
        Array<File> compiled;

        const File localModuleFolder = project.getModules().shouldCopyModuleFilesLocally (getID()).getValue()
                                          ? project.getLocalModuleFolder (getID())
                                          : moduleInfo.getFolder();

        findAndAddCompiledUnits (exporter, &projectSaver, compiled);

        if (project.getModules().shouldShowAllModuleFilesInProject (getID()).getValue())
            addBrowseableCode (exporter, compiled, localModuleFolder);
    }

    if (exporter.isXcode())
    {
        auto& xcodeExporter = dynamic_cast<XCodeProjectExporter&> (exporter);

        if (project.isAUPluginHost())
            xcodeExporter.xcodeFrameworks.addTokens (xcodeExporter.isOSX() ? "AudioUnit CoreAudioKit" : "CoreAudioKit", false);

        const String frameworks (moduleInfo.moduleInfo [xcodeExporter.isOSX() ? "OSXFrameworks" : "iOSFrameworks"].toString());
        xcodeExporter.xcodeFrameworks.addTokens (frameworks, ", ", {});

        parseAndAddLibs (xcodeExporter.xcodeLibs, moduleInfo.moduleInfo [exporter.isOSX() ? "OSXLibs" : "iOSLibs"].toString());
    }
    else if (exporter.isLinux())
    {
        parseAndAddLibs (exporter.linuxLibs, moduleInfo.moduleInfo ["linuxLibs"].toString());
        parseAndAddLibs (exporter.linuxPackages, moduleInfo.moduleInfo ["linuxPackages"].toString());
    }
    else if (exporter.isWindows())
    {
        if (exporter.isCodeBlocks())
            parseAndAddLibs (exporter.mingwLibs, moduleInfo.moduleInfo ["mingwLibs"].toString());
        else
            parseAndAddLibs (exporter.windowsLibs, moduleInfo.moduleInfo ["windowsLibs"].toString());
    }
}

void LibraryModule::getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
{
    const File header (moduleInfo.getHeader());
    jassert (header.exists());

    StringArray lines;
    header.readLines (lines);

    for (int i = 0; i < lines.size(); ++i)
    {
        String line (lines[i].trim());

        if (line.startsWith ("/**") && line.containsIgnoreCase ("Config:"))
        {
            ScopedPointer<Project::ConfigFlag> config (new Project::ConfigFlag());
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

//==============================================================================
struct FileSorter
{
    static int compareElements (const File& f1, const File& f2)
    {
        return f1.getFileName().compareNatural (f2.getFileName());
    }
};

bool LibraryModule::CompileUnit::hasSuffix (const File& f, const char* suffix)
{
    auto fileWithoutSuffix = f.getFileNameWithoutExtension() + ".";

    return fileWithoutSuffix.containsIgnoreCase (suffix + String ("."))
             || fileWithoutSuffix.containsIgnoreCase (suffix + String ("_"));
}

void LibraryModule::CompileUnit::writeInclude (MemoryOutputStream&) const
{
}

bool LibraryModule::CompileUnit::isNeededForExporter (ProjectExporter& exporter) const
{
    if ((hasSuffix (file, "_OSX")        && ! exporter.isOSX())
     || (hasSuffix (file, "_iOS")        && ! exporter.isiOS())
     || (hasSuffix (file, "_Windows")    && ! exporter.isWindows())
     || (hasSuffix (file, "_Linux")      && ! exporter.isLinux())
     || (hasSuffix (file, "_Android")    && ! exporter.isAndroid()))
        return false;

    auto targetType = Project::getTargetTypeFromFilePath (file, false);

    if (targetType != ProjectType::Target::unspecified && ! exporter.shouldBuildTargetType (targetType))
        return false;

    return exporter.usesMMFiles() ? isCompiledForObjC
                                  : isCompiledForNonObjC;
}

String LibraryModule::CompileUnit::getFilenameForProxyFile() const
{
    return "include_" + file.getFileName();
}

Array<LibraryModule::CompileUnit> LibraryModule::getAllCompileUnits() const
{
    Array<File> files;
    getFolder().findChildFiles (files, File::findFiles, false);

    FileSorter sorter;
    files.sort (sorter);

    Array<LibraryModule::CompileUnit> units;

    for (auto& file : files)
    {
        if (file.getFileName().startsWithIgnoreCase (getID())
              && file.hasFileExtension (sourceFileExtensions))
        {
            CompileUnit cu;
            cu.file = file;
            units.add (cu);
        }
    }

    for (auto& cu : units)
    {
        cu.isCompiledForObjC = true;
        cu.isCompiledForNonObjC = ! cu.file.hasFileExtension ("mm;m");

        if (cu.isCompiledForNonObjC)
            if (files.contains (cu.file.withFileExtension ("mm")))
                cu.isCompiledForObjC = false;

        jassert (cu.isCompiledForObjC || cu.isCompiledForNonObjC);
    }

    return units;
}

void LibraryModule::findAndAddCompiledUnits (ProjectExporter& exporter,
                                             ProjectSaver* projectSaver,
                                             Array<File>& result) const
{
    for (auto& cu : getAllCompileUnits())
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

static void addFileWithGroups (Project::Item& group, const RelativePath& file, const String& path)
{
    const int slash = path.indexOfChar (File::separator);

    if (slash >= 0)
    {
        const String topLevelGroup (path.substring (0, slash));
        const String remainingPath (path.substring (slash + 1));

        Project::Item newGroup (group.getOrCreateSubGroup (topLevelGroup));
        addFileWithGroups (newGroup, file, remainingPath);
    }
    else
    {
        if (! group.containsChildForFile (file))
            group.addRelativeFile (file, -1, false);
    }
}

void LibraryModule::findBrowseableFiles (const File& folder, Array<File>& filesFound) const
{
    Array<File> tempList;
    FileSorter sorter;

    DirectoryIterator iter (folder, true, "*", File::findFiles);
    bool isHiddenFile;

    while (iter.next (nullptr, &isHiddenFile, nullptr, nullptr, nullptr, nullptr))
        if (! isHiddenFile && iter.getFile().hasFileExtension (browseableFileExtensions))
            tempList.addSorted (sorter, iter.getFile());

    filesFound.addArray (tempList);
}

void LibraryModule::addBrowseableCode (ProjectExporter& exporter, const Array<File>& compiled, const File& localModuleFolder) const
{
    if (sourceFiles.isEmpty())
        findBrowseableFiles (localModuleFolder, sourceFiles);

    Project::Item sourceGroup (Project::Item::createGroup (exporter.getProject(), getID(), "__mainsourcegroup" + getID(), false));

    const RelativePath moduleFromProject (exporter.getModuleFolderRelativeToProject (getID()));

    for (auto& sourceFile : sourceFiles)
    {
        auto pathWithinModule = FileHelpers::getRelativePathFrom (sourceFile, localModuleFolder);

        // (Note: in exporters like MSVC we have to avoid adding the same file twice, even if one of those instances
        // is flagged as being excluded from the build, because this overrides the other and it fails to compile)
        if (exporter.canCopeWithDuplicateFiles() || ! compiled.contains (sourceFile))
            addFileWithGroups (sourceGroup,
                               moduleFromProject.getChildFile (pathWithinModule),
                               pathWithinModule);
    }

    sourceGroup.sortAlphabetically (true, true);
    sourceGroup.addFileAtIndex (moduleInfo.getHeader(), -1, false);

    exporter.getModulesGroup().state.addChild (sourceGroup.state.createCopy(), -1, nullptr);
}


//==============================================================================
EnabledModuleList::EnabledModuleList (Project& p, const ValueTree& s)
    : project (p), state (s)
{
}

ModuleDescription EnabledModuleList::getModuleInfo (const String& moduleID)
{
    return ModuleDescription (getModuleFolder (moduleID));
}

bool EnabledModuleList::isModuleEnabled (const String& moduleID) const
{
    return state.getChildWithProperty (Ids::ID, moduleID).isValid();
}

bool EnabledModuleList::isAudioPluginModuleMissing() const
{
    return project.getProjectType().isAudioPlugin()
            && ! isModuleEnabled ("juce_audio_plugin_client");
}

Value EnabledModuleList::shouldShowAllModuleFilesInProject (const String& moduleID)
{
    return state.getChildWithProperty (Ids::ID, moduleID)
                .getPropertyAsValue (Ids::showAllCode, getUndoManager());
}

File EnabledModuleList::findLocalModuleFolder (const String& moduleID, bool useExportersForOtherOSes)
{
    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        if (useExportersForOtherOSes || exporter->mayCompileOnCurrentOS())
        {
            auto path = exporter->getPathForModuleString (moduleID);

            if (path.isNotEmpty())
            {
                auto moduleFolder = project.resolveFilename (path);

                if (moduleFolder.exists())
                {
                    if (ModuleDescription (moduleFolder).isValid())
                        return moduleFolder;

                    auto f = moduleFolder.getChildFile (moduleID);

                    if (ModuleDescription (f).isValid())
                        return f;
                }
            }
        }
    }

    return {};
}

File EnabledModuleList::getModuleFolder (const String& moduleID)
{
    File f = findLocalModuleFolder (moduleID, false);

    if (f == File())
        f = findLocalModuleFolder (moduleID, true);

    return f;
}

struct ModuleTreeSorter
{
    static int compareElements (const ValueTree& m1, const ValueTree& m2)
    {
        return m1[Ids::ID].toString().compareIgnoreCase (m2[Ids::ID]);
    }
};

void EnabledModuleList::sortAlphabetically()
{
    ModuleTreeSorter sorter;
    state.sort (sorter, getUndoManager(), false);
}

Value EnabledModuleList::shouldCopyModuleFilesLocally (const String& moduleID) const
{
    return state.getChildWithProperty (Ids::ID, moduleID)
                .getPropertyAsValue (Ids::useLocalCopy, getUndoManager());
}

void EnabledModuleList::addModule (const File& moduleFolder, bool copyLocally)
{
    ModuleDescription info (moduleFolder);

    if (info.isValid())
    {
        const String moduleID (info.getID());

        if (! isModuleEnabled (moduleID))
        {
            ValueTree module (Ids::MODULE);
            module.setProperty (Ids::ID, moduleID, nullptr);

            state.addChild (module, -1, getUndoManager());
            sortAlphabetically();

            shouldShowAllModuleFilesInProject (moduleID) = true;
            shouldCopyModuleFilesLocally (moduleID) = copyLocally;

            RelativePath path (moduleFolder.getParentDirectory(),
                               project.getProjectFolder(), RelativePath::projectFolder);

            for (Project::ExporterIterator exporter (project); exporter.next();)
                exporter->getPathForModuleValue (moduleID) = path.toUnixStyle();
        }
    }
}

void EnabledModuleList::removeModule (String moduleID) // must be pass-by-value, and not a const ref!
{
    for (int i = state.getNumChildren(); --i >= 0;)
        if (state.getChild(i) [Ids::ID] == moduleID)
            state.removeChild (i, getUndoManager());

    for (Project::ExporterIterator exporter (project); exporter.next();)
        exporter->removePathForModule (moduleID);
}

void EnabledModuleList::createRequiredModules (OwnedArray<LibraryModule>& modules)
{
    for (int i = 0; i < getNumModules(); ++i)
        modules.add (new LibraryModule (getModuleInfo (getModuleID (i))));
}

StringArray EnabledModuleList::getAllModules() const
{
    StringArray moduleIDs;

    for (int i = 0; i < getNumModules(); ++i)
        moduleIDs.add (getModuleID(i));

    return moduleIDs;
}

static void getDependencies (Project& project, const String& moduleID, StringArray& dependencies)
{
    ModuleDescription info (project.getModules().getModuleInfo (moduleID));

    for (auto uid : info.getDependencies())
    {
        if (! dependencies.contains (uid, true))
        {
            dependencies.add (uid);
            getDependencies (project, uid, dependencies);
        }
    }
}

StringArray EnabledModuleList::getExtraDependenciesNeeded (const String& moduleID) const
{
    StringArray dependencies, extraDepsNeeded;
    getDependencies (project, moduleID, dependencies);

    for (auto dep : dependencies)
        if (dep != moduleID && ! isModuleEnabled (dep))
            extraDepsNeeded.add (dep);

    return extraDepsNeeded;
}

bool EnabledModuleList::areMostModulesCopiedLocally() const
{
    int numYes = 0, numNo = 0;

    for (int i = getNumModules(); --i >= 0;)
    {
        if (shouldCopyModuleFilesLocally (getModuleID (i)).getValue())
            ++numYes;
        else
            ++numNo;
    }

    return numYes > numNo;
}

void EnabledModuleList::setLocalCopyModeForAllModules (bool copyLocally)
{
    for (int i = getNumModules(); --i >= 0;)
        shouldCopyModuleFilesLocally (project.getModules().getModuleID (i)) = copyLocally;
}

File EnabledModuleList::findDefaultModulesFolder (Project& project)
{
    ModuleList available;
    available.scanAllKnownFolders (project);

    for (int i = available.modules.size(); --i >= 0;)
    {
        File f (available.modules.getUnchecked(i)->getFolder());

        if (f.isDirectory())
            return f.getParentDirectory();
    }

    return File::getCurrentWorkingDirectory();
}

void EnabledModuleList::addModuleFromUserSelectedFile()
{
    static File lastLocation (findDefaultModulesFolder (project));

    FileChooser fc ("Select a module to add...", lastLocation, String());

    if (fc.browseForDirectory())
    {
        lastLocation = fc.getResult();
        addModuleOfferingToCopy (lastLocation);
    }
}

void EnabledModuleList::addModuleInteractive (const String& moduleID)
{
    ModuleList list;
    list.scanAllKnownFolders (project);

    if (auto* info = list.getModuleWithID (moduleID))
        addModule (info->moduleFolder, areMostModulesCopiedLocally());
    else
        addModuleFromUserSelectedFile();
}

void EnabledModuleList::addModuleOfferingToCopy (const File& f)
{
    ModuleDescription m (f);

    if (! m.isValid())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          "Add Module", "This wasn't a valid module folder!");
        return;
    }

    if (isModuleEnabled (m.getID()))
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          "Add Module", "The project already contains this module!");
        return;
    }

    addModule (m.moduleFolder, areMostModulesCopiedLocally());
}

bool isJuceFolder (const File& f)
{
    return isJuceModulesFolder (f.getChildFile ("modules"));
}

bool isJuceModulesFolder (const File& f)
{
    return f.isDirectory() && f.getChildFile ("juce_core").isDirectory();
}
