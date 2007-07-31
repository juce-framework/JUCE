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

#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__
#define __JUCE_FILELISTCOMPONENT_JUCEHEADER__

#include "juce_DirectoryContentsDisplayComponent.h"
#include "juce_FileBrowserListener.h"
#include "../controls/juce_ListBox.h"
#include "../controls/juce_TreeView.h"


//==============================================================================
/**
    A component that displays the files in a directory as a listbox.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileTreeComponent
*/
class JUCE_API  FileListComponent  : public DirectoryContentsDisplayComponent,
                                     public ListBox,
                                     private ListBoxModel,
                                     private ChangeListener
{
public:
    //==============================================================================
    /** Creates a listbox to show the contents of a specified directory.
    */
    FileListComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    ~FileListComponent();

    //==============================================================================
    /** Returns the file that the user has currently selected.

        Returns File::nonexistent if none is selected.
    */
    const File getSelectedFile() const;

    /** Scrolls to the top of the list. */
    void scrollToTop();

    //==============================================================================
    /** @internal */
    void changeListenerCallback (void*);
    /** @internal */
    int getNumRows();
    /** @internal */
    void paintListBoxItem (int, Graphics&, int, int, bool);
    /** @internal */
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
    /** @internal */
    void selectedRowsChanged (int lastRowSelected);
    /** @internal */
    void deleteKeyPressed (int currentSelectedRow);
    /** @internal */
    void returnKeyPressed (int currentSelectedRow);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FileListComponent (const FileListComponent&);
    const FileListComponent& operator= (const FileListComponent&);
};


#endif   // __JUCE_FILELISTCOMPONENT_JUCEHEADER__
