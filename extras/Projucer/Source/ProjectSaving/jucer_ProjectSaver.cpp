/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "jucer_ProjectSaver.h"
#include "../Application/jucer_Application.h"

static constexpr const char* generatedGroupID = "__jucelibfiles";
static constexpr const char* generatedGroupUID = "__generatedcode__";

constexpr int jucerFormatVersion = 1;

//==============================================================================
ProjectSaver::ProjectSaver (Project& p)
    : project (p),
      generatedCodeFolder (project.getGeneratedCodeFolder()),
      generatedFilesGroup (Project::Item::createGroup (project, getJuceCodeGroupName(), generatedGroupUID, true)),
      projectLineFeed (project.getProjectLineFeed())
{
    generatedFilesGroup.setID (generatedGroupID);
}

void ProjectSaver::save (Async async, ProjectExporter* exporterToSave, std::function<void (Result)> onCompletion)
{
    if (async == Async::yes)
        saveProjectAsync (exporterToSave, std::move (onCompletion));
    else
        onCompletion (saveProject (exporterToSave));
}

void ProjectSaver::saveProjectAsync (ProjectExporter* exporterToSave, std::function<void (Result)> onCompletion)
{
    jassert (saveThread == nullptr);

    saveThread = std::make_unique<SaveThreadWithProgressWindow> (*this, exporterToSave,
                                                                 [ref = WeakReference<ProjectSaver> { this }, onCompletion] (Result result)
    {
        if (ref == nullptr)
            return;

        // Clean up old save thread in case onCompletion wants to start a new save thread
        ref->saveThread->waitForThreadToExit (-1);
        ref->saveThread = nullptr;

        NullCheckedInvocation::invoke (onCompletion, result);
    });
    saveThread->launchThread();
}

Result ProjectSaver::saveResourcesOnly()
{
    writeBinaryDataFiles();

    if (! errors.isEmpty())
        return Result::fail (errors[0]);

    return Result::ok();
}

void ProjectSaver::saveBasicProjectItems (const OwnedArray<LibraryModule>& modules, const String& appConfigUserContent)
{
    writeLV2DefinesFile();
    writePluginDefines();
    writeAppConfigFile (modules, appConfigUserContent);
    writeBinaryDataFiles();
    writeAppHeader (modules);
    writeModuleCppWrappers (modules);
}

Project::Item ProjectSaver::addFileToGeneratedGroup (const File& file)
{
    auto item = generatedFilesGroup.findItemForFile (file);

    if (item.isValid())
        return item;

    generatedFilesGroup.addFileAtIndex (file, -1, true);
    return generatedFilesGroup.findItemForFile (file);
}

bool ProjectSaver::copyFolder (const File& source, const File& dest)
{
    if (source.isDirectory() && dest.createDirectory())
    {
        for (auto& f : source.findChildFiles (File::findFiles, false))
        {
            auto target = dest.getChildFile (f.getFileName());
            filesCreated.add (target);

            if (! f.copyFileTo (target))
                return false;
        }

        for (auto& f : source.findChildFiles (File::findDirectories, false))
        {
            auto name = f.getFileName();

            if (name == ".git" || name == ".svn" || name == ".cvs")
                continue;

            if (! copyFolder (f, dest.getChildFile (f.getFileName())))
                return false;
        }

        return true;
    }

    return false;
}

//==============================================================================
Project::Item ProjectSaver::saveGeneratedFile (const String& filePath, const MemoryOutputStream& newData)
{
    if (! generatedCodeFolder.createDirectory())
    {
        addError ("Couldn't create folder: " + generatedCodeFolder.getFullPathName());
        return Project::Item (project, {}, false);
    }

    auto file = generatedCodeFolder.getChildFile (filePath);

    if (replaceFileIfDifferent (file, newData))
        return addFileToGeneratedGroup (file);

    return { project, {}, true };
}

