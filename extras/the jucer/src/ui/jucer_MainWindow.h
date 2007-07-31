/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
                    public ApplicationCommandTarget
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

    bool filesDropped (const StringArray& filenames, int mouseX, int mouseY);
    void activeWindowStatusChanged();

    //==============================================================================
    const StringArray getMenuBarNames ();
    const PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName);
    void menuItemSelected (int menuItemID, int topLevelMenuIndex);

    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget();
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result);
    bool isCommandActive (const CommandID commandID);
    bool perform (const InvocationInfo& info);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    MultiDocHolder* multiDocHolder;

    JucerDocument* getActiveDocument() const throw();
};


#endif   // __JUCER_MAINWINDOW_JUCEHEADER__
