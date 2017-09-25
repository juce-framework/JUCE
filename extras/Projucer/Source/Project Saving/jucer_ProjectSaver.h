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

#include "jucer_ResourceFile.h"
#include "../Project/jucer_Module.h"
#include "jucer_ProjectExporter.h"


//==============================================================================
class ProjectSaver
{
public:
    ProjectSaver (Project& p, const File& file)
        : project (p),
          projectFile (file),
          generatedCodeFolder (project.getGeneratedCodeFolder()),
          generatedFilesGroup (Project::Item::createGroup (project, getJuceCodeGroupName(), "__generatedcode__", true)),
          hasBinaryData (false)
    {
        generatedFilesGroup.setID (getGeneratedGroupID());
    }

    struct SaveThread  : public ThreadWithProgressWindow
    {
    public:
        SaveThread (ProjectSaver& ps, bool wait, const String& exp)
            : ThreadWithProgressWindow ("Saving...", true, false),
              saver (ps), result (Result::ok()),
              shouldWaitAfterSaving (wait),
              specifiedExporterToSave (exp)
        {}

        void run() override
        {
            setProgress (-1);
            result = saver.save (false, shouldWaitAfterSaving, specifiedExporterToSave);
        }

        ProjectSaver& saver;
        Result result;
        bool shouldWaitAfterSaving;
        String specifiedExporterToSave;

        JUCE_DECLARE_NON_COPYABLE (SaveThread)
    };

    Result save (bool showProgressBox, bool waitAfterSaving, const String& specifiedExporterToSave)
    {
        if (showProgressBox)
        {
            SaveThread thread (*this, waitAfterSaving, specifiedExporterToSave);
            thread.runThread();
            return thread.result;
        }

        const String appConfigUserContent (loadUserContentFromAppConfig());

        const File oldFile (project.getFile());
        project.setFile (projectFile);

        OwnedArray<LibraryModule> modules;
        project.getModules().createRequiredModules (modules);

        checkModuleValidity (modules);

        if (errors.size() == 0)
        {
            writeMainProjectFile();
            project.updateModificationTime();

            writeAppConfigFile (modules, appConfigUserContent);
            writeBinaryDataFiles();
            writeAppHeader (modules);
            writeModuleCppWrappers (modules);
            writeProjects (modules, specifiedExporterToSave);
            writeAppConfigFile (modules, appConfigUserContent); // (this is repeated in case the projects added anything to it)

            if (generatedCodeFolder.exists())
                writeReadmeFile();
        }

        if (generatedCodeFolder.exists())
            deleteUnwantedFilesIn (generatedCodeFolder);

        if (errors.size() > 0)
        {
            project.setFile (oldFile);
            return Result::fail (errors[0]);
        }

        // Workaround for a bug where Xcode thinks the project is invalid if opened immedietely
        // after writing
        if (waitAfterSaving)
            Thread::sleep (2000);

        return Result::ok();
    }

    Result saveResourcesOnly()
    {
        writeBinaryDataFiles();

        if (errors.size() > 0)
            return Result::fail (errors[0]);

        return Result::ok();
    }

    Project::Item saveGeneratedFile (const String& filePath, const MemoryOutputStream& newData)
    {
        if (! generatedCodeFolder.createDirectory())
        {
            addError ("Couldn't create folder: " + generatedCodeFolder.getFullPathName());
            return Project::Item (project, ValueTree(), false);
        }

        const File file (generatedCodeFolder.getChildFile (filePath));

        if (replaceFileIfDifferent (file, newData))
            return addFileToGeneratedGroup (file);

        return Project::Item (project, ValueTree(), true);
    }

    Project::Item addFileToGeneratedGroup (const File& file)
    {
        Project::Item item (generatedFilesGroup.findItemForFile (file));

        if (item.isValid())
            return item;

        generatedFilesGroup.addFileAtIndex (file, -1, true);
        return generatedFilesGroup.findItemForFile (file);
    }

    void setExtraAppConfigFileContent (const String& content)
    {
        extraAppConfigContent = content;
    }

    static void writeAutoGenWarningComment (OutputStream& out)
    {
        out << "/*" << newLine << newLine
            << "    IMPORTANT! This file is auto-generated each time you save your" << newLine
            << "    project - if you alter its contents, your changes may be overwritten!" << newLine
            << newLine;
    }

