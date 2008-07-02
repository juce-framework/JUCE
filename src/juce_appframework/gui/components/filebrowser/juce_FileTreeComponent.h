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

#ifndef __JUCE_FILETREECOMPONENT_JUCEHEADER__
#define __JUCE_FILETREECOMPONENT_JUCEHEADER__

#include "juce_DirectoryContentsDisplayComponent.h"
#include "../controls/juce_TreeView.h"


//==============================================================================
/**
    A component that displays the files in a directory as a treeview.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileListComponent
*/
class JUCE_API  FileTreeComponent  : public TreeView,
                                     public DirectoryContentsDisplayComponent
{
public:
    //==============================================================================
    /** Creates a listbox to show the contents of a specified directory.
    */
    FileTreeComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    ~FileTreeComponent();

    //==============================================================================
    /** Returns the number of selected files in the tree.
    */
    int getNumSelectedFiles() const throw()         { return TreeView::getNumSelectedItems(); }

    /** Returns one of the files that the user has currently selected.

        Returns File::nonexistent if none is selected.
    */
    const File getSelectedFile (int index) const throw();

    /** Returns the first of the files that the user has currently selected.

        Returns File::nonexistent if none is selected.
    */
    const File getSelectedFile() const;

    /** Scrolls the list to the top. */
    void scrollToTop();

    /** Setting a name for this allows tree items to be dragged.

        The string that you pass in here will be returned by the getDragSourceDescription()
        of the items in the tree. For more info, see TreeViewItem::getDragSourceDescription().
    */
    void setDragAndDropDescription (const String& description) throw();

    /** Returns the last value that was set by setDragAndDropDescription().
    */
    const String& getDragAndDropDescription() const throw()      { return dragAndDropDescription; }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    String dragAndDropDescription;

    FileTreeComponent (const FileTreeComponent&);
    const FileTreeComponent& operator= (const FileTreeComponent&);
};


#endif   // __JUCE_FILETREECOMPONENT_JUCEHEADER__
