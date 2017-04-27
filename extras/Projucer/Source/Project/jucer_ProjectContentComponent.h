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

#pragma once

#include "jucer_Project.h"
#include "../Application/jucer_OpenDocumentManager.h"

class CompileEngineChildProcess;
class ProjectTab;
class LiveBuildTab;
class HeaderComponent;

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
    ~ProjectContentComponent();

    Project* getProject() const noexcept    { return project; }
    virtual void setProject (Project*);

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
    Component* getEditorComponent() const    { return contentView; }
    Component& getSidebarComponent()         { return sidebarTabs; }

    bool goToPreviousFile();
    bool goToNextFile();
    bool canGoToCounterpart() const;
    bool goToCounterpart();

    bool saveProject();
    void closeProject();
    void openInSelectedIDE (bool saveFirst);
    void showNewExporterMenu();

    void showProjectTab()    { sidebarTabs.setCurrentTabIndex (0); }
    void showBuildTab()      { sidebarTabs.setCurrentTabIndex (1); }

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

    void updateMissingFileStatuses();
    void createProjectTabs();
    void deleteProjectTabs();
    void rebuildProjectTabs();
    void refreshTabsIfBuildStatusChanged();
    void toggleWarnings();
    void showNextError();
    void showPreviousError();
    void reinstantiateLivePreviewWindows();

    void showBubbleMessage (Rectangle<int>, const String&);

    StringArray getExportersWhichCanLaunch() const;

    static void getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails&,
                                                     OwnedArray<Project::Item>& selectedNodes);

    //==============================================================================
    void killChildProcess();
    void cleanAll();
    void handleMissingSystemHeaders();
    bool isBuildTabEnabled() const;
    void setBuildEnabled (bool);
    bool isBuildEnabled() const;
    bool areWarningsEnabled() const;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    void paint (Graphics&) override;
    void resized() override;
    void childBoundsChanged (Component*) override;
    void lookAndFeelChanged() override;

    String lastCrashMessage;

private:
    friend HeaderComponent;

    //==============================================================================
    Project* project;
    OpenDocumentManager::Document* currentDocument;
    RecentDocumentList recentDocumentList;
    ScopedPointer<Component> logo, translationTool, contentView, header;

    TabbedComponent sidebarTabs;
    ScopedPointer<ResizableEdgeComponent> resizerBar;
    ComponentBoundsConstrainer sidebarSizeConstrainer;

    BubbleMessageComponent bubbleMessage;
    ReferenceCountedObjectPtr<CompileEngineChildProcess> childProcess;
    bool isForeground = false;

    ScopedPointer<Label> fileNameLabel;

    int lastViewedTab = 0;

    //==============================================================================
    bool documentAboutToClose (OpenDocumentManager::Document*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void showTranslationTool();

    void globalFocusChanged (Component*) override;
    void timerCallback() override;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent)
};
