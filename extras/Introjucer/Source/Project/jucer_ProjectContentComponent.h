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

#ifndef JUCER_PROJECTCONTENTCOMPONENT_H_INCLUDED
#define JUCER_PROJECTCONTENTCOMPONENT_H_INCLUDED

#include "jucer_Project.h"
#include "../Application/jucer_OpenDocumentManager.h"


//==============================================================================
/**
*/
class ProjectContentComponent  : public Component,
                                 public ApplicationCommandTarget,
                                 private ChangeListener,
                                 private OpenDocumentManager::DocumentCloseListener
{
public:
    //==============================================================================
    ProjectContentComponent();
    ~ProjectContentComponent();

    Project* getProject() const noexcept    { return project; }
    virtual void setProject (Project* project);

    void saveTreeViewState();
    void saveOpenDocumentList();
    void reloadLastOpenDocuments();

    bool showEditorForFile (const File& f, bool grabFocus);
    File getCurrentFile() const;

    bool showDocument (OpenDocumentManager::Document* doc, bool grabFocus);
    void hideDocument (OpenDocumentManager::Document* doc);
    OpenDocumentManager::Document* getCurrentDocument() const   { return currentDocument; }
    void closeDocument();
    void saveDocument();
    void saveAs();

    void hideEditor();
    bool setEditorComponent (Component* editor, OpenDocumentManager::Document* doc);
    Component* getEditorComponent() const                       { return contentView; }

    bool goToPreviousFile();
    bool goToNextFile();
    bool canGoToCounterpart() const;
    bool goToCounterpart();

    bool saveProject();
    void closeProject();
    void openInIDE (bool saveFirst);
    void openInIDE (int exporterIndex, bool saveFirst);

    void showFilesTab();
    void showConfigTab();
    void showProjectSettings();
    void showModules();
    void showModule (const String& moduleID);

    void deleteSelectedTreeItems();

    void updateMainWindowTitle();

    void updateMissingFileStatuses();
    virtual void createProjectTabs();
    virtual void deleteProjectTabs();
    void rebuildProjectTabs();

    void showBubbleMessage (const Rectangle<int>&, const String&);

    StringArray getExportersWhichCanLaunch() const;

    static void getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails& dragSourceDetails,
                                                     OwnedArray<Project::Item>& selectedNodes);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array <CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;

    void paint (Graphics&) override;
    void paintOverChildren (Graphics&) override;
    void resized() override;
    void childBoundsChanged (Component*) override;
    void lookAndFeelChanged() override;

protected:
    Project* project;
    OpenDocumentManager::Document* currentDocument;
    RecentDocumentList recentDocumentList;
    ScopedPointer<Component> logo;
    ScopedPointer<Component> translationTool;

    TabbedComponent treeViewTabs;
    ScopedPointer<ResizableEdgeComponent> resizerBar;
    ScopedPointer<Component> contentView;

    ComponentBoundsConstrainer treeSizeConstrainer;
    BubbleMessageComponent bubbleMessage;

    bool documentAboutToClose (OpenDocumentManager::Document*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void showTranslationTool();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent)
};


#endif   // JUCER_PROJECTCONTENTCOMPONENT_H_INCLUDED
