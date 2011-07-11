/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_PROJECTSAVER_JUCEHEADER__
#define __JUCER_PROJECTSAVER_JUCEHEADER__

#include "jucer_ResourceFile.h"


//==============================================================================
class ProjectSaver
{
public:
    ProjectSaver (Project& project_, const File& projectFile_)
        : project (project_), projectFile (projectFile_),
          generatedFilesGroup (Project::Item::createGroup (project, project.getJuceCodeGroupName()))
    {
        generatedFilesGroup.setID (getGeneratedGroupID());
    }

    Project& getProject() noexcept      { return project; }

    String save()
    {
        jassert (generatedFilesGroup.getNumChildren() == 0); // this method can't be called more than once!

        const File oldFile (project.getFile());
        project.setFile (projectFile);

        writeMainProjectFile();

        if (! project.getGeneratedCodeFolder().createDirectory())
            errors.add ("Couldn't create folder: " + project.getGeneratedCodeFolder().getFullPathName());

        if (errors.size() == 0)
            writeAppConfigFile();

        if (errors.size() == 0)
            writeBinaryDataFiles();

        if (errors.size() == 0)
            writeAppHeader();

        if (errors.size() == 0)
            writeProjects();

        if (errors.size() > 0)
            project.setFile (oldFile);

        return errors[0];
    }

    bool saveGeneratedFile (const String& filePath, const MemoryOutputStream& newData)
    {
        const File file (project.getGeneratedCodeFolder().getChildFile (filePath));

        if (replaceFileIfDifferent (file, newData))
        {
            if (! generatedFilesGroup.findItemForFile (file).isValid())
                generatedFilesGroup.addFile (file, -1);

            return true;
        }

        return false;
    }

    static void writeAutoGenWarningComment (OutputStream& out)
    {
        out << "/*" << newLine << newLine
            << "    IMPORTANT! This file is auto-generated each time you save your" << newLine
            << "    project - if you alter its contents, your changes may be overwritten!" << newLine
            << newLine;
    }

    static const char* getGeneratedGroupID() noexcept       { return "__jucelibfiles"; }

private:
    Project& project;
    const File projectFile;
    Project::Item generatedFilesGroup;
    StringArray errors;

    File appConfigFile, binaryDataCpp;

    void writeMainProjectFile()
    {
        ScopedPointer <XmlElement> xml (project.getProjectRoot().createXml());
        jassert (xml != nullptr);

        if (xml != nullptr)
        {
           #if JUCE_DEBUG
            {
                MemoryOutputStream mo;
                project.getProjectRoot().writeToStream (mo);

                MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
                ValueTree v = ValueTree::readFromStream (mi);
                ScopedPointer <XmlElement> xml2 (v.createXml());

                // This bit just tests that ValueTree save/load works reliably.. Let me know if this asserts for you!
                jassert (xml->isEquivalentTo (xml2, true));
            }
           #endif

            MemoryOutputStream mo;
            xml->writeToStream (mo, String::empty);
            replaceFileIfDifferent (projectFile, mo);
        }
    }

    bool writeAppConfig (OutputStream& out)
    {
        writeAutoGenWarningComment (out);
        out << "    If you want to change any of these values, use the Introjucer to do so, rather than" << newLine
            << "    editing this file directly!" << newLine
            << newLine
            << "    Any commented-out settings will fall back to using the default values that" << newLine
            << "    they are given in juce_Config.h" << newLine
            << newLine
            << "*/" << newLine << newLine;

        bool notActive = project.getJuceLinkageMode() == Project::useLinkedJuce
                            || project.getJuceLinkageMode() == Project::notLinkedToJuce;
        if (notActive)
            out << "/* NOTE: These configs aren't available when you're linking to the juce library statically!" << newLine
                << "         If you need to set a configuration that differs from the default, you'll need" << newLine
                << "         to include the amalgamated Juce files." << newLine << newLine;

        OwnedArray <Project::ConfigFlag> flags;
        project.getAllConfigFlags (flags);

        for (int i = 0; i < flags.size(); ++i)
        {
            const Project::ConfigFlag* const f = flags[i];
            const String value (f->value.toString());

            if (value != Project::configFlagEnabled && value != Project::configFlagDisabled)
                out << "//#define  ";
            else
                out << "#define    ";

            out << f->symbol;

            if (value == Project::configFlagEnabled)
                out << " 1";
            else if (value == Project::configFlagDisabled)
                out << " 0";

            out << newLine;
        }

        if (notActive)
            out << newLine << "*/" << newLine;

        return flags.size() > 0;
    }

    void writeAppConfigFile()
    {
        appConfigFile = project.getGeneratedCodeFolder().getChildFile (project.getAppConfigFilename());

        MemoryOutputStream mem;
        if (writeAppConfig (mem))
            saveGeneratedFile (project.getAppConfigFilename(), mem);
        else
            appConfigFile.deleteFile();
    }

