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

#ifndef __JUCER_MAINWINDOW_JUCEHEADER__
#define __JUCER_MAINWINDOW_JUCEHEADER__

#include "../Project/jucer_ProjectContentComponent.h"


//==============================================================================
/**
    The big top-level window where everything happens.
*/
class MainWindow  : public DocumentWindow,
                    public ApplicationCommandTarget,
                    public FileDragAndDropTarget,
                    public DragAndDropContainer
{
public:
    //==============================================================================
    MainWindow();
    ~MainWindow();

    //==============================================================================
    void closeButtonPressed();

    //==============================================================================
    bool canOpenFile (const File& file) const;
    bool openFile (const File& file);
    void setProject (Project* newProject);
    Project* getProject() const                 { return currentProject; }

    void makeVisible();
    void restoreWindowPosition();
    bool closeProject (Project* project);
    bool closeCurrentProject();

    void showNewProjectWizard();

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int mouseX, int mouseY);

    void activeWindowStatusChanged();

    void updateTitle (const String& documentName);

    ProjectContentComponent* getProjectContentComponent() const;

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

private:
    ScopedPointer <Project> currentProject;

    String getProjectWindowPosName() const
    {
        jassert (currentProject != nullptr);
        if (currentProject == nullptr)
            return String::empty;

        return "projectWindowPos_" + currentProject->getProjectUID();
    }

    void createProjectContentCompIfNeeded();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow);
};

//==============================================================================
class MainWindowList
{
public:
    MainWindowList();

    void forceCloseAllWindows();
    bool askAllWindowsToClose();
    void closeWindow (MainWindow*);

    void createWindowIfNoneAreOpen();
    void openDocument (OpenDocumentManager::Document*, bool grabFocus);
    bool openFile (const File& file);

    MainWindow* createNewMainWindow();
    MainWindow* getOrCreateFrontmostWindow();
    MainWindow* getOrCreateEmptyWindow();

    void reopenLastProjects();
    void saveCurrentlyOpenProjectList();

    void avoidSuperimposedWindows (MainWindow*);

    void sendLookAndFeelChange();

    OwnedArray<MainWindow> windows;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindowList);
};


#endif   // __JUCER_MAINWINDOW_JUCEHEADER__
