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

#include "jucer_ProjectContentComponent.h"
#include "../Application/jucer_MainWindow.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "jucer_ProjectInformationComponent.h"
#include "jucer_TreeViewTypes.h"
#include "jucer_ProjectExporter.h"


//==============================================================================
ProjectContentComponent::ProjectContentComponent()
    : projectTree (0), project (0),
      currentDocument (0), resizerBar (0)
{
    setItemLayout (0, 100, 500, 300);
    setItemLayout (1, 4, 4, 4);
    setItemLayout (2, 100, 10000, 800);

    setOpaque (true);
    setWantsKeyboardFocus (true);
}

ProjectContentComponent::~ProjectContentComponent()
{
    setProject (0);
    contentView = 0;
    jassert (getNumChildComponents() == 0);
}

void ProjectContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour::greyLevel (0.8f));
}

void ProjectContentComponent::hasBeenMoved()
{
    resized();
}

void ProjectContentComponent::resized()
{
    Component* comps[] = { projectTree, resizerBar, contentView };
    layOutComponents (comps, 3, 0, 0, getWidth(), getHeight(), false, true);
}

void ProjectContentComponent::setProject (Project* newProject)
{
    if (project != newProject)
    {
        if (project != 0)
            project->removeChangeListener (this);

        if (projectTree != 0)
            projectTree->deleteRootItem();

        projectTree = 0;
        contentView = 0;
        resizerBar = 0;

        project = newProject;

        if (project != 0)
        {
            addAndMakeVisible (projectTree = new TreeView());
            projectTree->setRootItemVisible (true);
            projectTree->setMultiSelectEnabled (true);
            projectTree->setDefaultOpenness (true);
            projectTree->setColour (TreeView::backgroundColourId, Colour::greyLevel (0.93f));
            projectTree->setIndentSize (14);

            addAndMakeVisible (resizerBar = new StretchableLayoutResizerBar (this, 1, true));

            resized();

            projectTree->setRootItem (new GroupTreeViewItem (project->getMainGroup()));
            projectTree->getRootItem()->setOpen (true);

            project->addChangeListener (this);

            if (currentDocument == 0)
                invokeDirectly (CommandIDs::showProjectSettings, true);

            updateMissingFileStatuses();
        }
    }
}

void ProjectContentComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateMissingFileStatuses();
}

void ProjectContentComponent::updateMissingFileStatuses()
{
    if (projectTree != 0)
    {
        ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (projectTree->getRootItem());
        if (p != 0)
            p->checkFileStatus();
    }
}

bool ProjectContentComponent::showEditorForFile (const File& f)
{
    return showDocument (OpenDocumentManager::getInstance()
                           ->getDocumentForFile (project, f));
}

bool ProjectContentComponent::showDocument (OpenDocumentManager::Document* doc)
{
    if (doc == 0)
        return false;

    OpenDocumentManager::getInstance()->moveDocumentToTopOfStack (doc);

    if (doc->hasFileBeenModifiedExternally())
        doc->reloadFromFile();

    return setEditorComponent (doc->createEditor(), doc);
}

void ProjectContentComponent::hideDocument (OpenDocumentManager::Document* doc)
{
    if (doc == currentDocument)
    {
        currentDocument = 0;
        contentView = 0;
        updateMainWindowTitle();
        commandManager->commandStatusChanged();
    }
}

bool ProjectContentComponent::setEditorComponent (Component* editor, OpenDocumentManager::Document* doc)
{
    if (editor != 0)
    {
        contentView = editor;
        currentDocument = doc;
        addAndMakeVisible (editor);
        resized();
        updateMainWindowTitle();
        commandManager->commandStatusChanged();

        return true;
    }

    updateMainWindowTitle();
    return false;
}

void ProjectContentComponent::updateMainWindowTitle()
{
    MainWindow* mw = Component::findParentComponentOfClass ((MainWindow*) 0);

    if (mw != 0)
        mw->updateTitle (currentDocument != 0 ? currentDocument->getName() : String::empty);
}

