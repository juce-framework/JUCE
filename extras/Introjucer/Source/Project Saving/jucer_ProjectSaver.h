/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_PROJECTSAVER_H_INCLUDED
#define JUCER_PROJECTSAVER_H_INCLUDED

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
          generatedFilesGroup (Project::Item::createGroup (project, getJuceCodeGroupName(), "__generatedcode__")),
          hasBinaryData (false)
    {
        generatedFilesGroup.setID (getGeneratedGroupID());
    }

    struct SaveThread  : public ThreadWithProgressWindow
    {
    public:
        SaveThread (ProjectSaver& ps)
            : ThreadWithProgressWindow ("Saving...", true, false),
              saver (ps), result (Result::ok())
        {}

        void run() override
        {
            setProgress (-1);
            result = saver.save (false);
        }

        ProjectSaver& saver;
        Result result;

        JUCE_DECLARE_NON_COPYABLE (SaveThread)
    };

    Result save (bool showProgressBox)
    {
        if (showProgressBox)
        {
            SaveThread thread (*this);
            thread.runThread();
            return thread.result;
        }

        const String appConfigUserContent (loadUserContentFromAppConfig());

        const File oldFile (project.getFile());
        project.setFile (projectFile);

        writeMainProjectFile();

        OwnedArray<LibraryModule> modules;
        project.getModules().createRequiredModules (modules);

        if (errors.size() == 0)  writeAppConfigFile (modules, appConfigUserContent);
        if (errors.size() == 0)  writeBinaryDataFiles();
        if (errors.size() == 0)  writeAppHeader (modules);
        if (errors.size() == 0)  writeProjects (modules);
        if (errors.size() == 0)  writeAppConfigFile (modules, appConfigUserContent); // (this is repeated in case the projects added anything to it)

        if (errors.size() == 0 && generatedCodeFolder.exists())
            writeReadmeFile();

        if (generatedCodeFolder.exists())
            deleteUnwantedFilesIn (generatedCodeFolder);

        if (errors.size() > 0)
        {
            project.setFile (oldFile);
            return Result::fail (errors[0]);
        }

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
            return Project::Item (project, ValueTree());
        }

        const File file (generatedCodeFolder.getChildFile (filePath));

        if (replaceFileIfDifferent (file, newData))
            return addFileToGeneratedGroup (file);

        return Project::Item (project, ValueTree());
    }

    Project::Item addFileToGeneratedGroup (const File& file)
    {
        Project::Item item (generatedFilesGroup.findItemForFile (file));

        if (item.isValid())
            return item;

        generatedFilesGroup.addFile (file, -1, true);
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

private:
    const File projectFile, generatedCodeFolder;
    Project::Item generatedFilesGroup;
    String extraAppConfigContent;
    StringArray errors;
    CriticalSection errorLock;

    File appConfigFile;
    SortedSet<File> filesCreated;
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
        ScopedPointer <XmlElement> xml (project.getProjectRoot().createXml());
        jassert (xml != nullptr);

        if (xml != nullptr)
        {
            MemoryOutputStream mo;
            xml->writeToStream (mo, String::empty);
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
            userContent.add (String::empty);
            userContent.add ("// (You can add your own code in this section, and the Introjucer will not overwrite it)");
            userContent.add (String::empty);
        }

        return userContent.joinIntoString (newLine) + newLine;
    }

    void writeAppConfig (OutputStream& out, const OwnedArray<LibraryModule>& modules, const String& userContent)
    {
        writeAutoGenWarningComment (out);
        out << "    There's a section below where you can add your own custom code safely, and the" << newLine
            << "    Introjucer will preserve the contents of that block, but the best way to change" << newLine
            << "    any of these definitions is by using the Introjucer's project settings." << newLine
            << newLine
            << "    Any commented-out settings will assume their default values." << newLine
            << newLine
            << "*/" << newLine
            << newLine;

        const String headerGuard ("__JUCE_APPCONFIG_" + project.getProjectUID().toUpperCase() + "__");
        out << "#ifndef " << headerGuard << newLine
            << "#define " << headerGuard << newLine
            << newLine
            << "//==============================================================================" << newLine
            << "// [BEGIN_USER_CODE_SECTION]" << newLine
            << userContent
            << "// [END_USER_CODE_SECTION]" << newLine
            << newLine
            << "//==============================================================================" << newLine;

        const int longestName = findLongestModuleName (modules);

        for (int k = 0; k < modules.size(); ++k)
        {
            LibraryModule* const m = modules.getUnchecked(k);
            out << "#define JUCE_MODULE_AVAILABLE_" << m->getID()
                << String::repeatedString (" ", longestName + 5 - m->getID().length()) << " 1" << newLine;
        }

        out << newLine;

        {
            int isStandaloneApplication = 1;
            const ProjectType& type = project.getProjectType();

            if (type.isAudioPlugin() || type.isDynamicLibrary() || type.isBrowserPlugin())
                isStandaloneApplication = 0;

            out << "//==============================================================================" << newLine;
            out << "#ifndef    JUCE_STANDALONE_APPLICATION" << newLine;
            out << " #define   JUCE_STANDALONE_APPLICATION " << isStandaloneApplication << newLine;
            out << "#endif" << newLine;
        }

        out << newLine;

        for (int j = 0; j < modules.size(); ++j)
        {
            LibraryModule* const m = modules.getUnchecked(j);
            OwnedArray<Project::ConfigFlag> flags;
            m->getConfigFlags (project, flags);

            if (flags.size() > 0)
            {
                out << "//==============================================================================" << newLine
                    << "// " << m->getID() << " flags:" << newLine
                    << newLine;

                for (int i = 0; i < flags.size(); ++i)
                {
                    flags.getUnchecked(i)->value.referTo (project.getConfigFlag (flags.getUnchecked(i)->symbol));

                    const Project::ConfigFlag* const f = flags[i];
                    const String value (project.getConfigFlag (f->symbol).toString());

                    out << "#ifndef    " << f->symbol << newLine;

                    if (value == Project::configFlagEnabled)
                        out << " #define   " << f->symbol << " 1";
                    else if (value == Project::configFlagDisabled)
                        out << " #define   " << f->symbol << " 0";
                    else
                        out << " //#define " << f->symbol;

                    out << newLine
                        << "#endif" << newLine
                        << newLine;
                }
            }
        }

        if (extraAppConfigContent.isNotEmpty())
            out << newLine << extraAppConfigContent.trimEnd() << newLine;

        out << newLine
            << "#endif  // " << headerGuard << newLine;
    }

    void writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const String& userContent)
    {
        appConfigFile = getAppConfigFile();

        MemoryOutputStream mem;
        writeAppConfig (mem, modules, userContent);
        saveGeneratedFile (project.getAppConfigFilename(), mem);
    }

    void writeAppHeader (OutputStream& out, const OwnedArray<LibraryModule>& modules)
    {
        writeAutoGenWarningComment (out);

        out << "    This is the header file that your files should include in order to get all the" << newLine
            << "    JUCE library headers. You should avoid including the JUCE headers directly in" << newLine
            << "    your own source files, because that wouldn't pick up the correct configuration" << newLine
            << "    options for your app." << newLine
            << newLine
            << "*/" << newLine << newLine;

        String headerGuard ("__APPHEADERFILE_" + project.getProjectUID().toUpperCase() + "__");
        out << "#ifndef " << headerGuard << newLine
            << "#define " << headerGuard << newLine << newLine;

        if (appConfigFile.exists())
            out << CodeHelpers::createIncludeStatement (project.getAppConfigFilename()) << newLine;

        for (int i = 0; i < modules.size(); ++i)
            modules.getUnchecked(i)->writeIncludes (*this, out);

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
            << "#endif" << newLine
            << newLine
            << "#endif   // " << headerGuard << newLine;
    }

    void writeAppHeader (const OwnedArray<LibraryModule>& modules)
    {
        MemoryOutputStream mem;
        writeAppHeader (mem, modules);
        saveGeneratedFile (project.getJuceSourceHFilename(), mem);
    }

    void writeBinaryDataFiles()
    {
        const File binaryDataH (project.getBinaryDataHeaderFile());

        ResourceFile resourceFile (project);

        if (resourceFile.getNumFiles() > 0)
        {
            resourceFile.setClassName ("BinaryData");

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
                    generatedFilesGroup.addFile (f, -1, ! f.hasFileExtension (".h"));
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
            << "The purpose of this folder is to contain files that are auto-generated by the Introjucer," << newLine
            << "and ALL files in this folder will be mercilessly DELETED and completely re-written whenever" << newLine
            << "the Introjucer saves your project." << newLine
            << newLine
            << "Therefore, it's a bad idea to make any manual changes to the files in here, or to" << newLine
            << "put any of your own files in here if you don't want to lose them. (Of course you may choose" << newLine
            << "to add the folder's contents to your version-control system so that you can re-merge your own" << newLine
            << "modifications after the Introjucer has saved its changes)." << newLine;

        replaceFileIfDifferent (generatedCodeFolder.getChildFile ("ReadMe.txt"), out);
    }

    static void sortGroupRecursively (Project::Item group)
    {
        group.sortAlphabetically (true);

        for (int i = group.getNumChildren(); --i >= 0;)
            sortGroupRecursively (group.getChild(i));
    }

    void addError (const String& message)
    {
        const ScopedLock sl (errorLock);
        errors.add (message);
    }

    void writeProjects (const OwnedArray<LibraryModule>& modules)
    {
        ThreadPool threadPool;

        // keep a copy of the basic generated files group, as each exporter may modify it.
        const ValueTree originalGeneratedGroup (generatedFilesGroup.state.createCopy());

        for (Project::ExporterIterator exporter (project); exporter.next();)
        {
            if (exporter->getTargetFolder().createDirectory())
            {
                exporter->copyMainGroupFromProject();
                exporter->settings = exporter->settings.createCopy();

                exporter->addToExtraSearchPaths (RelativePath ("JuceLibraryCode", RelativePath::projectFolder));

                generatedFilesGroup.state = originalGeneratedGroup.createCopy();
                project.getProjectType().prepareExporter (*exporter);

                for (int j = 0; j < modules.size(); ++j)
                    modules.getUnchecked(j)->prepareExporter (*exporter, *this);

                sortGroupRecursively (generatedFilesGroup);
                exporter->getAllGroups().add (generatedFilesGroup);

                threadPool.addJob (new ExporterJob (*this, exporter.exporter.release(), modules), true);
            }
            else
            {
                addError ("Can't create folder: " + exporter->getTargetFolder().getFullPathName());
            }
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

        JobStatus runJob()
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


#endif   // JUCER_PROJECTSAVER_H_INCLUDED
