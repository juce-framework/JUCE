/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
                    public MenuBarModel,
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
    void askUserToOpenFile();
    bool canOpenFile (const File& file) const;
    bool openFile (const File& file);
    void createNewProject();
    void setProject (Project* newProject);
    void reloadLastProject();

    bool closeProject (Project* project);
    bool closeCurrentProject();
    bool closeAllDocuments (bool askUserToSave);

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int mouseX, int mouseY);

    void activeWindowStatusChanged();

    void updateTitle (const String& documentName);

    //==============================================================================
    const StringArray getMenuBarNames();
    const PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ScopedPointer <Project> currentProject;

    ProjectContentComponent* getProjectContentComponent() const;
};


#endif   // __JUCER_MAINWINDOW_JUCEHEADER__
