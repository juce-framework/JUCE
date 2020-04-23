/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Utility/PIPs/jucer_PIPGenerator.h"
#include "../Project/jucer_Project.h"
#include "../CodeEditor/jucer_OpenDocumentManager.h"

class ProjectContentComponent;

//==============================================================================
/**
    The big top-level window where everything happens.
*/
class MainWindow  : public DocumentWindow,
                    public ApplicationCommandTarget,
                    public FileDragAndDropTarget,
                    public DragAndDropContainer,
                    private Value::Listener
{
public:
    //==============================================================================
    MainWindow();
    ~MainWindow() override;

    enum class OpenInIDE { no, yes };

    //==============================================================================
    void closeButtonPressed() override;

    //==============================================================================
    bool canOpenFile (const File& file) const;
    bool openFile (const File& file);

    void setProject (std::unique_ptr<Project> newProject);
    Project* getProject() const  { return currentProject.get(); }

    bool tryToOpenPIP (const File& f);

    void makeVisible();
    void restoreWindowPosition();
    bool closeCurrentProject (OpenDocumentManager::SaveIfNeeded askToSave);
    void moveProject (File newProjectFile, OpenInIDE openInIDE);

    void showStartPage();

    void showLoginFormOverlay();
    void hideLoginFormOverlay();
    bool isShowingLoginForm() const noexcept  { return loginFormOpen; }

    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& filenames, int mouseX, int mouseY) override;

    void activeWindowStatusChanged() override;

    ProjectContentComponent* getProjectContentComponent() const;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array <CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;

    bool shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                               StringArray& files, bool& canMoveFiles) override;
private:
    void valueChanged (Value&) override;

    static const char* getProjectWindowPosName()   { return "projectWindowPos"; }
    void createProjectContentCompIfNeeded();
    void setTitleBarIcon();
    void openPIP (PIPGenerator&);

    std::unique_ptr<Project> currentProject;
    Value projectNameValue;

    std::unique_ptr<Component> blurOverlayComponent;
    bool loginFormOpen = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

//==============================================================================
class MainWindowList
{
public:
    MainWindowList();

    void forceCloseAllWindows();
    bool askAllWindowsToClose();
    void closeWindow (MainWindow*);

    void goToSiblingWindow (MainWindow*, int delta);

    void createWindowIfNoneAreOpen();
    void openDocument (OpenDocumentManager::Document*, bool grabFocus);
    bool openFile (const File& file, bool openInBackground = false);

    MainWindow* createNewMainWindow();
    MainWindow* getFrontmostWindow (bool createIfNotFound = true);
    MainWindow* getOrCreateEmptyWindow();
    MainWindow* getMainWindowForFile (const File&);
    MainWindow* getMainWindowWithLoginFormOpen();

    Project* getFrontmostProject();

    void reopenLastProjects();
    void saveCurrentlyOpenProjectList();

    void checkWindowBounds (MainWindow&);

    void sendLookAndFeelChange();

    OwnedArray<MainWindow> windows;

private:
    bool isInReopenLastProjects = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindowList)
};