bool ProjectSaver::replaceFileIfDifferent (const File& f, const MemoryOutputStream& newData)
{
    filesCreated.add (f);

    if (! build_tools::overwriteFileWithNewDataIfDifferent (f, newData))
    {
        addError ("Can't write to file: " + f.getFullPathName());
        return false;
    }

    return true;
}

bool ProjectSaver::deleteUnwantedFilesIn (const File& parent)
{
    // Recursively clears out any files in a folder that we didn't create, but avoids
    // any folders containing hidden files that might be used by version-control systems.
    auto shouldFileBeKept = [] (const String& filename)
    {
        static StringArray filesToKeep (".svn", ".cvs", "CMakeLists.txt");
        return filesToKeep.contains (filename);
    };

    bool folderIsNowEmpty = true;
    Array<File> filesToDelete;

    for (const auto& i : RangedDirectoryIterator (parent, false, "*", File::findFilesAndDirectories))
    {
        auto f = i.getFile();

        if (filesCreated.contains (f) || shouldFileBeKept (f.getFileName()))
        {
            folderIsNowEmpty = false;
        }
        else if (i.isDirectory())
        {
            if (deleteUnwantedFilesIn (f))
                filesToDelete.add (f);
            else
                folderIsNowEmpty = false;
        }
        else
        {
            filesToDelete.add (f);
        }
    }

    for (int j = filesToDelete.size(); --j >= 0;)
        filesToDelete.getReference (j).deleteRecursively();

    return folderIsNowEmpty;
}

//==============================================================================
void ProjectSaver::addError (const String& message)
{
    const ScopedLock sl (errorLock);
    errors.add (message);
}

//==============================================================================
File ProjectSaver::getAppConfigFile() const
{
    return generatedCodeFolder.getChildFile (Project::getAppConfigFilename());
}

File ProjectSaver::getPluginDefinesFile() const
{
    return generatedCodeFolder.getChildFile (Project::getPluginDefinesFilename());
}

String ProjectSaver::loadUserContentFromAppConfig() const
{
    StringArray userContent;
    bool foundCodeSection = false;
    auto lines = StringArray::fromLines (getAppConfigFile().loadFileAsString());

    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].contains ("[BEGIN_USER_CODE_SECTION]"))
        {
            for (int j = i + 1; j < lines.size() && ! lines[j].contains ("[END_USER_CODE_SECTION]"); ++j)
                userContent.add (lines[j]);

            foundCodeSection = true;
            break;
        }
    }

    if (! foundCodeSection)
    {
        userContent.add ({});
        userContent.add ("// (You can add your own code in this section, and the Projucer will not overwrite it)");
        userContent.add ({});
    }

    return userContent.joinIntoString (projectLineFeed) + projectLineFeed;
}

//==============================================================================
OwnedArray<LibraryModule> ProjectSaver::getModules()
{
    OwnedArray<LibraryModule> modules;
    project.getEnabledModules().createRequiredModules (modules);

    auto isCommandLine = ProjucerApplication::getApp().isRunningCommandLine;

    for (auto* module : modules)
    {
        if (! module->isValid())
        {
            addError (String ("At least one of your JUCE module paths is invalid!\n")
                + (isCommandLine ? "Please ensure each module path points to the correct JUCE modules folder."
                                 : "Please go to the Modules settings page and ensure each path points to the correct JUCE modules folder."));

            return {};
        }

        if (project.getEnabledModules().getExtraDependenciesNeeded (module->getID()).size() > 0)
        {
            addError (String ("At least one of your modules has missing dependencies!\n")
                + (isCommandLine ? "Please add the required dependencies, or run the command again with the \"--fix-missing-dependencies\" option."
                                 : "Please go to the settings page of the highlighted modules and add the required dependencies."));

            return {};
        }
    }

    return modules;
}

