/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2017 - ROLI Ltd.

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "jucer_ProjectSaver.h"

#include "jucer_ProjectExport_CLion.h"

//==============================================================================
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

void ProjectSaver::writeProjects (const OwnedArray<LibraryModule>& modules, const String& specifiedExporterToSave, bool isCommandLineApp)
{
    ThreadPool threadPool;

    // keep a copy of the basic generated files group, as each exporter may modify it.
    auto originalGeneratedGroup = generatedFilesGroup.state.createCopy();

    CLionProjectExporter* clionExporter = nullptr;
    OwnedArray<ProjectExporter> exporters;

    try
    {
        for (Project::ExporterIterator exp (project); exp.next();)
        {
            if (specifiedExporterToSave.isNotEmpty() && exp->getName() != specifiedExporterToSave)
                continue;

            auto exporter = exporters.add (std::move (exp.exporter));

            exporter->initialiseDependencyPathValues();

            if (exporter->getTargetFolder().createDirectory())
            {
                if (exporter->isCLion())
                {
                    clionExporter = dynamic_cast<CLionProjectExporter*> (exporter);
                }
                else
                {
                    exporter->copyMainGroupFromProject();
                    exporter->settings = exporter->settings.createCopy();

                    exporter->addToExtraSearchPaths (build_tools::RelativePath ("JuceLibraryCode", build_tools::RelativePath::projectFolder));

                    generatedFilesGroup.state = originalGeneratedGroup.createCopy();
                    exporter->addSettingsForProjectType (project.getProjectType());

                    for (auto& module: modules)
                        module->addSettingsForModuleToExporter (*exporter, *this);

                    generatedFilesGroup.sortAlphabetically (true, true);
                    exporter->getAllGroups().add (generatedFilesGroup);
                }

                if (isCommandLineApp)
                    saveExporter (exporter, modules);
                else
                    threadPool.addJob (new ExporterJob (*this, exporter, modules), true);
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

    if (! isCommandLineApp)
        while (threadPool.getNumJobs() > 0)
            Thread::sleep (10);

    if (clionExporter != nullptr)
    {
        for (auto* exporter : exporters)
            clionExporter->writeCMakeListsExporterSection (exporter);

        std::cout << "Finished saving: " << clionExporter->getName() << std::endl;
    }
}
