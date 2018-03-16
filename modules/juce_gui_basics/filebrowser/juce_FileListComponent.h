/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component that displays the files in a directory as a listbox.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileTreeComponent

    @tags{GUI}
*/
class JUCE_API  FileListComponent  : public ListBox,
                                     public DirectoryContentsDisplayComponent,
                                     private ListBoxModel,
                                     private ChangeListener
{
public:
    //==============================================================================
    /** Creates a listbox to show the contents of a specified directory. */
    FileListComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    ~FileListComponent();

    //==============================================================================
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    int getNumSelectedFiles() const override;

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    File getSelectedFile (int index = 0) const override;

    /** Deselects any files that are currently selected. */
    void deselectAllFiles() override;

    /** Scrolls to the top of the list. */
    void scrollToTop() override;

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    void setSelectedFile (const File&) override;

private:
    //==============================================================================
    File lastDirectory, fileWaitingToBeSelected;
    class ItemComponent;

    void changeListenerCallback (ChangeBroadcaster*) override;
    int getNumRows() override;
    void paintListBoxItem (int, Graphics&, int, int, bool) override;
    Component* refreshComponentForRow (int rowNumber, bool isRowSelected, Component*) override;
    void selectedRowsChanged (int row) override;
    void deleteKeyPressed (int currentSelectedRow) override;
    void returnKeyPressed (int currentSelectedRow) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileListComponent)
};

} // namespace juce
