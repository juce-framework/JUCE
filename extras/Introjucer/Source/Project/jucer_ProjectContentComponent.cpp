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
class TreePanelBase   : public Component
{
public:
    TreePanelBase (const String& opennessStateKey_)
        : opennessStateKey (opennessStateKey_)
    {
        addAndMakeVisible (&tree);
        tree.setRootItemVisible (true);
        tree.setDefaultOpenness (true);
        tree.setColour (TreeView::backgroundColourId, Colours::transparentBlack);
        tree.setIndentSize (14);
    }

    ~TreePanelBase()
    {
        tree.setRootItem (nullptr);
    }

    void setRoot (JucerTreeViewBase* root)
    {
        rootItem = root;
        tree.setRootItem (root);
        tree.getRootItem()->setOpen (true);

        const ScopedPointer<XmlElement> treeOpenness (StoredSettings::getInstance()->getProps()
                                                        .getXmlValue (opennessStateKey));
        if (treeOpenness != nullptr)
            tree.restoreOpennessState (*treeOpenness, true);
    }

    void saveOpenness()
    {
        const ScopedPointer<XmlElement> opennessState (tree.getOpennessState (true));

        if (opennessState != nullptr)
            StoredSettings::getInstance()->getProps().setValue (opennessStateKey, opennessState);
    }

    void deleteSelectedItems()
    {
        if (rootItem != nullptr)
            rootItem->deleteAllSelectedItems();
    }

    void resized()
    {
        tree.setBounds (getLocalBounds());
    }

    TreeView tree;
    ScopedPointer<JucerTreeViewBase> rootItem;

private:
    String opennessStateKey;
};

//==============================================================================
class FileTreeTab   : public TreePanelBase
{
public:
    FileTreeTab (Project& project)
        : TreePanelBase ("treeViewState_" + project.getProjectUID())
    {
        tree.setMultiSelectEnabled (true);
        setRoot (new GroupTreeViewItem (project.getMainGroup()));
    }
};

//==============================================================================
class ConfigTreeTab   : public TreePanelBase
{
public:
    ConfigTreeTab (Project& project)
        : TreePanelBase ("settingsTreeViewState_" + project.getProjectUID())
    {
        tree.setMultiSelectEnabled (false);
        setRoot (createProjectConfigTreeViewRoot (project));

        if (tree.getNumSelectedItems() == 0)
            tree.getRootItem()->setSelected (true, true);

       #if JUCE_MAC || JUCE_WINDOWS
        addAndMakeVisible (&openProjectButton);
        openProjectButton.setCommandToTrigger (commandManager, CommandIDs::openInIDE, true);
        openProjectButton.setButtonText (commandManager->getNameOfCommand (CommandIDs::openInIDE));

        addAndMakeVisible (&saveAndOpenButton);
        saveAndOpenButton.setCommandToTrigger (commandManager, CommandIDs::saveAndOpenInIDE, true);
        saveAndOpenButton.setButtonText (commandManager->getNameOfCommand (CommandIDs::saveAndOpenInIDE));
       #endif
    }

    void resized()
    {
        Rectangle<int> r (getLocalBounds());
        r.removeFromBottom (6);

        if (saveAndOpenButton.isVisible())
            saveAndOpenButton.setBounds (r.removeFromBottom (28).reduced (20, 3));

        if (openProjectButton.isVisible())
            openProjectButton.setBounds (r.removeFromBottom (28).reduced (20, 3));

        tree.setBounds (r);
    }

    TextButton openProjectButton, saveAndOpenButton;
};

//==============================================================================
ProjectContentComponent::ProjectContentComponent()
    : project (nullptr),
      currentDocument (nullptr),
      treeViewTabs (TabbedButtonBar::TabsAtTop)
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
    jassert (getNumChildComponents() <= 1);
}

void ProjectContentComponent::paint (Graphics& g)
{
    g.fillAll (Colour::greyLevel (0.8f));
}

void ProjectContentComponent::setProject (Project* newProject)
{
    if (project != newProject)
    {
        PropertiesFile& settings = StoredSettings::getInstance()->getProps();

        if (project != nullptr)
            project->removeChangeListener (this);

        contentView = nullptr;
        resizerBar = nullptr;

        treeViewTabs.clearTabs();

        if (treeViewTabs.isShowing() && treeViewTabs.getWidth() > 0)
            settings.setValue ("projectTreeviewWidth", treeViewTabs.getWidth());

        project = newProject;

        if (project != nullptr)
        {
            treeViewTabs.setVisible (true);
            addChildAndSetID (&treeViewTabs, "tree");

            createProjectTabs();

            String lastTreeWidth (settings.getValue ("projectTreeviewWidth"));
            if (lastTreeWidth.getIntValue() < 150)
                lastTreeWidth = "250";

            treeViewTabs.setBounds ("0, 0, left + " + lastTreeWidth + ", parent.height");

            addChildAndSetID (resizerBar = new ResizableEdgeComponent (&treeViewTabs, &treeSizeConstrainer,
                                                                       ResizableEdgeComponent::rightEdge),
                              "resizer");

            resizerBar->setBounds ("tree.right, 0, tree.right + 4, parent.height");

            project->addChangeListener (this);

            if (currentDocument == nullptr)
                invokeDirectly (CommandIDs::showProjectSettings, true);

            updateMissingFileStatuses();
        }
        else
        {
            treeViewTabs.setVisible (false);
        }
    }
}

void ProjectContentComponent::createProjectTabs()
{
    treeViewTabs.addTab ("Files",  Colour::greyLevel (0.93f), new FileTreeTab (*project), true);
    treeViewTabs.addTab ("Config", Colour::greyLevel (0.93f), new ConfigTreeTab (*project), true);
}

