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

#ifndef __JUCE_FILELISTCOMPONENT_JUCEHEADER__x
#define __JUCE_FILELISTCOMPONENT_JUCEHEADER__x

#include "juce_DirectoryContentsList.h"
#include "juce_FileBrowserListener.h"


//==============================================================================
/**
    A base class for components that display a list of the files in a directory.

    @see DirectoryContentsList
*/
class JUCE_API  FileListComponentBase
{
public:
    //==============================================================================
    /** 
    */
    FileListComponentBase (DirectoryContentsList& listToShow);

    /** Destructor. */
    virtual ~FileListComponentBase();

    //==============================================================================
    /** Returns the file that the user has currently selected.

        Returns File::nonexistent if none is selected.
    */
    virtual const File getSelectedFile() const = 0;

    virtual void scrollToTop() = 0;

    //==============================================================================
    /** Adds a listener to be told when files are selected or clicked.

        @see removeListener
    */
    void addListener (FileBrowserListener* const listener) throw();

    /** Removes a listener.

        @see addListener
    */
    void removeListener (FileBrowserListener* const listener) throw();


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the label.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        Note that you can also use the constants from TextEditor::ColourIds to change the
        colour of the text editor that is opened when a label is editable.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        highlightColourId      = 0x1000540, /**< The colour to use to fill a highlighted row of the list. */
        textColourId           = 0x1000541, /**< The colour for the text. */
    };

    //==============================================================================
    /** @internal */
    void sendSelectionChangeMessage();
    /** @internal */
    void sendDoubleClickMessage (const File& file);
    /** @internal */
    void sendMouseClickMessage (const File& file, const MouseEvent& e);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    DirectoryContentsList& fileList;
    SortedSet <void*> listeners;

    FileListComponentBase (const FileListComponentBase&);
    const FileListComponentBase& operator= (const FileListComponentBase&);
};


//==============================================================================
/**
    A listbox showing the files in a directory.

    @see DirectoryContentsList
*/
class JUCE_API  FileListComponent  : public FileListComponentBase,
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




//==============================================================================
/**
    A listbox showing the files in a directory.

    @see DirectoryContentsList
*/
class JUCE_API  FileTreeComponent  : public FileListComponentBase,
                                     public TreeView
{
public:
    //==============================================================================
    /** Creates a listbox to show the contents of a specified directory.
    */
    FileTreeComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    ~FileTreeComponent();

    //==============================================================================
    /** Returns the file that the user has currently selected.

        Returns File::nonexistent if none is selected.
    */
    const File getSelectedFile() const;

    void scrollToTop();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FileTreeComponent (const FileTreeComponent&);
    const FileTreeComponent& operator= (const FileTreeComponent&);
};


#endif   // __JUCE_FILELISTCOMPONENT_JUCEHEADER__
