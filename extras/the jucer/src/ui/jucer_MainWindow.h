/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_MAINWINDOW_JUCEHEADER__
#define __JUCER_MAINWINDOW_JUCEHEADER__

#include "jucer_JucerDocumentHolder.h"
class MultiDocHolder;


//==============================================================================
/**
    The big top-level window where everything happens.
*/
class MainWindow  : public DocumentWindow,
                    public MenuBarModel,
                    public ApplicationCommandTarget,
                    public FileDragAndDropTarget
{
public:
    //==============================================================================
    MainWindow();
    ~MainWindow();

    //==============================================================================
    void closeButtonPressed();

    //==============================================================================
    void openDocument (JucerDocument* const newDoc);
    bool openFile (const File& file);

    bool closeDocument (JucerDocumentHolder* designHolder);
    bool closeAllDocuments();

    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& filenames, int mouseX, int mouseY);

    void activeWindowStatusChanged();

    //==============================================================================
    StringArray getMenuBarNames ();
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool isCommandActive (const CommandID commandID);
    bool perform (const InvocationInfo& info);

private:
    MultiDocHolder* multiDocHolder;

    JucerDocument* getActiveDocument() const throw();
};


#endif   // __JUCER_MAINWINDOW_JUCEHEADER__