    static const char* getGeneratedGroupID() noexcept               { return "__jucelibfiles"; }
    Project::Item& getGeneratedCodeGroup()                          { return generatedFilesGroup; }

    static String getJuceCodeGroupName()                            { return "Juce Library Code"; }

    File getGeneratedCodeFolder() const                             { return generatedCodeFolder; }

    bool replaceFileIfDifferent (const File& f, const MemoryOutputStream& newData)
    {
        filesCreated.add (f);

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (f, newData))
        {
            addError ("Can't write to file: " + f.getFullPathName());
            return false;
        }

        return true;
    }

    static bool shouldFolderBeIgnoredWhenCopying (const File& f)
    {
        return f.getFileName() == ".git" || f.getFileName() == ".svn" || f.getFileName() == ".cvs";
    }

    bool copyFolder (const File& source, const File& dest)
    {
        if (source.isDirectory() && dest.createDirectory())
        {
            Array<File> subFiles;
            source.findChildFiles (subFiles, File::findFiles, false);

            for (int i = 0; i < subFiles.size(); ++i)
            {
                const File f (subFiles.getReference(i));
                const File target (dest.getChildFile (f.getFileName()));
                filesCreated.add (target);

                if (! f.copyFileTo (target))
                    return false;
            }

            Array<File> subFolders;
            source.findChildFiles (subFolders, File::findDirectories, false);

            for (int i = 0; i < subFolders.size(); ++i)
            {
                const File f (subFolders.getReference(i));

                if (! shouldFolderBeIgnoredWhenCopying (f))
                    if (! copyFolder (f, dest.getChildFile (f.getFileName())))
                        return false;
            }

            return true;
        }

        return false;
    }

    Project& project;
    SortedSet<File> filesCreated;

