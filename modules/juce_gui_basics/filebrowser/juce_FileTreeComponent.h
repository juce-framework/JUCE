/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    A component that displays the files in a directory as a treeview.

    This implements the DirectoryContentsDisplayComponent base class so that
    it can be used in a FileBrowserComponent.

    To attach a listener to it, use its DirectoryContentsDisplayComponent base
    class and the FileBrowserListener class.

    @see DirectoryContentsList, FileListComponent

    @tags{GUI}
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
    ~FileTreeComponent() override;

    //==============================================================================
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    int getNumSelectedFiles() const override               { return TreeView::getNumSelectedItems(); }

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    File getSelectedFile (int index = 0) const override;

    /** Deselects any files that are currently selected. */
    void deselectAllFiles() override;

    /** Scrolls the list to the top. */
    void scrollToTop() override;

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    void setSelectedFile (const File&) override;

    /** Updates the files in the list. */
    void refresh();

    /** Setting a name for this allows tree items to be dragged.

        The string that you pass in here will be returned by the getDragSourceDescription()
        of the items in the tree. For more info, see TreeViewItem::getDragSourceDescription().
    */
    void setDragAndDropDescription (const String& description);

    /** Returns the last value that was set by setDragAndDropDescription().
    */
    const String& getDragAndDropDescription() const noexcept    { return dragAndDropDescription; }

    /** Changes the height of the treeview items. */
    void setItemHeight (int newHeight);

    /** Returns the height of the treeview items. */
    int getItemHeight() const noexcept                          { return itemHeight; }

private:
    //==============================================================================
    String dragAndDropDescription;
    int itemHeight;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileTreeComponent)
};

} // namespace juce
