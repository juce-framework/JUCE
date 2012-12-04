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

#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__
#define __JUCE_FILELISTCOMPONENT_JUCEHEADER__

#include "juce_DirectoryContentsDisplayComponent.h"
#include "juce_FileBrowserListener.h"
#include "../widgets/juce_ListBox.h"
#include "../widgets/juce_TreeView.h"


//==============================================================================
/**
    A component that displays the files in a directory as a listbox.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileTreeComponent
*/
class JUCE_API  FileListComponent  : public ListBox,
                                     public DirectoryContentsDisplayComponent,
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
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    int getNumSelectedFiles() const;

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    File getSelectedFile (int index = 0) const;

    /** Deselects any files that are currently selected. */
    void deselectAllFiles();

    /** Scrolls to the top of the list. */
    void scrollToTop();

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    void setSelectedFile (const File&);

private:
    //==============================================================================
    File lastDirectory;

    class ItemComponent;

    void changeListenerCallback (ChangeBroadcaster*);

    int getNumRows();
    void paintListBoxItem (int, Graphics&, int, int, bool);
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate);
    void selectedRowsChanged (int lastRowSelected);
    void deleteKeyPressed (int currentSelectedRow);
    void returnKeyPressed (int currentSelectedRow);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListComponent)
};


#endif   // __JUCE_FILELISTCOMPONENT_JUCEHEADER__