//==============================================================================
Result ProjectSaver::saveProject (ProjectExporter* specifiedExporterToSave)
{
    if (project.getNumExporters() == 0)
    {
        return Result::fail ("No exporters found!\n"
                             "Please add an exporter before saving.");
    }

    auto oldProjectFile = project.getFile();
    auto modules = getModules();

    if (errors.isEmpty())
    {
        if (project.isAudioPluginProject())
        {
            if (project.shouldBuildUnityPlugin())
                writeUnityScriptFile();
        }

        saveBasicProjectItems (modules, loadUserContentFromAppConfig());
        writeProjects (modules, specifiedExporterToSave);
        writeProjectFile();

        runPostExportScript();

        if (generatedCodeFolder.exists())
        {
            writeReadmeFile();
            deleteUnwantedFilesIn (generatedCodeFolder);
        }

        if (errors.isEmpty())
            return Result::ok();
    }

    project.setFile (oldProjectFile);
    return Result::fail (errors[0]);
}

//==============================================================================
void ProjectSaver::writePluginDefines (MemoryOutputStream& out) const
{
    const auto pluginDefines = getAudioPluginDefines();

    if (pluginDefines.isEmpty())
        return;

    writeAutoGenWarningComment (out);

    out << "*/" << newLine << newLine
        << "#pragma once" << newLine << newLine
        << pluginDefines << newLine;
}

void ProjectSaver::writeProjectFile()
{
    auto root = project.getProjectRoot();

    root.removeProperty ("jucerVersion", nullptr);

    if ((int) root.getProperty (Ids::jucerFormatVersion, -1) != jucerFormatVersion)
        root.setProperty (Ids::jucerFormatVersion, jucerFormatVersion, nullptr);

    project.updateCachedFileState();

    auto newSerialisedXml = project.serialiseProjectXml (root.createXml());
    jassert (newSerialisedXml.isNotEmpty());

    if (newSerialisedXml != project.getCachedFileStateContent())
    {
        project.getFile().replaceWithText (newSerialisedXml);
        project.updateCachedFileState();
    }
}

