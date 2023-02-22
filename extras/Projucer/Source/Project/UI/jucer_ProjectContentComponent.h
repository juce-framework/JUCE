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

#include "../../CodeEditor/jucer_OpenDocumentManager.h"
#include "jucer_HeaderComponent.h"
#include "jucer_ProjectMessagesComponent.h"
#include "jucer_ContentViewComponent.h"

class Sidebar;
struct WizardHolder;

//==============================================================================
class ProjectContentComponent  : public Component,
                                 public ApplicationCommandTarget,
                                 private ChangeListener,
                                 private OpenDocumentManager::DocumentCloseListener
{
public:
    //==============================================================================
    ProjectContentComponent();
    ~ProjectContentComponent() override;

    Project* getProject() const noexcept    { return project; }
    void setProject (Project*);

    void saveOpenDocumentList();
    void reloadLastOpenDocuments();

    bool showEditorForFile (const File&, bool grabFocus);
    bool hasFileInRecentList (const File&) const;
    File getCurrentFile() const;

    bool showDocument (OpenDocumentManager::Document*, bool grabFocus);
    void hideDocument (OpenDocumentManager::Document*);
    OpenDocumentManager::Document* getCurrentDocument() const    { return currentDocument; }
    void closeDocument();
    void saveDocumentAsync();
    void saveAsAsync();

    void hideEditor();
    void setScrollableEditorComponent (std::unique_ptr<Component> component);
    void setEditorDocument (std::unique_ptr<Component> component, OpenDocumentManager::Document* doc);
    Component* getEditorComponent();

    Component& getSidebarComponent();

    bool goToPreviousFile();
    bool goToNextFile();
    bool canGoToCounterpart() const;
    bool goToCounterpart();

    void saveProjectAsync();
    void closeProject();
    void openInSelectedIDE (bool saveFirst);
    void showNewExporterMenu();

    void showFilesPanel()        { showProjectPanel (0); }
    void showModulesPanel()      { showProjectPanel (1); }
    void showExportersPanel()    { showProjectPanel (2); }

    void showProjectSettings();
    void showCurrentExporterSettings();
    void showExporterSettings (const String& exporterName);
    void showModule (const String& moduleID);
    void showUserSettings();

    void deleteSelectedTreeItems();

    void refreshProjectTreeFileStatuses();
    void updateMissingFileStatuses();
    void addNewGUIFile();

    void showBubbleMessage (Rectangle<int>, const String&);

    StringArray getExportersWhichCanLaunch() const;

    static void getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails&,
                                                     OwnedArray<Project::Item>& selectedNodes);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    bool isSaveCommand (const CommandID id);

    void paint (Graphics&) override;
    void resized() override;
    void childBoundsChanged (Component*) override;
    void lookAndFeelChanged() override;

    ProjectMessagesComponent& getProjectMessagesComponent()  { return projectMessagesComponent; }

    static String getProjectTabName()    { return "Project"; }

private:
    //==============================================================================
    bool documentAboutToClose (OpenDocumentManager::Document*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void showTranslationTool();

    //==============================================================================
    void showProjectPanel (const int index);
    bool canSelectedProjectBeLaunch();

    //==============================================================================
    Project* project = nullptr;
    OpenDocumentManager::Document* currentDocument = nullptr;
    RecentDocumentList recentDocumentList;

    HeaderComponent headerComponent { this };
    std::unique_ptr<Sidebar> sidebar;
    ProjectMessagesComponent projectMessagesComponent;
    ContentViewComponent contentViewComponent;

    std::unique_ptr<ResizableEdgeComponent> resizerBar;
    ComponentBoundsConstrainer sidebarSizeConstrainer;
    std::unique_ptr<Component> translationTool;
    BubbleMessageComponent bubbleMessage;

    bool isForeground = false;
    int lastViewedTab = 0;

    std::unique_ptr<WizardHolder> wizardHolder;
    ScopedMessageBox messageBox;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent)
};
