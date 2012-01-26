/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
#include "../Project Saving/jucer_ProjectExporter.h"


//==============================================================================
ProjectContentComponent::ProjectContentComponent()
    : project (nullptr),
      currentDocument (nullptr)
{
    setOpaque (true);
    setWantsKeyboardFocus (true);

    treeSizeConstrainer.setMinimumWidth (100);
    treeSizeConstrainer.setMaximumWidth (500);
}

ProjectContentComponent::~ProjectContentComponent()
{
    setProject (nullptr);
    contentView = nullptr;
    jassert (getNumChildComponents() == 0);
}

void ProjectContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour::greyLevel (0.8f));
}

void ProjectContentComponent::setProject (Project* newProject)
{
    if (project != newProject)
    {
        if (project != nullptr)
            project->removeChangeListener (this);

        contentView = nullptr;
        resizerBar = nullptr;

        if (projectTree != nullptr)
        {
            StoredSettings::getInstance()->getProps().setValue ("projectTreeviewWidth", projectTree->getWidth());
            projectTree->deleteRootItem();
            projectTree = nullptr;
        }

        project = newProject;

        if (project != nullptr)
        {
            addAndMakeVisible (projectTree = new TreeView());
            projectTree->setComponentID ("tree");
            projectTree->setRootItemVisible (true);
            projectTree->setMultiSelectEnabled (true);
            projectTree->setDefaultOpenness (true);
            projectTree->setColour (TreeView::backgroundColourId, Colour::greyLevel (0.93f));
            projectTree->setIndentSize (14);

            projectTree->setRootItem (new GroupTreeViewItem (project->getMainGroup()));
            projectTree->getRootItem()->setOpen (true);

            String lastTreeWidth (StoredSettings::getInstance()->getProps().getValue ("projectTreeviewWidth"));
            if (lastTreeWidth.getIntValue() < 150)
                lastTreeWidth = "250";

            projectTree->setBounds ("0, 0, left + " + lastTreeWidth + ", parent.height");

            addAndMakeVisible (resizerBar = new ResizableEdgeComponent (projectTree, &treeSizeConstrainer,
                                                                        ResizableEdgeComponent::rightEdge));
            resizerBar->setComponentID ("resizer");
            resizerBar->setBounds ("tree.right, 0, tree.right + 4, parent.height");

            project->addChangeListener (this);

            if (currentDocument == nullptr)
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
    if (projectTree != nullptr)
    {
        ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (projectTree->getRootItem());
        if (p != nullptr)
            p->checkFileStatus();
    }
}

bool ProjectContentComponent::showEditorForFile (const File& f)
{
    return showDocument (OpenDocumentManager::getInstance()->openFile (project, f));
}

bool ProjectContentComponent::showDocument (OpenDocumentManager::Document* doc)
{
    if (doc == nullptr)
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
        currentDocument = nullptr;
        contentView = nullptr;
        updateMainWindowTitle();
        commandManager->commandStatusChanged();
    }
}

bool ProjectContentComponent::setEditorComponent (Component* editor, OpenDocumentManager::Document* doc)
{
    if (editor != nullptr)
    {
        contentView = editor;
        currentDocument = doc;
        addAndMakeVisible (editor);
        editor->setBounds ("resizer.right, 0, parent.right, parent.height");

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

    if (mw != nullptr)
        mw->updateTitle (currentDocument != nullptr ? currentDocument->getName() : String::empty);
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
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::saveProjectAs:
        result.setInfo ("Save Project As...",
                        "Saves the current project to a different filename",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::closeProject:
        result.setInfo ("Close Project",
                        "Closes the current project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
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
        result.setActive (project != nullptr);
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
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('l', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showProjectSettings:
        result.setInfo ("Show Project Build Settings",
                        "Shows the build options for the project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('i', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case StandardApplicationCommandIDs::del:
        result.setInfo ("Delete", String::empty, CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (KeyPress::deleteKey, 0, 0));
        result.defaultKeypresses.add (KeyPress (KeyPress::backspaceKey, 0, 0));
        result.setActive (projectTree != nullptr);
        break;

    default:
        break;
    }
}

bool ProjectContentComponent::isCommandActive (const CommandID commandID)
{
    return project != nullptr;
}

bool ProjectContentComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::saveProject:
        if (project != nullptr)
            project->save (true, true);

        break;

    case CommandIDs::saveProjectAs:
        if (project != nullptr)
            project->saveAsInteractive (true);

        break;

    case CommandIDs::closeProject:
        {
            MainWindow* mw = Component::findParentComponentOfClass ((MainWindow*) 0);

            if (mw != nullptr)
                mw->closeCurrentProject();
        }

        break;

    case CommandIDs::openInIDE:
        if (project != nullptr)
        {
            ScopedPointer <ProjectExporter> exporter (ProjectExporter::createPlatformDefaultExporter (*project));

            if (exporter != nullptr)
                exporter->launchProject();
        }
        break;

    case CommandIDs::saveAndOpenInIDE:
        if (project != nullptr && project->save (true, true) == FileBasedDocument::savedOk)
        {
            ScopedPointer <ProjectExporter> exporter (ProjectExporter::createPlatformDefaultExporter (*project));

            if (exporter != nullptr)
                exporter->launchProject();
        }
        break;

    case CommandIDs::showProjectSettings:
        if (projectTree != nullptr)
            projectTree->getRootItem()->setSelected (true, true);

        break;

    case StandardApplicationCommandIDs::del:
        if (projectTree != nullptr)
        {
            ProjectTreeViewBase* p = dynamic_cast <ProjectTreeViewBase*> (projectTree->getRootItem());
            if (p != nullptr)
                p->deleteAllSelectedItems();
        }

        break;

    default:
        return false;
    }

    return true;
}
