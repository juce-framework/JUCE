/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Application/jucer_Headers.h"
#include "jucer_ResourceFile.h"
#include "../Project/Modules/jucer_Modules.h"
#include "jucer_ProjectExporter.h"

//==============================================================================
class ProjectSaver
{
public:
    ProjectSaver (Project& projectToSave);

    Result save (ProjectExporter* exporterToSave = nullptr);
    Result saveResourcesOnly();
    void saveBasicProjectItems (const OwnedArray<LibraryModule>& modules, const String& appConfigUserContent);
    Result saveContentNeededForLiveBuild();

    Project& getProject()  { return project; }

    Project::Item addFileToGeneratedGroup (const File& file);
    bool copyFolder (const File& source, const File& dest);

    static String getJuceCodeGroupName()  { return "JUCE Library Code"; }

private:
    //==============================================================================
    struct SaveThreadWithProgressWindow  : public ThreadWithProgressWindow
    {
    public:
        SaveThreadWithProgressWindow (ProjectSaver& ps, ProjectExporter* exporterToSave)
            : ThreadWithProgressWindow ("Saving...", true, false),
              saver (ps),
              specifiedExporterToSave (exporterToSave)
        {}

        void run() override
        {
            setProgress (-1);
            result = saver.saveProject (specifiedExporterToSave);
        }

        ProjectSaver& saver;
        Result result = Result::ok();
        ProjectExporter* specifiedExporterToSave;

        JUCE_DECLARE_NON_COPYABLE (SaveThreadWithProgressWindow)
    };

    //==============================================================================
    Project::Item saveGeneratedFile (const String& filePath, const MemoryOutputStream& newData);
    bool replaceFileIfDifferent (const File& f, const MemoryOutputStream& newData);
    bool deleteUnwantedFilesIn (const File& parent);

    void addError (const String& message);

    File getAppConfigFile() const;
    File getPluginDefinesFile() const;

    String loadUserContentFromAppConfig() const;
    String getAudioPluginDefines() const;
    OwnedArray<LibraryModule> getModules();

    Result saveProject (ProjectExporter* specifiedExporterToSave);

    template <typename WriterCallback>
    void writeOrRemoveGeneratedFile (const String& name, WriterCallback&& writerCallback);

    void writePluginDefines (MemoryOutputStream& outStream) const;
    void writePluginDefines();
    void writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const String& userContent);

    void writeMainProjectFile();
    void writeAppConfig (MemoryOutputStream& outStream, const OwnedArray<LibraryModule>& modules, const String& userContent);
    void writeAppHeader (MemoryOutputStream& outStream, const OwnedArray<LibraryModule>& modules);
    void writeAppHeader (const OwnedArray<LibraryModule>& modules);
    void writeModuleCppWrappers (const OwnedArray<LibraryModule>& modules);
    void writeBinaryDataFiles();
    void writeReadmeFile();
    void writePluginCharacteristicsFile();
    void writeUnityScriptFile();
    void writeProjects (const OwnedArray<LibraryModule>&, ProjectExporter*);
    void runPostExportScript();
    void saveExporter (ProjectExporter& exporter, const OwnedArray<LibraryModule>& modules);

    //==============================================================================
    Project& project;

    File generatedCodeFolder;
    Project::Item generatedFilesGroup;
    SortedSet<File> filesCreated;
    String projectLineFeed;

    CriticalSection errorLock;
    StringArray errors;

    bool hasBinaryData = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSaver)
};
