/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Project/UI/jucer_ProjectContentComponent.h"
#include "../Utility/PIPs/jucer_PIPGenerator.h"

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

    //==============================================================================
    void closeButtonPressed() override;

    //==============================================================================
    bool canOpenFile (const File& file) const;
    bool openFile (const File& file);
    void setProject (Project* newProject);
    Project* getProject() const                 { return currentProject.get(); }
    bool tryToOpenPIP (const File& f);

    void makeVisible();
    void restoreWindowPosition();
    bool closeProject (Project* project, bool askToSave = true);
    bool closeCurrentProject();
    void moveProject (File newProjectFile);

    void showStartPage();

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
    std::unique_ptr<Project> currentProject;
    Value projectNameValue;

    static const char* getProjectWindowPosName()   { return "projectWindowPos"; }
    void createProjectContentCompIfNeeded();
    void setTitleBarIcon();

    void openPIP (PIPGenerator&);

    void valueChanged (Value&) override;

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

    Project* getFrontmostProject();

    void reopenLastProjects();
    void saveCurrentlyOpenProjectList();

    void avoidSuperimposedWindows (MainWindow*);

    void sendLookAndFeelChange();

    OwnedArray<MainWindow> windows;

private:
    bool isInReopenLastProjects = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindowList)
};
