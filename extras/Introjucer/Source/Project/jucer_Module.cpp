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

#include "jucer_Module.h"
#include "jucer_ProjectType.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Project Saving/jucer_ProjectSaver.h"
#include "jucer_AudioPluginModule.h"


ModuleDescription::ModuleDescription (const File& manifest)
   : moduleInfo (JSON::parse (manifest)), manifestFile (manifest)
{
    if (moduleInfo.isVoid() && manifestFile.exists())
    {
        var json;
        Result r (JSON::parse (manifestFile.loadFileAsString(), json));

        if (r.failed() && manifestFile.loadFileAsString().isNotEmpty())
        {
            DBG (r.getErrorMessage());
            jassertfalse; // broken JSON in a module manifest.
        }
    }
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
    for (int i = 0; i < modules.size(); ++i)
    {
        ModuleDescription* m = modules.getUnchecked(i);
        if (m->getID() == moduleID)
            return m;
    }

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

    for (int i = 0; i < modules.size(); ++i)
        results.add (modules.getUnchecked(i)->getID());

    results.sort (true);
    return results;
}

Result ModuleList::addAllModulesInFolder (const File& path)
{
    const File moduleDef (path.getChildFile (ModuleDescription::getManifestFileName()));

    if (moduleDef.exists())
    {
        ModuleDescription m (moduleDef);

        if (! m.isValid())
            return Result::fail ("Failed to load module manifest: " + moduleDef.getFullPathName());

        modules.add (new ModuleDescription (m));
    }
    else
    {
        for (DirectoryIterator iter (path, false, "*", File::findDirectories); iter.next();)
        {
            Result r = addAllModulesInFolder (iter.getFile().getLinkedTarget());
            if (r.failed())
                return r;
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

    for (int i = 0; i < paths.size(); ++i)
    {
        const File f (project.resolveFilename (paths[i]));

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

    const Array<File> modulePaths (getAllPossibleModulePaths (project));

    for (int i = 0; i < modulePaths.size(); ++i)
    {
        result = addAllModulesInFolder (modulePaths.getReference(i));

        if (result.failed())
            break;
    }

    sort();
    return result;
}

bool ModuleList::loadFromWebsite()
{
    modules.clear();

    URL baseURL ("http://www.juce.com/juce/modules");
    URL url (baseURL.getChildURL ("modulelist.php"));

    const ScopedPointer<InputStream> in (url.createInputStream (false, nullptr, nullptr, String::empty, 4000));

    if (in == nullptr)
        return false;

    var infoList (JSON::parse (in->readEntireStreamAsString()));

    if (! infoList.isArray())
        return false;

    const Array<var>* moduleList = infoList.getArray();

    for (int i = 0; i < moduleList->size(); ++i)
    {
        const var& m = moduleList->getReference(i);
        const String file (m [Ids::file].toString());

        if (file.isNotEmpty())
        {
            ModuleDescription lm (m [Ids::info]);

            if (lm.isValid())
            {
                lm.url = baseURL.getChildURL (file);
                modules.add (new ModuleDescription (lm));
            }
        }
    }

    sort();
    return true;
}

//==============================================================================
LibraryModule::LibraryModule (const ModuleDescription& d)
    : moduleInfo (d)
{
}

bool LibraryModule::isAUPluginHost (const Project& project) const   { return getID() == "juce_audio_processors" && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_AU"); }
bool LibraryModule::isVSTPluginHost (const Project& project) const  { return getID() == "juce_audio_processors" && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_VST"); }
bool LibraryModule::isVST3PluginHost (const Project& project) const { return getID() == "juce_audio_processors" && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_VST3"); }

File LibraryModule::getModuleHeaderFile (const File& folder) const
{
    return folder.getChildFile (moduleInfo.getHeaderName());
}

//==============================================================================
void LibraryModule::writeIncludes (ProjectSaver& projectSaver, OutputStream& out)
{
    const File localModuleFolder (projectSaver.getLocalModuleFolder (getID()));
    const File localHeader (getModuleHeaderFile (localModuleFolder));

    localModuleFolder.createDirectory();

    if (projectSaver.project.getModules().shouldCopyModuleFilesLocally (getID()).getValue())
    {
        projectSaver.copyFolder (moduleInfo.getFolder(), localModuleFolder);
    }
    else
    {
        localModuleFolder.createDirectory();
        createLocalHeaderWrapper (projectSaver, getModuleHeaderFile (moduleInfo.getFolder()), localHeader);
    }

    out << CodeHelpers::createIncludeStatement (localHeader, projectSaver.getGeneratedCodeFolder()
                                                                         .getChildFile ("AppConfig.h")) << newLine;
}

static void writeGuardedInclude (OutputStream& out, StringArray paths, StringArray guards)
{
    StringArray uniquePaths (paths);
    uniquePaths.removeDuplicates (false);

    if (uniquePaths.size() == 1)
    {
        out << "#include " << paths[0] << newLine;
    }
    else
    {
        for (int i = paths.size(); --i >= 0;)
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

        for (int i = 0; i < paths.size(); ++i)
        {
            out << (i == 0 ? "#if " : "#elif ") << guards[i] << newLine
                << " #include " << paths[i] << newLine;
        }

        out << "#else" << newLine
            << " #error \"This file is designed to be used in an Introjucer-generated project!\"" << newLine
            << "#endif" << newLine;
    }
}

void LibraryModule::createLocalHeaderWrapper (ProjectSaver& projectSaver, const File& originalHeader, const File& localHeader) const
{
    Project& project = projectSaver.project;

    MemoryOutputStream out;

    out << "// This is an auto-generated file to redirect any included" << newLine
        << "// module headers to the correct external folder." << newLine
        << newLine;

    StringArray paths, guards;

    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        const RelativePath headerFromProject (exporter->getModuleFolderRelativeToProject (getID(), projectSaver)
                                                .getChildFile (originalHeader.getFileName()));

        const RelativePath fileFromHere (headerFromProject.rebased (project.getProjectFolder(),
                                                                    localHeader.getParentDirectory(), RelativePath::unknown));

        paths.add (fileFromHere.toUnixStyle().quoted());
        guards.add ("defined (" + exporter->getExporterIdentifierMacro() + ")");
    }

    writeGuardedInclude (out, paths, guards);
    out << newLine;

    projectSaver.replaceFileIfDifferent (localHeader, out);
}

//==============================================================================
void LibraryModule::prepareExporter (ProjectExporter& exporter, ProjectSaver& projectSaver) const
{
    Project& project = exporter.getProject();

    exporter.addToExtraSearchPaths (exporter.getModuleFolderRelativeToProject (getID(), projectSaver).getParentDirectory());

    const String extraDefs (moduleInfo.getPreprocessorDefs().trim());

    if (extraDefs.isNotEmpty())
        exporter.getExporterPreprocessorDefs() = exporter.getExporterPreprocessorDefsString() + "\n" + extraDefs;

    {
        Array<File> compiled;

        const File localModuleFolder = project.getModules().shouldCopyModuleFilesLocally (getID()).getValue()
                                          ? projectSaver.getLocalModuleFolder (getID())
                                          : moduleInfo.getFolder();

        findAndAddCompiledCode (exporter, projectSaver, localModuleFolder, compiled);

        if (project.getModules().shouldShowAllModuleFilesInProject (getID()).getValue())
            addBrowsableCode (exporter, projectSaver, compiled, localModuleFolder);
    }

    if (isVSTPluginHost (project))  VSTHelpers::addVSTFolderToPath (exporter, false);
    if (isVST3PluginHost (project)) VSTHelpers::addVSTFolderToPath (exporter, true);

    if (exporter.isXcode())
    {
        if (isAUPluginHost (project))
            exporter.xcodeFrameworks.addTokens ("AudioUnit CoreAudioKit", false);

        const String frameworks (moduleInfo.moduleInfo [exporter.isOSX() ? "OSXFrameworks" : "iOSFrameworks"].toString());
        exporter.xcodeFrameworks.addTokens (frameworks, ", ", StringRef());
    }
    else if (exporter.isLinux())
    {
        const String libs (moduleInfo.moduleInfo ["LinuxLibs"].toString());
        exporter.linuxLibs.addTokens (libs, ", ", StringRef());
        exporter.linuxLibs.trim();
        exporter.linuxLibs.sort (false);
        exporter.linuxLibs.removeDuplicates (false);
    }
    else if (exporter.isCodeBlocks())
    {
        const String libs (moduleInfo.moduleInfo ["mingwLibs"].toString());
        exporter.mingwLibs.addTokens (libs, ", ", StringRef());
        exporter.mingwLibs.trim();
        exporter.mingwLibs.sort (false);
        exporter.mingwLibs.removeDuplicates (false);
    }

    if (moduleInfo.isPluginClient())
    {
        if (shouldBuildVST  (project).getValue())  VSTHelpers::prepareExporter (exporter, projectSaver, false);
        if (shouldBuildVST3 (project).getValue())  VSTHelpers::prepareExporter (exporter, projectSaver, true);
        if (shouldBuildAU   (project).getValue())  AUHelpers::prepareExporter (exporter, projectSaver);
        if (shouldBuildAAX  (project).getValue())  AAXHelpers::prepareExporter (exporter, projectSaver);
        if (shouldBuildRTAS (project).getValue())  RTASHelpers::prepareExporter (exporter, projectSaver);
    }
}

void LibraryModule::createPropertyEditors (ProjectExporter& exporter, PropertyListBuilder& props) const
{
    if (isVSTPluginHost (exporter.getProject())
         && ! (moduleInfo.isPluginClient() && shouldBuildVST  (exporter.getProject()).getValue()))
        VSTHelpers::createVSTPathEditor (exporter, props, false);

    if (isVST3PluginHost (exporter.getProject())
         && ! (moduleInfo.isPluginClient() && shouldBuildVST3  (exporter.getProject()).getValue()))
        VSTHelpers::createVSTPathEditor (exporter, props, true);

    if (moduleInfo.isPluginClient())
    {
        if (shouldBuildVST  (exporter.getProject()).getValue())  VSTHelpers::createPropertyEditors (exporter, props, false);
        if (shouldBuildVST3 (exporter.getProject()).getValue())  VSTHelpers::createPropertyEditors (exporter, props, true);
        if (shouldBuildRTAS (exporter.getProject()).getValue())  RTASHelpers::createPropertyEditors (exporter, props);
        if (shouldBuildAAX  (exporter.getProject()).getValue())  AAXHelpers::createPropertyEditors (exporter, props);
    }
}

void LibraryModule::getConfigFlags (Project& project, OwnedArray<Project::ConfigFlag>& flags) const
{
    const File header (getModuleHeaderFile (moduleInfo.getFolder()));
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

//==============================================================================
static bool exporterTargetMatches (const String& test, String target)
{
    StringArray validTargets;
    validTargets.addTokens (target, ",;", "");
    validTargets.trim();
    validTargets.removeEmptyStrings();

    if (validTargets.size() == 0)
        return true;

    for (int i = validTargets.size(); --i >= 0;)
    {
        const String& targetName = validTargets[i];

        if (targetName == test
             || (targetName.startsWithChar ('!') && test != targetName.substring (1).trimStart()))
            return true;
    }

    return false;
}

struct FileSorter
{
    static int compareElements (const File& f1, const File& f2)
    {
        return f1.getFileName().compareNatural (f2.getFileName());
    }
};

void LibraryModule::findWildcardMatches (const File& localModuleFolder, const String& wildcardPath, Array<File>& result) const
{
    String path (wildcardPath.upToLastOccurrenceOf ("/", false, false));
    String wildCard (wildcardPath.fromLastOccurrenceOf ("/", false, false));

    Array<File> tempList;
    FileSorter sorter;

    DirectoryIterator iter (localModuleFolder.getChildFile (path), false, wildCard);
    bool isHiddenFile;

    while (iter.next (nullptr, &isHiddenFile, nullptr, nullptr, nullptr, nullptr))
        if (! isHiddenFile)
            tempList.addSorted (sorter, iter.getFile());

    result.addArray (tempList);
}

static bool fileTargetMatches (ProjectExporter& exporter, const String& target)
{
    if (exporter.isXcode())         return exporterTargetMatches ("xcode", target);
    if (exporter.isWindows())       return exporterTargetMatches ("msvc", target);
    if (exporter.isLinux())         return exporterTargetMatches ("linux", target);
    if (exporter.isAndroid())       return exporterTargetMatches ("android", target);
    if (exporter.isCodeBlocks())    return exporterTargetMatches ("mingw", target);
    return target.isEmpty();
}

static bool fileShouldBeAdded (ProjectExporter& exporter, const var& properties)
{
    if (! fileTargetMatches (exporter, properties["target"].toString()))
        return false;

    if (properties["RTASOnly"] && ! shouldBuildRTAS (exporter.getProject()).getValue())
        return false;

    if (properties["AudioUnitOnly"] && ! shouldBuildAU (exporter.getProject()).getValue())
        return false;

    return true;
}

void LibraryModule::findAndAddCompiledCode (ProjectExporter& exporter, ProjectSaver& projectSaver,
                                            const File& localModuleFolder, Array<File>& result) const
{
    const var compileArray (moduleInfo.moduleInfo ["compile"]); // careful to keep this alive while the array is in use!

    if (const Array<var>* const files = compileArray.getArray())
    {
        for (int i = 0; i < files->size(); ++i)
        {
            const var& file = files->getReference(i);
            const String filename (file ["file"].toString());

            if (filename.isNotEmpty() && fileShouldBeAdded (exporter, file))
            {
                const File compiledFile (localModuleFolder.getChildFile (filename));
                result.add (compiledFile);

                Project::Item item (projectSaver.addFileToGeneratedGroup (compiledFile));

                if (file ["warnings"].toString().equalsIgnoreCase ("disabled"))
                    item.getShouldInhibitWarningsValue() = true;

                if (file ["stdcall"])
                    item.getShouldUseStdCallValue() = true;
            }
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

void LibraryModule::findBrowseableFiles (const File& localModuleFolder, Array<File>& filesFound) const
{
    const var filesArray (moduleInfo.moduleInfo ["browse"]);

    if (const Array<var>* const files = filesArray.getArray())
        for (int i = 0; i < files->size(); ++i)
            findWildcardMatches (localModuleFolder, files->getReference(i), filesFound);
}

void LibraryModule::addBrowsableCode (ProjectExporter& exporter, ProjectSaver& projectSaver,
                                      const Array<File>& compiled, const File& localModuleFolder) const
{
    if (sourceFiles.size() == 0)
        findBrowseableFiles (localModuleFolder, sourceFiles);

    Project::Item sourceGroup (Project::Item::createGroup (exporter.getProject(), getID(), "__mainsourcegroup" + getID()));

    const RelativePath moduleFromProject (exporter.getModuleFolderRelativeToProject (getID(), projectSaver));

    for (int i = 0; i < sourceFiles.size(); ++i)
    {
        const String pathWithinModule (FileHelpers::getRelativePathFrom (sourceFiles.getReference(i), localModuleFolder));

        // (Note: in exporters like MSVC we have to avoid adding the same file twice, even if one of those instances
        // is flagged as being excluded from the build, because this overrides the other and it fails to compile)
        if (exporter.canCopeWithDuplicateFiles() || ! compiled.contains (sourceFiles.getReference(i)))
            addFileWithGroups (sourceGroup,
                               moduleFromProject.getChildFile (pathWithinModule),
                               pathWithinModule);
    }

    sourceGroup.addFile (localModuleFolder.getChildFile (FileHelpers::getRelativePathFrom (moduleInfo.manifestFile,
                                                                                           moduleInfo.getFolder())), -1, false);
    sourceGroup.addFile (getModuleHeaderFile (localModuleFolder), -1, false);

    exporter.getModulesGroup().state.addChild (sourceGroup.state.createCopy(), -1, nullptr);
}


//==============================================================================
EnabledModuleList::EnabledModuleList (Project& p, const ValueTree& s)
    : project (p), state (s)
{
}

ModuleDescription EnabledModuleList::getModuleInfo (const String& moduleID)
{
    return ModuleDescription (getModuleInfoFile (moduleID));
}

bool EnabledModuleList::isModuleEnabled (const String& moduleID) const
{
    for (int i = 0; i < state.getNumChildren(); ++i)
        if (state.getChild(i) [Ids::ID] == moduleID)
            return true;

    return false;
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

File EnabledModuleList::findLocalModuleInfoFile (const String& moduleID, bool useExportersForOtherOSes)
{
    for (Project::ExporterIterator exporter (project); exporter.next();)
    {
        if (useExportersForOtherOSes || exporter->mayCompileOnCurrentOS())
        {
            const String path (exporter->getPathForModuleString (moduleID));

            if (path.isNotEmpty())
            {
                const File moduleFolder (project.resolveFilename (path));

                if (moduleFolder.exists())
                {
                    File f (moduleFolder.getChildFile (ModuleDescription::getManifestFileName()));

                    if (f.exists())
                        return f;

                    f = moduleFolder.getChildFile (moduleID)
                                    .getChildFile (ModuleDescription::getManifestFileName());

                    if (f.exists())
                        return f;

                    f = moduleFolder.getChildFile ("modules")
                                    .getChildFile (moduleID)
                                    .getChildFile (ModuleDescription::getManifestFileName());

                    if (f.exists())
                        return f;
                }
            }
        }
    }

    return File::nonexistent;
}

File EnabledModuleList::getModuleInfoFile (const String& moduleID)
{
    const File f (findLocalModuleInfoFile (moduleID, false));

    if (f != File::nonexistent)
        return f;

    return findLocalModuleInfoFile (moduleID, true);
}

File EnabledModuleList::getModuleFolder (const String& moduleID)
{
    const File infoFile (getModuleInfoFile (moduleID));

    return infoFile.exists() ? infoFile.getParentDirectory()
                             : File::nonexistent;
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

void EnabledModuleList::addModule (const File& moduleManifestFile, bool copyLocally)
{
    ModuleDescription info (moduleManifestFile);

    if (info.isValid())
    {
        const String moduleID (info.getID());

        if (! isModuleEnabled (moduleID))
        {
            ValueTree module (Ids::MODULES);
            module.setProperty (Ids::ID, moduleID, nullptr);

            state.addChild (module, -1, getUndoManager());
            sortAlphabetically();

            shouldShowAllModuleFilesInProject (moduleID) = true;
            shouldCopyModuleFilesLocally (moduleID) = copyLocally;

            RelativePath path (moduleManifestFile.getParentDirectory().getParentDirectory(),
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
    {
        ModuleDescription info (getModuleInfo (getModuleID (i)));

        if (info.isValid())
            modules.add (new LibraryModule (info));
    }
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

    if (info.isValid())
    {
        const var depsArray (info.moduleInfo ["dependencies"]);

        if (const Array<var>* const deps = depsArray.getArray())
        {
            for (int i = 0; i < deps->size(); ++i)
            {
                const var& d = deps->getReference(i);

                String uid (d [Ids::ID].toString());
                String version (d [Ids::version].toString());

                if (! dependencies.contains (uid, true))
                {
                    dependencies.add (uid);
                    getDependencies (project, uid, dependencies);
                }
            }
        }
    }
}

StringArray EnabledModuleList::getExtraDependenciesNeeded (const String& moduleID) const
{
    StringArray dependencies, extraDepsNeeded;
    getDependencies (project, moduleID, dependencies);

    for (int i = 0; i < dependencies.size(); ++i)
        if ((! isModuleEnabled (dependencies[i])) && dependencies[i] != moduleID)
            extraDepsNeeded.add (dependencies[i]);

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
    available.scanAllKnownFolders  (project);

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

    FileChooser fc ("Select a module to add...", lastLocation, String::empty, false);

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

    if (const ModuleDescription* info = list.getModuleWithID (moduleID))
        addModule (info->manifestFile, areMostModulesCopiedLocally());
    else
        addModuleFromUserSelectedFile();
}

void EnabledModuleList::addModuleOfferingToCopy (const File& f)
{
    ModuleDescription m (f);

    if (! m.isValid())
        m = ModuleDescription (f.getChildFile (ModuleDescription::getManifestFileName()));

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

    addModule (m.manifestFile, areMostModulesCopiedLocally());
}

bool isJuceFolder (const File& f)
{
    return isJuceModulesFolder (f.getChildFile ("modules"));
}

bool isJuceModulesFolder (const File& f)
{
    return f.isDirectory() && f.getChildFile ("juce_core").isDirectory();
}
