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

    void save (Async async, ProjectExporter* exporterToSave, std::function<void (Result)> onCompletion);
    Result saveResourcesOnly();
    void saveBasicProjectItems (const OwnedArray<LibraryModule>& modules, const String& appConfigUserContent);

    Project& getProject()  { return project; }

    Project::Item addFileToGeneratedGroup (const File& file);
    bool copyFolder (const File& source, const File& dest);

    static String getJuceCodeGroupName()  { return "JUCE Library Code"; }

private:
    //==============================================================================
    struct SaveThreadWithProgressWindow  : public ThreadWithProgressWindow
    {
    public:
        SaveThreadWithProgressWindow (ProjectSaver& ps,
                                      ProjectExporter* exporterToSave,
                                      std::function<void (Result)> onCompletionIn)
            : ThreadWithProgressWindow ("Saving...", true, false),
              saver (ps),
              specifiedExporterToSave (exporterToSave),
              onCompletion (std::move (onCompletionIn))
        {
            jassert (onCompletion != nullptr);
        }

        void run() override
        {
            setProgress (-1);
            const auto result = saver.saveProject (specifiedExporterToSave);
            const auto callback = onCompletion;

            MessageManager::callAsync ([callback, result] { callback (result); });
        }

    private:
        ProjectSaver& saver;
        ProjectExporter* specifiedExporterToSave;
        std::function<void (Result)> onCompletion;

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
    void saveProjectAsync (ProjectExporter* exporterToSave, std::function<void (Result)> onCompletion);

    template <typename WriterCallback>
    void writeOrRemoveGeneratedFile (const String& name, WriterCallback&& writerCallback);

    void writePluginDefines (MemoryOutputStream& outStream) const;
    void writePluginDefines();
    void writeAppConfigFile (const OwnedArray<LibraryModule>& modules, const String& userContent);
    void writeLV2Defines (MemoryOutputStream&);

    void writeProjectFile();
    void writeAppConfig (MemoryOutputStream& outStream, const OwnedArray<LibraryModule>& modules, const String& userContent);
    void writeAppHeader (MemoryOutputStream& outStream, const OwnedArray<LibraryModule>& modules);
    void writeAppHeader (const OwnedArray<LibraryModule>& modules);
    void writeModuleCppWrappers (const OwnedArray<LibraryModule>& modules);
    void writeBinaryDataFiles();
    void writeReadmeFile();
    void writePluginCharacteristicsFile();
    void writeUnityScriptFile();
    void writeProjects (const OwnedArray<LibraryModule>&, ProjectExporter*);
    void writeLV2DefinesFile();
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

    std::unique_ptr<SaveThreadWithProgressWindow> saveThread;

    bool hasBinaryData = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSaver)
    JUCE_DECLARE_WEAK_REFERENCEABLE (ProjectSaver)
};
