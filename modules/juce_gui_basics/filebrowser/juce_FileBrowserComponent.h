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

#ifndef __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
#define __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__

#include "juce_DirectoryContentsDisplayComponent.h"
#include "juce_FilePreviewComponent.h"
#include "../widgets/juce_TextEditor.h"
#include "../widgets/juce_ComboBox.h"
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
                                        private FileBrowserListener,
                                        private TextEditorListener,
                                        private ButtonListener,
                                        private ComboBoxListener,  // (can't use ComboBox::Listener due to idiotic VC2005 bug)
                                        private FileFilter
{
public:
    //==============================================================================
    /** Various options for the browser.

        A combination of these is passed into the FileBrowserComponent constructor.
    */
    enum FileChooserFlags
    {
        openMode                = 1,    /**< specifies that the component should allow the user to
                                             choose an existing file with the intention of opening it. */
        saveMode                = 2,    /**< specifies that the component should allow the user to specify
                                             the name of a file that will be used to save something. */
        canSelectFiles          = 4,    /**< specifies that the user can select files (can be used in
                                             conjunction with canSelectDirectories). */
        canSelectDirectories    = 8,    /**< specifies that the user can select directories (can be used in
                                             conjuction with canSelectFiles). */
        canSelectMultipleItems  = 16,   /**< specifies that the user can select multiple items. */
        useTreeView             = 32,   /**< specifies that a tree-view should be shown instead of a file list. */
        filenameBoxIsReadOnly   = 64,   /**< specifies that the user can't type directly into the filename box. */
        warnAboutOverwriting    = 128   /**< specifies that the dialog should warn about overwriting existing files (if possible). */
    };

    //==============================================================================
    /** Creates a FileBrowserComponent.

        @param flags                    A combination of flags from the FileChooserFlags enumeration, used to
                                        specify the component's behaviour. The flags must contain either openMode
                                        or saveMode, and canSelectFiles and/or canSelectDirectories.
        @param initialFileOrDirectory   The file or directory that should be selected when the component begins.
                                        If this is File::nonexistent, a default directory will be chosen.
        @param fileFilter               an optional filter to use to determine which files are shown.
                                        If this is nullptr then all files are displayed. Note that a pointer
                                        is kept internally to this object, so make sure that it is not deleted
                                        before the FileBrowserComponent object is deleted.
        @param previewComp              an optional preview component that will be used to show previews of
                                        files that the user selects
    */
    FileBrowserComponent (int flags,
                          const File& initialFileOrDirectory,
                          const FileFilter* fileFilter,
                          FilePreviewComponent* previewComp);

    /** Destructor. */
    ~FileBrowserComponent();

    //==============================================================================
    /** Returns the number of files that the user has got selected.
        If multiple select isn't active, this will only be 0 or 1. To get the complete
        list of files they've chosen, pass an index to getCurrentFile().
    */
    int getNumSelectedFiles() const noexcept;

    /** Returns one of the files that the user has chosen.
        If the box has multi-select enabled, the index lets you specify which of the files
        to get - see getNumSelectedFiles() to find out how many files were chosen.
        @see getHighlightedFile
    */
    File getSelectedFile (int index) const noexcept;

    /** Deselects any files that are currently selected.
    */
    void deselectAllFiles();

    /** Returns true if the currently selected file(s) are usable.

        This can be used to decide whether the user can press "ok" for the
        current file. What it does depends on the mode, so for example in an "open"
        mode, this only returns true if a file has been selected and if it exists.
        In a "save" mode, a non-existent file would also be valid.
    */
    bool currentFileIsValid() const;

    /** This returns the last item in the view that the user has highlighted.
        This may be different from getCurrentFile(), which returns the value
        that is shown in the filename box, and if there are multiple selections,
        this will only return one of them.
        @see getSelectedFile
    */
    File getHighlightedFile() const noexcept;

    //==============================================================================
    /** Returns the directory whose contents are currently being shown in the listbox. */
    const File& getRoot() const;

    /** Changes the directory that's being shown in the listbox. */
    void setRoot (const File& newRootDirectory);

    /** Changes the name that is currently shown in the filename box. */
    void setFileName (const String& newName);

    /** Equivalent to pressing the "up" button to browse the parent directory. */
    void goUp();

    /** Refreshes the directory that's currently being listed. */
    void refresh();

    /** Changes the filter that's being used to sift the files. */
    void setFileFilter (const FileFilter* newFileFilter);

    /** Returns a verb to describe what should happen when the file is accepted.

        E.g. if browsing in "load file" mode, this will be "Open", if in "save file"
        mode, it'll be "Save", etc.
    */
    virtual String getActionVerb() const;

    /** Returns true if the saveMode flag was set when this component was created.
    */
    bool isSaveMode() const noexcept;

    /** Sets the label that will be displayed next to the filename entry box.
        By default this is just "file", but you might want to change it to something more
        appropriate for your app.
    */
    void setFilenameBoxLabel (const String& name);

    //==============================================================================
    /** Adds a listener to be told when the user selects and clicks on files.
        @see removeListener
    */
    void addListener (FileBrowserListener* listener);

    /** Removes a listener.
        @see addListener
    */
    void removeListener (FileBrowserListener* listener);


    //==============================================================================
    /** @internal */
    void resized();
    /** @internal */
    void buttonClicked (Button*);
    /** @internal */
    void comboBoxChanged (ComboBox*);
    /** @internal */
    void textEditorTextChanged (TextEditor&);
    /** @internal */
    void textEditorReturnKeyPressed (TextEditor&);
    /** @internal */
    void textEditorEscapeKeyPressed (TextEditor&);
    /** @internal */
    void textEditorFocusLost (TextEditor&);
    /** @internal */
    bool keyPressed (const KeyPress&);
    /** @internal */
    void selectionChanged();
    /** @internal */
    void fileClicked (const File&, const MouseEvent&);
    /** @internal */
    void fileDoubleClicked (const File&);
    /** @internal */
    void browserRootChanged (const File&);
    /** @internal */
    bool isFileSuitable (const File&) const;
    /** @internal */
    bool isDirectorySuitable (const File&) const;

    /** @internal */
    FilePreviewComponent* getPreviewComponent() const noexcept;

    /** @internal */
    DirectoryContentsDisplayComponent* getDisplayComponent() const noexcept;

protected:
    /** Returns a list of names and paths for the default places the user might want to look.
        Use an empty string to indicate a section break.
    */
    virtual void getRoots (StringArray& rootNames, StringArray& rootPaths);

    /** Updates the items in the dropdown list of recent paths with the values from getRoots(). */
    void resetRecentPaths();

private:
    //==============================================================================
    ScopedPointer <DirectoryContentsList> fileList;
    const FileFilter* fileFilter;

    int flags;
    File currentRoot;
    Array<File> chosenFiles;
    ListenerList <FileBrowserListener> listeners;

    ScopedPointer<DirectoryContentsDisplayComponent> fileListComponent;
    FilePreviewComponent* previewComp;
    ComboBox currentPathBox;
    TextEditor filenameBox;
    Label fileLabel;
    ScopedPointer<Button> goUpButton;

    TimeSliceThread thread;

    void sendListenerChangeMessage();
    bool isFileOrDirSuitable (const File& f) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileBrowserComponent)
};



#endif   // __JUCE_FILEBROWSERCOMPONENT_JUCEHEADER__