TreeView* ProjectContentComponent::getFilesTreeView() const
{
    FileTreeTab* ft = dynamic_cast<FileTreeTab*> (treeViewTabs.getTabContentComponent (0));
    return ft != nullptr ? &(ft->tree) : nullptr;
}

ProjectTreeViewBase* ProjectContentComponent::getFilesTreeRoot() const
{
    TreeView* tv = getFilesTreeView();
    return tv != nullptr ? dynamic_cast <ProjectTreeViewBase*> (tv->getRootItem()) : nullptr;
}

void ProjectContentComponent::saveTreeViewState()
{
    for (int i = treeViewTabs.getNumTabs(); --i >= 0;)
    {
        TreePanelBase* t = dynamic_cast<TreePanelBase*> (treeViewTabs.getTabContentComponent (i));

        if (t != nullptr)
            t->saveOpenness();
    }
}

void ProjectContentComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateMissingFileStatuses();
}

void ProjectContentComponent::updateMissingFileStatuses()
{
    ProjectTreeViewBase* p = getFilesTreeRoot();

    if (p != nullptr)
        p->checkFileStatus();
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

void ProjectContentComponent::hideEditor()
{
    currentDocument = nullptr;
    contentView = nullptr;
    updateMainWindowTitle();
    commandManager->commandStatusChanged();
}

void ProjectContentComponent::hideDocument (OpenDocumentManager::Document* doc)
{
    if (doc == currentDocument)
        hideEditor();
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
    MainWindow* mw = findParentComponentOfClass<MainWindow>();

    if (mw != nullptr)
        mw->updateTitle (currentDocument != nullptr ? currentDocument->getName() : String::empty);
}

bool ProjectContentComponent::canProjectBeLaunched() const
{
    if (project != nullptr)
    {
        ScopedPointer <ProjectExporter> launcher (ProjectExporter::createPlatformDefaultExporter (*project));
        return launcher != nullptr;
    }

    return false;
}

ApplicationCommandTarget* ProjectContentComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void ProjectContentComponent::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::saveDocument,
                              CommandIDs::closeDocument,
                              CommandIDs::saveProject,
                              CommandIDs::closeProject,
                              CommandIDs::openInIDE,
                              CommandIDs::saveAndOpenInIDE,
                              CommandIDs::showProjectSettings,
                              StandardApplicationCommandIDs::del };

    commands.addArray (ids, numElementsInArray (ids));
}

void ProjectContentComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    String documentName;
    if (currentDocument != nullptr)
        documentName = " '" + currentDocument->getName().substring (0, 32) + "'";

    switch (commandID)
    {
    case CommandIDs::saveProject:
        result.setInfo ("Save Project",
                        "Saves the current project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        break;

    case CommandIDs::closeProject:
        result.setInfo ("Close Project",
                        "Closes the current project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        break;

    case CommandIDs::saveDocument:
        result.setInfo ("Save" + documentName,
                        "Saves the current document",
                        CommandCategories::general, 0);
        result.setActive (currentDocument != nullptr || project != nullptr);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::closeDocument:
        result.setInfo ("Close" + documentName,
                        "Closes the current document",
                        CommandCategories::general, 0);
        result.setActive (currentDocument != nullptr);
       #if JUCE_MAC
        result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier | ModifierKeys::ctrlModifier, 0));
       #else
        result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
       #endif
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
        result.setActive (canProjectBeLaunched());
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
        result.setActive (canProjectBeLaunched());
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
        result.setActive (dynamic_cast<TreePanelBase*> (treeViewTabs.getCurrentContentComponent()) != nullptr);
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
        if (project != nullptr && ! reinvokeCommandAfterClosingPropertyEditors (info))
            project->save (true, true);

        break;

    case CommandIDs::closeProject:
        {
            MainWindow* const mw = findParentComponentOfClass<MainWindow>();

            if (mw != nullptr && ! reinvokeCommandAfterClosingPropertyEditors (info))
                mw->closeCurrentProject();
        }

        break;

    case CommandIDs::saveDocument:
        if (! reinvokeCommandAfterClosingPropertyEditors (info))
        {
            if (currentDocument != nullptr)
                currentDocument->save();
            else if (project != nullptr)
                project->save (true, true);
        }

        break;

    case CommandIDs::closeDocument:
        if (currentDocument != nullptr)
            OpenDocumentManager::getInstance()->closeDocument (currentDocument, true);
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
        if (project != nullptr)
        {
            if (! reinvokeCommandAfterClosingPropertyEditors (info))
            {
                if (project->save (true, true) == FileBasedDocument::savedOk)
                {
                    ScopedPointer <ProjectExporter> exporter (ProjectExporter::createPlatformDefaultExporter (*project));

                    if (exporter != nullptr)
                        exporter->launchProject();
                }
            }
        }
        break;

    case CommandIDs::showProjectSettings:
        treeViewTabs.setCurrentTabIndex (1);
        break;

    case StandardApplicationCommandIDs::del:
        {
            TreePanelBase* const tree = dynamic_cast<TreePanelBase*> (treeViewTabs.getCurrentContentComponent());

            if (tree != nullptr)
                tree->deleteSelectedItems();
        }

        break;

    default:
        return false;
    }

    return true;
}

bool ProjectContentComponent::reinvokeCommandAfterClosingPropertyEditors (const InvocationInfo& info)
{
    if (reinvokeCommandAfterCancellingModalComps (info))
    {
        grabKeyboardFocus(); // to force any open labels to close their text editors
        return true;
    }

    return false;
}
