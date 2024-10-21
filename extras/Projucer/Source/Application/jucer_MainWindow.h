/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
class MainWindow final : public DocumentWindow,
                         public ApplicationCommandTarget,
                         public FileDragAndDropTarget,
                         public DragAndDropContainer,
                         private Value::Listener,
                         private ChangeListener
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
    void openFile (const File& file, std::function<void (bool)> callback);

    void setProject (std::unique_ptr<Project> newProject);
    Project* getProject() const  { return currentProject.get(); }

    void makeVisible();
    void restoreWindowPosition();
    void updateTitleBarIcon();
    void closeCurrentProject (OpenDocumentManager::SaveIfNeeded askToSave, std::function<void (bool)> callback);
    void moveProject (File newProjectFile, OpenInIDE openInIDE);

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
    void valueChanged (Value&) override;
    void changeListenerCallback (ChangeBroadcaster* source) override;

    static const char* getProjectWindowPosName()   { return "projectWindowPos"; }
    void createProjectContentCompIfNeeded();

    void openPIP (const File&, std::function<void (bool)> callback);
    void setupTemporaryPIPProject (PIPGenerator&);

    void initialiseProjectWindow();

    std::unique_ptr<Project> currentProject;
    Value projectNameValue;

    std::unique_ptr<Component> blurOverlayComponent;

    ScopedMessageBox messageBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
};

//==============================================================================
class MainWindowList
{
public:
    MainWindowList();

    void forceCloseAllWindows();
    void askAllWindowsToClose (std::function<void (bool)> callback);
    void closeWindow (MainWindow*);

    void goToSiblingWindow (MainWindow*, int delta);

    void createWindowIfNoneAreOpen();
    void openDocument (OpenDocumentManager::Document*, bool grabFocus);
    void openFile (const File& file, std::function<void (bool)> callback, bool openInBackground = false);

    MainWindow* createNewMainWindow();
    MainWindow* getFrontmostWindow (bool createIfNotFound = true);
    MainWindow* getOrCreateEmptyWindow();
    MainWindow* getMainWindowForFile (const File&);

    Project* getFrontmostProject();

    void reopenLastProjects();
    void saveCurrentlyOpenProjectList();

    void checkWindowBounds (MainWindow&);

    void sendLookAndFeelChange();

    OwnedArray<MainWindow> windows;

private:
    bool isInReopenLastProjects = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindowList)
    JUCE_DECLARE_WEAK_REFERENCEABLE (MainWindowList)
};