void ProjectSaver::writeAppConfig (MemoryOutputStream& out, const OwnedArray<LibraryModule>& modules, const String& userContent)
{
    if (! project.shouldUseAppConfig())
        return;

    writeAutoGenWarningComment (out);

    out << "    There's a section below where you can add your own custom code safely, and the" << newLine
        << "    Projucer will preserve the contents of that block, but the best way to change" << newLine
        << "    any of these definitions is by using the Projucer's project settings." << newLine
        << newLine
        << "    Any commented-out settings will assume their default values." << newLine
        << newLine
        << "*/" << newLine
        << newLine;

    out << "#pragma once" << newLine
        << newLine
        << "//==============================================================================" << newLine
        << "// [BEGIN_USER_CODE_SECTION]" << newLine
        << userContent
        << "// [END_USER_CODE_SECTION]" << newLine;

    if (getPluginDefinesFile().existsAsFile() && getAudioPluginDefines().isNotEmpty())
        out << newLine << CodeHelpers::createIncludeStatement (Project::getPluginDefinesFilename()) << newLine;

    out << newLine
        << "/*" << newLine
        << "  ==============================================================================" << newLine
        << newLine
        << "   In accordance with the terms of the JUCE 7 End-Use License Agreement, the" << newLine
        << "   JUCE Code in SECTION A cannot be removed, changed or otherwise rendered" << newLine
        << "   ineffective unless you have a JUCE Indie or Pro license, or are using JUCE" << newLine
        << "   under the GPL v3 license." << newLine
        << newLine
        << "   End User License Agreement: www.juce.com/juce-7-licence" << newLine
        << newLine
        << "  ==============================================================================" << newLine
        << "*/" << newLine
        << newLine
        << "// BEGIN SECTION A" << newLine
        << newLine
        << "#ifndef JUCE_DISPLAY_SPLASH_SCREEN" << newLine
        << " #define JUCE_DISPLAY_SPLASH_SCREEN "   << (project.shouldDisplaySplashScreen() ? "1" : "0") << newLine
        << "#endif" << newLine << newLine
        << "// END SECTION A" << newLine
        << newLine
        << "#define JUCE_USE_DARK_SPLASH_SCREEN "  << (project.getSplashScreenColourString() == "Dark" ? "1" : "0") << newLine
        << newLine
        << "#define JUCE_PROJUCER_VERSION 0x" << String::toHexString (ProjectInfo::versionNumber) << newLine;

    out << newLine
        << "//==============================================================================" << newLine;

    auto longestModuleName = [&modules]()
    {
        int longest = 0;

        for (auto* module : modules)
            longest = jmax (longest, module->getID().length());

        return longest;
    }();

    for (auto* module : modules)
    {
        out << "#define JUCE_MODULE_AVAILABLE_" << module->getID()
            << String::repeatedString (" ", longestModuleName + 5 - module->getID().length()) << " 1" << newLine;
    }

    out << newLine << "#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1" << newLine;

    for (auto* module : modules)
    {
        OwnedArray<Project::ConfigFlag> flags;
        module->getConfigFlags (project, flags);

        if (flags.size() > 0)
        {
            out << newLine
                << "//==============================================================================" << newLine
                << "// " << module->getID() << " flags:" << newLine;

            for (auto* flag : flags)
            {
                out << newLine
                << "#ifndef    " << flag->symbol
                << newLine
                << (flag->value.isUsingDefault() ? " //#define " : " #define   ") << flag->symbol << " " << (flag->value.get() ? "1" : "0")
                << newLine
                << "#endif"
                << newLine;
            }
        }
    }

    auto& type = project.getProjectType();
    auto isStandaloneApplication = (! type.isAudioPlugin() && ! type.isDynamicLibrary());

    out << newLine
        << "//==============================================================================" << newLine
        << "#ifndef    JUCE_STANDALONE_APPLICATION" << newLine
        << " #if defined(JucePlugin_Name) && defined(JucePlugin_Build_Standalone)" << newLine
        << "  #define  JUCE_STANDALONE_APPLICATION JucePlugin_Build_Standalone" << newLine
        << " #else" << newLine
        << "  #define  JUCE_STANDALONE_APPLICATION " << (isStandaloneApplication ? "1" : "0") << newLine
        << " #endif" << newLine
        << "#endif" << newLine;
}

template <typename WriterCallback>
void ProjectSaver::writeOrRemoveGeneratedFile (const String& name, WriterCallback&& writerCallback)
{
    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

    writerCallback (mem);

    if (mem.getDataSize() != 0)
    {
        saveGeneratedFile (name, mem);
        return;
    }

    const auto destFile = generatedCodeFolder.getChildFile (name);

    if (destFile.existsAsFile())
    {
        if (! destFile.deleteFile())
            addError ("Couldn't remove unnecessary file: " + destFile.getFullPathName());
    }
}

void ProjectSaver::writePluginDefines()
{
    writeOrRemoveGeneratedFile (Project::getPluginDefinesFilename(), [&] (MemoryOutputStream& mem)
    {
        writePluginDefines (mem);
    });
}

void ProjectSaver::writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const String& userContent)
{
    writeOrRemoveGeneratedFile (Project::getAppConfigFilename(), [&] (MemoryOutputStream& mem)
    {
        writeAppConfig (mem, modules, userContent);
    });
}

void ProjectSaver::writeLV2DefinesFile()
{
    if (! project.shouldBuildLV2())
        return;

    writeOrRemoveGeneratedFile (Project::getJuceLV2DefinesFilename(), [&] (MemoryOutputStream& mem)
    {
        writeLV2Defines (mem);
    });
}