    void writeAppHeader (OutputStream& out)
    {
        writeAutoGenWarningComment (out);

        out << "    This is the header file that your files should include in order to get all the" << newLine
            << "    Juce library headers. You should NOT include juce.h or juce_amalgamated.h directly in" << newLine
            << "    your own source files, because that wouldn't pick up the correct Juce configuration" << newLine
            << "    options for your app." << newLine
            << newLine
            << "*/" << newLine << newLine;

        String headerGuard ("__APPHEADERFILE_" + project.getProjectUID().toUpperCase() + "__");
        out << "#ifndef " << headerGuard << newLine
            << "#define " << headerGuard << newLine << newLine;

        if (appConfigFile.exists())
            out << CodeHelpers::createIncludeStatement (project.getAppConfigFilename()) << newLine;

        {
            OwnedArray<LibraryModule> modules;
            project.getProjectType().createRequiredModules (project, modules);

            StringArray paths, guards;

            for (int i = 0; i < modules.size(); ++i)
                modules.getUnchecked(i)->getHeaderFiles (project, paths, guards);

            StringArray uniquePaths (paths);
            uniquePaths.removeDuplicates (false);

            if (uniquePaths.size() == 1)
            {
                out << "#include " << paths[0] << newLine;
            }
            else
            {
                int i = paths.size();
                for (; --i >= 0;)
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

                for (i = 0; i < paths.size(); ++i)
                {
                    out << (i == 0 ? "#if " : "#elif ") << guards[i] << newLine
                        << " #include " << paths[i] << newLine;
                }

                out << "#endif" << newLine;
            }
        }

        if (binaryDataCpp.exists())
            out << CodeHelpers::createIncludeStatement (binaryDataCpp.withFileExtension (".h"), appConfigFile) << newLine;

        out << newLine
            << "namespace ProjectInfo" << newLine
            << "{" << newLine
            << "    const char* const  projectName    = " << CodeHelpers::addEscapeChars (project.getProjectName().toString()).quoted() << ";" << newLine
            << "    const char* const  versionString  = " << CodeHelpers::addEscapeChars (project.getVersion().toString()).quoted() << ";" << newLine
            << "    const int          versionNumber  = " << project.getVersionAsHex() << ";" << newLine
            << "}" << newLine
            << newLine
            << "#endif   // " << headerGuard << newLine;
    }

    void writeAppHeader()
    {
        if (project.getJuceLinkageMode() != Project::notLinkedToJuce
             || ! project.getProjectType().isLibrary())
        {
            MemoryOutputStream mem;
            writeAppHeader (mem);
            saveGeneratedFile (project.getJuceSourceHFilename(), mem);
        }
        else
        {
            project.getAppIncludeFile().deleteFile();
        }
    }

    void writeBinaryDataFiles()
    {
        binaryDataCpp = project.getGeneratedCodeFolder().getChildFile ("BinaryData.cpp");

        ResourceFile resourceFile (project);

        if (resourceFile.getNumFiles() > 0)
        {
            resourceFile.setClassName ("BinaryData");

            if (resourceFile.write (binaryDataCpp))
            {
                generatedFilesGroup.addFile (binaryDataCpp, -1);
                generatedFilesGroup.addFile (binaryDataCpp.withFileExtension (".h"), -1);
            }
            else
            {
                errors.add ("Can't create binary resources file: " + binaryDataCpp.getFullPathName());
            }
        }
        else
        {
            binaryDataCpp.deleteFile();
            binaryDataCpp.withFileExtension ("h").deleteFile();
        }
    }

    void writeProjects()
    {
        for (int i = project.getNumExporters(); --i >= 0;)
        {
            ScopedPointer <ProjectExporter> exporter (project.createExporter (i));
            std::cout << "Writing files for: " << exporter->getName() << std::endl;

            if (exporter->getTargetFolder().createDirectory())
            {
                project.getProjectType().prepareExporter (*exporter);

                // start with a copy of the basic files, as each exporter may modify it.
                const ValueTree generatedGroupCopy (generatedFilesGroup.getNode().createCopy());

                for (int j = 0; j < exporter->libraryModules.size(); ++j)
                    exporter->libraryModules.getUnchecked(j)->prepareExporter (*exporter, *this);

                exporter->groups.add (generatedFilesGroup);

                try
                {
                    exporter->create();
                }
                catch (ProjectExporter::SaveError& error)
                {
                    errors.add (error.message);
                }

                generatedFilesGroup.getNode() = generatedGroupCopy;
            }
            else
            {
                errors.add ("Can't create folder: " + exporter->getTargetFolder().getFullPathName());
            }
        }
    }

    bool replaceFileIfDifferent (const File& f, const MemoryOutputStream& newData)
    {
        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (f, newData))
        {
            errors.add ("Can't write to file: " + f.getFullPathName());
            return false;
        }

        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSaver);
};


#endif
