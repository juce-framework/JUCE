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

class CompileEngineChildProcess;


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
    OpenDocumentManager::Document* getCurrentDocument() const   { return currentDocument; }
    void closeDocument();
    void saveDocument();
    void saveAs();

    void hideEditor();
    bool setEditorComponent (Component* editor, OpenDocumentManager::Document* doc);
    Component* getEditorComponent() const                       { return contentView; }
    Component& getTabsComponent()                               { return treeViewTabs; }

    bool goToPreviousFile();
    bool goToNextFile();
    bool canGoToCounterpart() const;
    bool goToCounterpart();

    bool saveProject();
    void closeProject();
    void openInIDE (bool saveFirst);
    void openInIDE (int exporterIndex, bool saveFirst);
    void showNewExporterMenu();

    void showFilesTab();
    void showConfigTab();
    void showBuildTab();
    void showProjectSettings();
    void showModules();
    void showModule (const String& moduleID);

    void deleteSelectedTreeItems();

    void updateMainWindowTitle();

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
    void paintOverChildren (Graphics&) override;
    void resized() override;
    void childBoundsChanged (Component*) override;
    void lookAndFeelChanged() override;

    String lastCrashMessage;

private:
    //==============================================================================
    Project* project;
    OpenDocumentManager::Document* currentDocument;
    RecentDocumentList recentDocumentList;
    ScopedPointer<Component> logo, translationTool, contentView;

    TabbedComponent treeViewTabs;
    ScopedPointer<ResizableEdgeComponent> resizerBar;

    ComponentBoundsConstrainer treeSizeConstrainer;
    BubbleMessageComponent bubbleMessage;

    ReferenceCountedObjectPtr<CompileEngineChildProcess> childProcess;
    bool isForeground = false;

    //==============================================================================
    bool documentAboutToClose (OpenDocumentManager::Document*) override;
    void changeListenerCallback (ChangeBroadcaster*) override;
    void showTranslationTool();

    void globalFocusChanged (Component*) override;
    void timerCallback() override;

    Component* createBuildTab (CompileEngineChildProcess*);
    Component* createDisabledBuildTabSubscribe (String textPrefix, bool loggedIn, bool dllPresent);
    Component* createDisabledBuildTabInfoOnly (const char* messsage);

    bool isContinuousRebuildEnabled()           { return getAppSettings().getGlobalProperties().getBoolValue ("continuousRebuild", true);  }
    void setContinuousRebuildEnabled (bool b)   { getAppSettings().getGlobalProperties().setValue ("continuousRebuild", b); }
    void rebuildNow();
    void handleCrash (const String& message);
    void updateWarningState();
    void launchApp();
    void killApp();
    bool isBuildTabSuitableForLoggedInUser() const;
    bool isBuildTabLoggedInWithoutLicense() const;

    ReferenceCountedObjectPtr<CompileEngineChildProcess> getChildProcess();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectContentComponent)
};


#endif   // JUCER_PROJECTCONTENTCOMPONENT_H_INCLUDED