void ProjectSaver::writeAppHeader (MemoryOutputStream& out, const OwnedArray<LibraryModule>& modules)
{
    writeAutoGenWarningComment (out);

    out << "    This is the header file that your files should include in order to get all the" << newLine
        << "    JUCE library headers. You should avoid including the JUCE headers directly in" << newLine
        << "    your own source files, because that wouldn't pick up the correct configuration" << newLine
        << "    options for your app." << newLine
        << newLine
        << "*/" << newLine << newLine;

    out << "#pragma once" << newLine << newLine;

    if (getAppConfigFile().exists() && project.shouldUseAppConfig())
        out << CodeHelpers::createIncludeStatement (Project::getAppConfigFilename()) << newLine;

    if (modules.size() > 0)
    {
        out << newLine;

        for (auto* module : modules)
            module->writeIncludes (*this, out);

        out << newLine;
    }

    if (hasBinaryData && project.shouldIncludeBinaryInJuceHeader())
        out << CodeHelpers::createIncludeStatement (project.getBinaryDataHeaderFile(), getAppConfigFile()) << newLine;

    out << newLine
        << "#if defined (JUCE_PROJUCER_VERSION) && JUCE_PROJUCER_VERSION < JUCE_VERSION" << newLine
        << " /** If you've hit this error then the version of the Projucer that was used to generate this project is" << newLine
        << "     older than the version of the JUCE modules being included. To fix this error, re-save your project" << newLine
        << "     using the latest version of the Projucer or, if you aren't using the Projucer to manage your project," << newLine
        << "     remove the JUCE_PROJUCER_VERSION define." << newLine
        << " */" << newLine
        << " #error \"This project was last saved using an outdated version of the Projucer! Re-save this project with the latest version to fix this error.\"" << newLine
        << "#endif" << newLine
        << newLine;

    if (project.shouldAddUsingNamespaceToJuceHeader())
        out << "#if ! DONT_SET_USING_JUCE_NAMESPACE" << newLine
            << " // If your code uses a lot of JUCE classes, then this will obviously save you" << newLine
            << " // a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE." << newLine
            << " using namespace juce;" << newLine
            << "#endif" << newLine;

    out << newLine
        << "#if ! JUCE_DONT_DECLARE_PROJECTINFO" << newLine
        << "namespace ProjectInfo" << newLine
        << "{" << newLine
        << "    const char* const  projectName    = " << CppTokeniserFunctions::addEscapeChars (project.getProjectNameString()).quoted() << ";" << newLine
        << "    const char* const  companyName    = " << CppTokeniserFunctions::addEscapeChars (project.getCompanyNameString()).quoted() << ";" << newLine
        << "    const char* const  versionString  = " << CppTokeniserFunctions::addEscapeChars (project.getVersionString()).quoted() << ";" << newLine
        << "    const int          versionNumber  = " << project.getVersionAsHex() << ";" << newLine
        << "}" << newLine
        << "#endif" << newLine;
}

void ProjectSaver::writeAppHeader (const OwnedArray<LibraryModule>& modules)
{
    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

    writeAppHeader (mem, modules);
    saveGeneratedFile (Project::getJuceSourceHFilename(), mem);
}

void ProjectSaver::writeModuleCppWrappers (const OwnedArray<LibraryModule>& modules)
{
    for (auto* module : modules)
    {
        for (auto& cu : module->getAllCompileUnits())
        {
            MemoryOutputStream mem;
            mem.setNewLineString (projectLineFeed);

            writeAutoGenWarningComment (mem);

            mem << "*/" << newLine << newLine;

            if (project.shouldUseAppConfig())
                mem << "#include " << Project::getAppConfigFilename().quoted() << newLine;

            mem << "#include <";

            if (cu.file.getFileExtension() != ".r")   // .r files are included without the path
                mem << module->getID() << "/";

            mem << cu.file.getFileName() << ">" << newLine;

            replaceFileIfDifferent (generatedCodeFolder.getChildFile (cu.getFilenameForProxyFile()), mem);
        }
    }
}