private:
    const File projectFile, generatedCodeFolder;
    Project::Item generatedFilesGroup;
    String extraAppConfigContent;
    StringArray errors;
    CriticalSection errorLock;

    File appConfigFile;
    bool hasBinaryData;

    // Recursively clears out any files in a folder that we didn't create, but avoids
    // any folders containing hidden files that might be used by version-control systems.
    bool deleteUnwantedFilesIn (const File& parent)
    {
        bool folderIsNowEmpty = true;
        DirectoryIterator i (parent, false, "*", File::findFilesAndDirectories);
        Array<File> filesToDelete;

        bool isFolder;
        while (i.next (&isFolder, nullptr, nullptr, nullptr, nullptr, nullptr))
        {
            const File f (i.getFile());

            if (filesCreated.contains (f) || shouldFileBeKept (f.getFileName()))
            {
                folderIsNowEmpty = false;
            }
            else if (isFolder)
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
            filesToDelete.getReference(j).deleteRecursively();

        return folderIsNowEmpty;
    }

    static bool shouldFileBeKept (const String& filename)
    {
        static const char* filesToKeep[] = { ".svn", ".cvs", "CMakeLists.txt" };

        for (int i = 0; i < numElementsInArray (filesToKeep); ++i)
            if (filename == filesToKeep[i])
                return true;

        return false;
    }

    void writeMainProjectFile()
    {
        ScopedPointer<XmlElement> xml (project.getProjectRoot().createXml());
        jassert (xml != nullptr);

        if (xml != nullptr)
        {
            MemoryOutputStream mo;
            xml->writeToStream (mo, String());
            replaceFileIfDifferent (projectFile, mo);
        }
    }

    static int findLongestModuleName (const OwnedArray<LibraryModule>& modules)
    {
        int longest = 0;

        for (int i = modules.size(); --i >= 0;)
            longest = jmax (longest, modules.getUnchecked(i)->getID().length());

        return longest;
    }

    File getAppConfigFile() const   { return generatedCodeFolder.getChildFile (project.getAppConfigFilename()); }

    String loadUserContentFromAppConfig() const
    {
        StringArray lines, userContent;
        lines.addLines (getAppConfigFile().loadFileAsString());
        bool foundCodeSection = false;

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
            userContent.add (String());
            userContent.add ("// (You can add your own code in this section, and the Projucer will not overwrite it)");
            userContent.add (String());
        }

        return userContent.joinIntoString (newLine) + newLine;
    }

    void checkModuleValidity (OwnedArray<LibraryModule>& modules)
    {
        if (project.getNumExporters() == 0)
        {
            addError ("No exporters found!\n"
                      "Please add an exporter before saving.");
            return;
        }

        for (LibraryModule** moduleIter = modules.begin(); moduleIter != modules.end(); ++moduleIter)
        {
            if (const LibraryModule* const module = *moduleIter)
            {
                if (! module->isValid())
                {
                    addError ("At least one of your JUCE module paths is invalid!\n"
                              "Please go to the Modules settings page and ensure each path points to the correct JUCE modules folder.");
                    return;
                }

                if (project.getModules().getExtraDependenciesNeeded (module->getID()).size() > 0)
                {
                    addError ("At least one of your modules has missing dependencies!\n"
                              "Please go to the settings page of the highlighted modules and add the required dependencies.");
                    return;
                }
            }
            else
            {
                // this should never happen!
                jassertfalse;
            }
        }
    }

    void writeAppConfig (MemoryOutputStream& out, const OwnedArray<LibraryModule>& modules, const String& userContent)
    {
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

        out << newLine
            << "/*" << newLine
            << "  ==============================================================================" << newLine
            << newLine
            << "   In accordance with the terms of the JUCE 5 End-Use License Agreement, the" << newLine
            << "   JUCE Code in SECTION A cannot be removed, changed or otherwise rendered" << newLine
            << "   ineffective unless you have a JUCE Indie or Pro license, or are using JUCE" << newLine
            << "   under the GPL v3 license." << newLine
            << newLine
            << "   End User License Agreement: www.juce.com/juce-5-licence" << newLine
            << "  ==============================================================================" << newLine
            << "*/" << newLine
            << newLine
            << "// BEGIN SECTION A" << newLine
            << newLine
            << "#ifndef JUCE_DISPLAY_SPLASH_SCREEN" << newLine
            << " #define JUCE_DISPLAY_SPLASH_SCREEN "   << (project.shouldDisplaySplashScreen().getValue() ? "1" : "0") << newLine
            << "#endif" << newLine << newLine

            << "#ifndef JUCE_REPORT_APP_USAGE" << newLine
            << " #define JUCE_REPORT_APP_USAGE "        << (project.shouldReportAppUsage().getValue()      ? "1" : "0") << newLine
            << "#endif" << newLine << newLine
            << newLine
            << "// END SECTION A" << newLine
            << newLine
            << "#define JUCE_USE_DARK_SPLASH_SCREEN "  << (project.splashScreenColour().toString() == "Dark" ? "1" : "0") << newLine;

        out << newLine
            << "//==============================================================================" << newLine;

        const int longestName = findLongestModuleName (modules);

        for (int k = 0; k < modules.size(); ++k)
        {
            LibraryModule* const m = modules.getUnchecked(k);
            out << "#define JUCE_MODULE_AVAILABLE_" << m->getID()
                << String::repeatedString (" ", longestName + 5 - m->getID().length()) << " 1" << newLine;
        }

        out << newLine << "#define JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED 1" << newLine;

        for (int j = 0; j < modules.size(); ++j)
        {
            LibraryModule* const m = modules.getUnchecked(j);
            OwnedArray<Project::ConfigFlag> flags;
            m->getConfigFlags (project, flags);

            if (flags.size() > 0)
            {
                out << newLine
                    << "//==============================================================================" << newLine
                    << "// " << m->getID() << " flags:" << newLine;

                for (int i = 0; i < flags.size(); ++i)
                {
                    flags.getUnchecked(i)->value.referTo (project.getConfigFlag (flags.getUnchecked(i)->symbol));

                    const Project::ConfigFlag* const f = flags[i];
                    const String value (project.getConfigFlag (f->symbol).toString());

                    out << newLine
                        << "#ifndef    " << f->symbol << newLine;

                    if (value == Project::configFlagEnabled)
                        out << " #define   " << f->symbol << " 1";
                    else if (value == Project::configFlagDisabled)
                        out << " #define   " << f->symbol << " 0";
                    else if (f->defaultValue.isEmpty())
                        out << " //#define " << f->symbol << " 1";
                    else
                        out << " #define " << f->symbol << " " << f->defaultValue;


                    out << newLine
                        << "#endif" << newLine;
                }
            }
        }

        {
            int isStandaloneApplication = 1;
            const ProjectType& type = project.getProjectType();

            if (type.isAudioPlugin() || type.isDynamicLibrary())
                isStandaloneApplication = 0;

            out << "//==============================================================================" << newLine
                << "#ifndef    JUCE_STANDALONE_APPLICATION" << newLine
                << " #if defined(JucePlugin_Name) && defined(JucePlugin_Build_Standalone)" << newLine
                << "  #define  JUCE_STANDALONE_APPLICATION JucePlugin_Build_Standalone" << newLine
                << " #else" << newLine
                << "  #define  JUCE_STANDALONE_APPLICATION " << isStandaloneApplication << newLine
                << " #endif" << newLine
                << "#endif" << newLine;
        }

        if (extraAppConfigContent.isNotEmpty())
            out << newLine << extraAppConfigContent.trimEnd() << newLine;
    }

    void writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const String& userContent)
    {
        appConfigFile = getAppConfigFile();

        MemoryOutputStream mem;
        writeAppConfig (mem, modules, userContent);
        saveGeneratedFile (project.getAppConfigFilename(), mem);
    }

    void writeAppHeader (MemoryOutputStream& out, const OwnedArray<LibraryModule>& modules)
    {
        writeAutoGenWarningComment (out);

        out << "    This is the header file that your files should include in order to get all the" << newLine
            << "    JUCE library headers. You should avoid including the JUCE headers directly in" << newLine
            << "    your own source files, because that wouldn't pick up the correct configuration" << newLine
            << "    options for your app." << newLine
            << newLine
            << "*/" << newLine << newLine;

        out << "#pragma once" << newLine << newLine;

        if (appConfigFile.exists())
            out << CodeHelpers::createIncludeStatement (project.getAppConfigFilename()) << newLine;

        if (modules.size() > 0)
        {
            out << newLine;

            for (int i = 0; i < modules.size(); ++i)
                modules.getUnchecked(i)->writeIncludes (*this, out);

            out << newLine;
        }

        if (hasBinaryData && project.shouldIncludeBinaryInAppConfig().getValue())
            out << CodeHelpers::createIncludeStatement (project.getBinaryDataHeaderFile(), appConfigFile) << newLine;

        out << newLine
            << "#if ! DONT_SET_USING_JUCE_NAMESPACE" << newLine
            << " // If your code uses a lot of JUCE classes, then this will obviously save you" << newLine
            << " // a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE." << newLine
            << " using namespace juce;" << newLine
            << "#endif" << newLine
            << newLine
            << "#if ! JUCE_DONT_DECLARE_PROJECTINFO" << newLine
            << "namespace ProjectInfo" << newLine
            << "{" << newLine
            << "    const char* const  projectName    = " << CppTokeniserFunctions::addEscapeChars (project.getTitle()).quoted() << ";" << newLine
            << "    const char* const  versionString  = " << CppTokeniserFunctions::addEscapeChars (project.getVersionString()).quoted() << ";" << newLine
            << "    const int          versionNumber  = " << project.getVersionAsHex() << ";" << newLine
            << "}" << newLine
            << "#endif" << newLine;
    }

    void writeAppHeader (const OwnedArray<LibraryModule>& modules)
    {
        MemoryOutputStream mem;
        writeAppHeader (mem, modules);
        saveGeneratedFile (project.getJuceSourceHFilename(), mem);
    }

    void writeModuleCppWrappers (const OwnedArray<LibraryModule>& modules)
    {
        for (auto* module : modules)
        {
            for (auto& cu : module->getAllCompileUnits())
            {
                MemoryOutputStream mem;

                writeAutoGenWarningComment (mem);

                mem << "*/" << newLine
                    << newLine
                    << "#include " << project.getAppConfigFilename().quoted() << newLine
                    << "#include <";

                if (cu.file.getFileExtension() != ".r")   // .r files are included without the path
                    mem << module->getID() << "/";

                mem << cu.file.getFileName() << ">" << newLine;

                replaceFileIfDifferent (generatedCodeFolder.getChildFile (cu.getFilenameForProxyFile()), mem);
            }
        }
    }

    void writeBinaryDataFiles()
    {
        const File binaryDataH (project.getBinaryDataHeaderFile());

        ResourceFile resourceFile (project);

        if (resourceFile.getNumFiles() > 0)
        {
            auto dataNamespace = project.binaryDataNamespace().toString().trim();
            if (dataNamespace.isEmpty())
                dataNamespace = "BinaryData";

            resourceFile.setClassName (dataNamespace);

            Array<File> binaryDataFiles;

            int maxSize = project.getMaxBinaryFileSize().getValue();
            if (maxSize <= 0)
                maxSize = 10 * 1024 * 1024;

            Result r (resourceFile.write (binaryDataFiles, maxSize));

            if (r.wasOk())
            {
                hasBinaryData = true;

                for (int i = 0; i < binaryDataFiles.size(); ++i)
                {
                    const File& f = binaryDataFiles.getReference(i);

                    filesCreated.add (f);
                    generatedFilesGroup.addFileRetainingSortOrder (f, ! f.hasFileExtension (".h"));
                }
            }
            else
            {
                addError (r.getErrorMessage());
            }
        }
        else
        {
            for (int i = 20; --i >= 0;)
                project.getBinaryDataCppFile (i).deleteFile();

            binaryDataH.deleteFile();
        }
    }

    void writeReadmeFile()
    {
        MemoryOutputStream out;
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

    void addError (const String& message)
    {
        const ScopedLock sl (errorLock);
        errors.add (message);
    }

    void writePluginCharacteristicsFile();

    void writeProjects (const OwnedArray<LibraryModule>& modules, const String& specifiedExporterToSave)
    {
        ThreadPool threadPool;

        // keep a copy of the basic generated files group, as each exporter may modify it.
        const ValueTree originalGeneratedGroup (generatedFilesGroup.state.createCopy());

        try
        {
            for (Project::ExporterIterator exporter (project); exporter.next();)
            {
                if (specifiedExporterToSave.isNotEmpty() && exporter->getName() != specifiedExporterToSave)
                    continue;

                exporter->initialiseDependencyPathValues();

                if (exporter->getTargetFolder().createDirectory())
                {
                    exporter->copyMainGroupFromProject();
                    exporter->settings = exporter->settings.createCopy();

                    exporter->addToExtraSearchPaths (RelativePath ("JuceLibraryCode", RelativePath::projectFolder));

                    generatedFilesGroup.state = originalGeneratedGroup.createCopy();
                    exporter->addSettingsForProjectType (project.getProjectType());

                    for (auto& module: modules)
                        module->addSettingsForModuleToExporter (*exporter, *this);

                    if (project.getProjectType().isAudioPlugin())
                        writePluginCharacteristicsFile();

                    generatedFilesGroup.sortAlphabetically (true, true);
                    exporter->getAllGroups().add (generatedFilesGroup);

                    threadPool.addJob (new ExporterJob (*this, exporter.exporter.release(), modules), true);
                }
                else
                {
                    addError ("Can't create folder: " + exporter->getTargetFolder().getFullPathName());
                }
            }
        }
        catch (ProjectExporter::SaveError& saveError)
        {
            addError (saveError.message);
        }

        while (threadPool.getNumJobs() > 0)
            Thread::sleep (10);
    }

    class ExporterJob   : public ThreadPoolJob
    {
    public:
        ExporterJob (ProjectSaver& ps, ProjectExporter* pe,
                     const OwnedArray<LibraryModule>& moduleList)
            : ThreadPoolJob ("export"),
              owner (ps), exporter (pe), modules (moduleList)
        {
        }

        JobStatus runJob() override
        {
            try
            {
                exporter->create (modules);
                std::cout << "Finished saving: " << exporter->getName() << std::endl;
            }
            catch (ProjectExporter::SaveError& error)
            {
                owner.addError (error.message);
            }

            return jobHasFinished;
        }

    private:
        ProjectSaver& owner;
        ScopedPointer<ProjectExporter> exporter;
        const OwnedArray<LibraryModule>& modules;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterJob)
    };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSaver)
};