ApplicationCommandTarget* ProjectContentComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void ProjectContentComponent::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::saveProject,
                              CommandIDs::saveProjectAs,
                              CommandIDs::closeProject,
                              CommandIDs::openInIDE,
                              CommandIDs::saveAndOpenInIDE,
                              CommandIDs::showProjectSettings,
                              StandardApplicationCommandIDs::del};

    commands.addArray (ids, numElementsInArray (ids));
}

void ProjectContentComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
    case CommandIDs::saveProject:
        result.setInfo ("Save Project",
                        "Saves the current project",
                        CommandCategories::general, 0);
        result.setActive (project != 0);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::saveProjectAs:
        result.setInfo ("Save Project As...",
                        "Saves the current project to a different filename",
                        CommandCategories::general, 0);
        result.setActive (project != 0);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::closeProject:
        result.setInfo ("Close Project",
                        "Closes the current project",
                        CommandCategories::general, 0);
        result.setActive (project != 0);
        result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::openInIDE:
       #if JUCE_MAC
        result.setInfo ("Open in XCode...",
       #elif JUCE_WINDOWS
        result.setInfo ("Open in Visual Studio...",
       #else
        result.setInfo ("Open as a Makefile...",
       #endif
                        "Launches the project in an external IDE",
                        CommandCategories::general, 0);
        result.setActive (project != 0);
        break;

    case CommandIDs::saveAndOpenInIDE:
       #if JUCE_MAC
        result.setInfo ("Save Project and Open in XCode...",
       #elif JUCE_WINDOWS
        result.setInfo ("Save Project and Open in Visual Studio...",
       #else
        result.setInfo ("Save Project and Open as a Makefile...",
       #endif
                        "Saves the project and launches it in an external IDE",
                        CommandCategories::general, 0);
        result.setActive (project != 0);
        result.defaultKeypresses.add (KeyPress ('l', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showProjectSettings:
        result.setInfo ("Show Project Build Settings",
                        "Shows the build options for the project",
                        CommandCategories::general, 0);
        result.setActive (project != 0);
        result.defaultKeypresses.add (KeyPress ('i', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case StandardApplicationCommandIDs::del:
        result.setInfo ("Delete", String::empty, CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (KeyPress::deleteKey, 0, 0));
        result.defaultKeypresses.add (KeyPress (KeyPress::backspaceKey, 0, 0));
        result.setActive (projectTree != 0);
        break;

    default:
        break;
    }
}

bool ProjectContentComponent::isCommandActive (const CommandID commandID)
{
    return project != 0;
}

bool ProjectContentComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::saveProject:
        if (project != 0)
            project->save (true, true);

        break;

    case CommandIDs::saveProjectAs:
        if (project != 0)
            project->saveAsInteractive (true);

        break;

    case CommandIDs::closeProject:
        {
            MainWindow* mw = Component::findParentComponentOfClass ((MainWindow*) 0);

            if (mw != 0)
                mw->closeCurrentProject();
        }

        break;

    case CommandIDs::openInIDE:
        if (project != 0)
        {
            ScopedPointer <ProjectExporter> exporter (ProjectExporter::createPlatformDefaultExporter (*project));
            exporter->launchProject();
        }
        break;

    case CommandIDs::saveAndOpenInIDE:
        if (project != 0 && project->save (true, true) == FileBasedDocument::savedOk)
        {
            ScopedPointer <ProjectExporter> exporter (ProjectExporter::createPlatformDefaultExporter (*project));
            exporter->launchProject();
        }
        break;

    case CommandIDs::showProjectSettings:
        if (projectTree != 0)
            projectTree->getRootItem()->setSelected (true, true);

        break;

    case StandardApplicationCommandIDs::del:
        if (projectTree != 0)
        {
            ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (projectTree->getRootItem());
            if (p != 0)
                p->deleteAllSelectedItems();
        }

        break;

    default:
        return false;
    }

    return true;
}