void ProjectSaver::writeBinaryDataFiles()
{
    auto binaryDataH = project.getBinaryDataHeaderFile();

    JucerResourceFile resourceFile (project);

    if (resourceFile.getNumFiles() > 0)
    {
        auto dataNamespace = project.getBinaryDataNamespaceString().trim();

        if (dataNamespace.isEmpty())
            dataNamespace = "BinaryData";

        resourceFile.setClassName (dataNamespace);

        auto maxSize = project.getMaxBinaryFileSize();

        if (maxSize <= 0)
            maxSize = 10 * 1024 * 1024;

        Array<File> binaryDataFiles;
        auto r = resourceFile.write (maxSize);

        if (r.result.wasOk())
        {
            hasBinaryData = true;

            for (auto& f : r.filesCreated)
            {
                filesCreated.add (f);
                generatedFilesGroup.addFileRetainingSortOrder (f, ! f.hasFileExtension (".h"));
            }
        }
        else
        {
            addError (r.result.getErrorMessage());
        }
    }
    else
    {
        for (int i = 20; --i >= 0;)
            project.getBinaryDataCppFile (i).deleteFile();

        binaryDataH.deleteFile();
    }
}

void ProjectSaver::writeLV2Defines (MemoryOutputStream& mem)
{
    String templateFile { BinaryData::JuceLV2Defines_h_in };

    const auto isValidUri = [&] (const String& text) { return URL (text).isWellFormed(); };

    if (! isValidUri (project.getLV2URI()))
    {
        addError ("LV2 URI is malformed.");
        return;
    }

    mem << templateFile.replace ("${JUCE_LV2URI}",    project.getLV2URI());
}

void ProjectSaver::writeReadmeFile()
{
    MemoryOutputStream out;
    out.setNewLineString (projectLineFeed);

    out << newLine
        << " Important Note!!" << newLine
        << " ================" << newLine
        << newLine
        << "The purpose of this folder is to contain files that are auto-generated by the Projucer," << newLine
        << "and ALL files in this folder will be mercilessly DELETED and completely re-written whenever" << newLine
        << "the Projucer saves your project." << newLine
        << newLine
        << "Therefore, it's a bad idea to make any manual changes to the files in here, or to" << newLine
        << "put any of your own files in here if you don't want to lose them. (Of course you may choose" << newLine
        << "to add the folder's contents to your version-control system so that you can re-merge your own" << newLine
        << "modifications after the Projucer has saved its changes)." << newLine;

    replaceFileIfDifferent (generatedCodeFolder.getChildFile ("ReadMe.txt"), out);
}

String ProjectSaver::getAudioPluginDefines() const
{
    const auto flags = project.getAudioPluginFlags();

    if (flags.size() == 0)
        return {};

    MemoryOutputStream mem;
    mem.setNewLineString (projectLineFeed);

    mem << "//==============================================================================" << newLine
        << "// Audio plugin settings.." << newLine
        << newLine;

    for (int i = 0; i < flags.size(); ++i)
    {
        mem << "#ifndef  " << flags.getAllKeys()[i] << newLine
            << " #define " << flags.getAllKeys()[i].paddedRight (' ', 32) << "  "
                           << flags.getAllValues()[i] << newLine
            << "#endif" << newLine;
    }

    return mem.toString().trim();
}

void ProjectSaver::writeUnityScriptFile()
{
    auto unityScriptContents = replaceLineFeeds (BinaryData::UnityPluginGUIScript_cs_in,
                                                 projectLineFeed);

    auto projectName = Project::addUnityPluginPrefixIfNecessary (project.getProjectNameString());

    unityScriptContents = unityScriptContents.replace ("${plugin_class_name}",  projectName.replace (" ", "_"))
                                             .replace ("${plugin_name}",        projectName)
                                             .replace ("${plugin_vendor}",      project.getPluginManufacturerString())
                                             .replace ("${plugin_description}", project.getPluginDescriptionString());

    auto f = generatedCodeFolder.getChildFile (project.getUnityScriptName());

    MemoryOutputStream out;
    out << unityScriptContents;

    replaceFileIfDifferent (f, out);
}

