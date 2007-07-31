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

#ifndef __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
#define __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__

#include "juce_DirectoryContentsDisplayComponent.h"
#include "juce_FilePreviewComponent.h"
#include "../../../../juce_core/io/files/juce_File.h"
#include "../../../../juce_core/containers/juce_BitArray.h"
#include "../controls/juce_TextEditor.h"
#include "../controls/juce_ComboBox.h"
#include "../buttons/juce_DrawableButton.h"


//==============================================================================
/**
    A component for browsing and selecting a file or directory to open or save.

    This contains a FileListComponent and adds various boxes and controls for
    navigating and selecting a file. It can work in different modes so that it can
    be used for loading or saving a file, or for choosing a directory.

    @see FileChooserDialogBox, FileChooser, FileListComponent
*/
class JUCE_API  FileBrowserComponent  : public Component,
                                        public ChangeBroadcaster,
                                        private FileBrowserListener,
                                        private TextEditorListener,
                                        private ButtonListener,
                                        private ComboBoxListener
{
public:
    //==============================================================================
    /** Various modes that the browser can be used in.

        One of these is passed into the constructor.
    */
    enum FileChooserMode
    {
        loadFileMode,           /**< the component should allow the user to choose an existing
                                     file with the intention of opening it. */
        saveFileMode,           /**< the component should allow the user to specify the name of
                                     a file that will be used to save something. */
        chooseDirectoryMode     /**< the component should allow the user to select an existing
                                     directory. */
    };

    //==============================================================================
    /** Creates a FileBrowserComponent.

        @param browserMode              The intended purpose for the browser - see the
                                        FileChooserMode enum for the various options
        @param initialFileOrDirectory   The file or directory that should be selected when
                                        the component begins. If this is File::nonexistent,
                                        a default directory will be chosen.
        @param fileFilter               an optional filter to use to determine which files
                                        are shown. If this is 0 then all files are displayed. Note
                                        that a pointer is kept internally to this object, so
                                        make sure that it is not deleted before the browser object
                                        is deleted.
        @param previewComp              an optional preview component that will be used to
                                        show previews of files that the user selects
        @param useTreeView              if this is false, the files are shown in a list; if true,
                                        they are shown in a treeview
        @param filenameTextBoxIsReadOnly    if true, the user won't be allowed to type their own
                                        text into the filename box.
    */
    FileBrowserComponent (FileChooserMode browserMode,
                          const File& initialFileOrDirectory,
                          const FileFilter* fileFilter,
                          FilePreviewComponent* previewComp,
                          const bool useTreeView = false,
                          const bool filenameTextBoxIsReadOnly = false);

    /** Destructor. */
    ~FileBrowserComponent();

    //==============================================================================
    /**
    */
    const File getCurrentFile() const throw();

    /** Returns true if the current file is usable.

        This can be used to decide whether the user can press "ok" for the
        current file. What it does depends on the mode, so for example in an "open"
        mode, the current file is only valid if one has been selected and if the file
        exists. In a "save" mode, a non-existent file would also be valid.
    */
    bool currentFileIsValid() const;

    //==============================================================================
    /** Returns the directory whose contents are currently being shown in the listbox. */
    const File getRoot() const;

    /** Changes the directory that's being shown in the listbox. */
    void setRoot (const File& newRootDirectory);

    /** Equivalent to pressing the "up" button to browse the parent directory. */
    void goUp();

    /** Refreshes the directory that's currently being listed. */
    void refresh();

    /** Returns the browser's current mode. */
    FileChooserMode getMode() const throw()                     { return mode; }

    /** Returns a verb to describe what should happen when the file is accepted.

        E.g. if browsing in "load file" mode, this will be "Open", if in "save file"
        mode, it'll be "Save", etc.
    */
    virtual const String getActionVerb() const;

    //==============================================================================
    /** Adds a listener to be told when the user selects and clicks on files.

        @see removeListener
    */
    void addListener (FileBrowserListener* const listener) throw();

    /** Removes a listener.

        @see addListener
    */
    void removeListener (FileBrowserListener* const listener) throw();


    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void buttonClicked (Button* b);
    /** @internal */
    void comboBoxChanged (ComboBox*);

    /** @internal */
    void textEditorTextChanged (TextEditor& editor);
    /** @internal */
    void textEditorReturnKeyPressed (TextEditor& editor);
    /** @internal */
    void textEditorEscapeKeyPressed (TextEditor& editor);
    /** @internal */
    void textEditorFocusLost (TextEditor& editor);

    /** @internal */
    void selectionChanged();
    /** @internal */
    void fileClicked (const File& f, const MouseEvent& e);
    /** @internal */
    void fileDoubleClicked (const File& f);

    /** @internal */
    FilePreviewComponent* getPreviewComponent() const throw();

    juce_UseDebuggingNewOperator

protected:
    virtual const BitArray getRoots (StringArray& rootNames, StringArray& rootPaths);

private:
    //==============================================================================
    DirectoryContentsList* fileList;
    FileFilter* directoriesOnlyFilter;

    FileChooserMode mode;
    File currentRoot;
    SortedSet <void*> listeners;

    DirectoryContentsDisplayComponent* fileListComponent;
    FilePreviewComponent* previewComp;
    ComboBox* currentPathBox;
    TextEditor* filenameBox;
    DrawableButton* goUpButton;

    TimeSliceThread thread;

    void sendListenerChangeMessage();

    FileBrowserComponent (const FileBrowserComponent&);
    const FileBrowserComponent& operator= (const FileBrowserComponent&);
};



#endif   // __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
