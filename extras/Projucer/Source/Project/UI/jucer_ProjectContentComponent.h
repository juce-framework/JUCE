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

#include "../../CodeEditor/jucer_OpenDocumentManager.h"
#include "jucer_HeaderComponent.h"
#include "jucer_ProjectMessagesComponent.h"

class CompileEngineChildProcess;
class ProjectTab;
class LiveBuildTab;

//==============================================================================
class ProjectContentComponent  : public Component,
                                 public ApplicationCommandTarget,
                                 private ChangeListener,
                                 private OpenDocumentManager::DocumentCloseListener,
                                 private FocusChangeListener,
                                 private Timer
{
public:
    //==============================================================================
    ProjectContentComponent();
    ~ProjectContentComponent() override;

    Project* getProject() const noexcept    { return project; }
    void setProject (Project*);

    void saveTreeViewState();
    void saveOpenDocumentList();
    void reloadLastOpenDocuments();

    bool showEditorForFile (const File&, bool grabFocus);
    bool hasFileInRecentList (const File&) const;
    File getCurrentFile() const;

    bool showDocument (OpenDocumentManager::Document*, bool grabFocus);
    void hideDocument (OpenDocumentManager::Document*);
    OpenDocumentManager::Document* getCurrentDocument() const    { return currentDocument; }
    void closeDocument();
    void saveDocument();
    void saveAs();

    void hideEditor();
    bool setEditorComponent (Component* editor, OpenDocumentManager::Document* doc);
    Component* getEditorComponentContent() const;
    Component* getEditorComponent() const    { return contentView.get(); }
    Component& getSidebarComponent()         { return sidebarTabs; }

    bool goToPreviousFile();
    bool goToNextFile();
    bool canGoToCounterpart() const;
    bool goToCounterpart();

    bool saveProject();
    void closeProject();
    void openInSelectedIDE (bool saveFirst);
    void showNewExporterMenu();

    void showProjectTab()        { sidebarTabs.setCurrentTabIndex (0); }
    void showBuildTab()          { sidebarTabs.setCurrentTabIndex (1); }
    int getCurrentTabIndex()     { return sidebarTabs.getCurrentTabIndex(); }

    void showFilesPanel()        { showProjectPanel (0); }
    void showModulesPanel()      { showProjectPanel (1); }
    void showExportersPanel()    { showProjectPanel (2); }

    void showProjectSettings();
    void showCurrentExporterSettings();
    void showExporterSettings (const String& exporterName);
    void showModule (const String& moduleID);
    void showLiveBuildSettings();
    void showUserSettings();

    void deleteSelectedTreeItems();

    void refreshProjectTreeFileStatuses();
    void updateMissingFileStatuses();
    void createProjectTabs();
    void deleteProjectTabs();
    void rebuildProjectUI();
    void refreshTabsIfBuildStatusChanged();
    void toggleWarnings();
    void showNextError();
    void showPreviousError();
    void reinstantiateLivePreviewWindows();
    void addNewGUIFile();

    void showBubbleMessage (Rectangle<int>, const String&);

    StringArray getExportersWhichCanLaunch() const;

    static void getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails&,
                                                     OwnedArray<Project::Item>& selectedNodes);

    //==============================================================================
    void killChildProcess();
    void cleanAll();
    void handleMissingSystemHeaders();
    bool isBuildTabEnabled() const;
    void setBuildEnabled (bool enabled, bool displayError = false);
    bool isBuildEnabled() const;
    bool areWarningsEnabled() const;

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
    static String getBuildTabName()      { return "Build"; }

private:
    //==============================================================================
    struct LogoComponent  : public Component
    {
        LogoComponent();
        void paint (Graphics& g) override;
        static String getVersionInfo();

        std::unique_ptr<Drawable> logo;
    };

    struct ContentViewport  : public Component
    {
        ContentViewport (Component* content);
        void resized() override;

        Viewport viewport;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ContentViewport)
    };

    //==============================================================================
    bool documentAboutToClose (OpenDocumentManager::Document*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void showTranslationTool();

    void globalFocusChanged (Component*) override;
    void timerCallback() override;

    void liveBuildEnablementChanged (bool isEnabled);

    bool isContinuousRebuildEnabled();
    void setContinuousRebuildEnabled (bool b);

    void rebuildNow();
    void handleCrash (const String& message);
    void updateWarningState();
    void launchApp();
    void killApp();

    ReferenceCountedObjectPtr<CompileEngineChildProcess> getChildProcess();

    //==============================================================================
    void showProjectPanel (const int index);
    ProjectTab* getProjectTab();
    LiveBuildTab* getLiveBuildTab();
    bool canSelectedProjectBeLaunch();

    //==============================================================================
    Project* project = nullptr;
    OpenDocumentManager::Document* currentDocument = nullptr;
    RecentDocumentList recentDocumentList;

    LogoComponent logoComponent;
    HeaderComponent headerComponent { this };
    ProjectMessagesComponent projectMessagesComponent;
    Label fileNameLabel;
    TabbedComponent sidebarTabs  { TabbedButtonBar::TabsAtTop };
    std::unique_ptr<ResizableEdgeComponent> resizerBar;
    ComponentBoundsConstrainer sidebarSizeConstrainer;
    std::unique_ptr<Component> translationTool, contentView;
    BubbleMessageComponent bubbleMessage;

    ReferenceCountedObjectPtr<CompileEngineChildProcess> childProcess;
    String lastCrashMessage;

    bool isForeground = false, isLiveBuildEnabled = false;
    int lastViewedTab = 0;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent)
};