void ProjectSaver::writeProjects (const OwnedArray<LibraryModule>& modules, ProjectExporter* specifiedExporterToSave)
{
    ThreadPool threadPool;

    // keep a copy of the basic generated files group, as each exporter may modify it.
    auto originalGeneratedGroup = generatedFilesGroup.state.createCopy();

    std::vector<std::unique_ptr<ProjectExporter>> exporters;

    try
    {
        for (Project::ExporterIterator exp (project); exp.next();)
        {
            if (specifiedExporterToSave != nullptr && exp->getUniqueName() != specifiedExporterToSave->getUniqueName())
                continue;

            exporters.push_back (std::move (exp.exporter));
        }

        for (auto& exporter : exporters)
        {
            exporter->initialiseDependencyPathValues();

            if (exporter->getTargetFolder().createDirectory())
            {
                exporter->copyMainGroupFromProject();
                exporter->settings = exporter->settings.createCopy();

                exporter->addToExtraSearchPaths (build_tools::RelativePath ("JuceLibraryCode", build_tools::RelativePath::projectFolder));

                generatedFilesGroup.state = originalGeneratedGroup.createCopy();
                exporter->addSettingsForProjectType (project.getProjectType());

                for (auto* module : modules)
                    module->addSettingsForModuleToExporter (*exporter, *this);

                generatedFilesGroup.sortAlphabetically (true, true);
                exporter->getAllGroups().add (generatedFilesGroup);

                if (ProjucerApplication::getApp().isRunningCommandLine)
                    saveExporter (*exporter, modules);
                else
                    threadPool.addJob ([this, &exporter, &modules] { saveExporter (*exporter, modules); });
            }
            else
            {
                addError ("Can't create folder: " + exporter->getTargetFolder().getFullPathName());
            }
        }
    }
    catch (build_tools::SaveError& saveError)
    {
        addError (saveError.message);
    }

    while (threadPool.getNumJobs() > 0)
        Thread::sleep (10);
}

void ProjectSaver::runPostExportScript()
{
   #if JUCE_WINDOWS
    auto cmdString = project.getPostExportShellCommandWinString();
   #else
    auto cmdString = project.getPostExportShellCommandPosixString();
   #endif

    auto shellCommand = cmdString.replace ("%%1%%", project.getProjectFolder().getFullPathName());

    if (shellCommand.isNotEmpty())
    {
       #if JUCE_WINDOWS
        StringArray argList ("cmd.exe", "/c");
       #else
        StringArray argList ("/bin/sh", "-c");
       #endif

        argList.add (shellCommand);
        ChildProcess shellProcess;

        if (! shellProcess.start (argList))
        {
            addError ("Failed to run shell command: " + argList.joinIntoString (" "));
            return;
        }

        if (! shellProcess.waitForProcessToFinish (10000))
        {
            addError ("Timeout running shell command: " + argList.joinIntoString (" "));
            return;
        }

        auto exitCode = shellProcess.getExitCode();

        if (exitCode != 0)
            addError ("Shell command: " + argList.joinIntoString (" ") + " failed with exit code: " + String (exitCode));
    }
}

void ProjectSaver::saveExporter (ProjectExporter& exporter, const OwnedArray<LibraryModule>& modules)
{
    try
    {
        exporter.create (modules);

        auto outputString = "Finished saving: " + exporter.getUniqueName();

        if (MessageManager::getInstance()->isThisTheMessageThread())
            std::cout <<  outputString << std::endl;
        else
            MessageManager::callAsync ([outputString] { std::cout <<  outputString << std::endl; });
    }
    catch (build_tools::SaveError& error)
    {
        addError (error.message);
    }
}
