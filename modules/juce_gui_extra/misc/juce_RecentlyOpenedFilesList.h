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
    Manages a set of files for use as a list of recently-opened documents.

    This is a handy class for holding your list of recently-opened documents, with
    helpful methods for things like purging any non-existent files, automatically
    adding them to a menu, and making persistence easy.

    @see File, FileBasedDocument
*/
class JUCE_API  RecentlyOpenedFilesList
{
public:
    //==============================================================================
    /** Creates an empty list.
    */
    RecentlyOpenedFilesList();

    /** Destructor. */
    ~RecentlyOpenedFilesList();

    //==============================================================================
    /** Sets a limit for the number of files that will be stored in the list.

        When addFile() is called, then if there is no more space in the list, the
        least-recently added file will be dropped.

        @see getMaxNumberOfItems
    */
    void setMaxNumberOfItems (int newMaxNumber);

    /** Returns the number of items that this list will store.
        @see setMaxNumberOfItems
    */
    int getMaxNumberOfItems() const noexcept                            { return maxNumberOfItems; }

    /** Returns the number of files in the list.

        The most recently added file is always at index 0.
    */
    int getNumFiles() const;

    /** Returns one of the files in the list.

        The most recently added file is always at index 0.
    */
    File getFile (int index) const;

    /** Returns an array of all the absolute pathnames in the list.
    */
    const StringArray& getAllFilenames() const noexcept                 { return files; }

    /** Clears all the files from the list. */
    void clear();

    /** Adds a file to the list.

        The file will be added at index 0. If this file is already in the list, it will
        be moved up to index 0, but a file can only appear once in the list.

        If the list already contains the maximum number of items that is permitted, the
        least-recently added file will be dropped from the end.
    */
    void addFile (const File& file);

    /** Removes a file from the list. */
    void removeFile (const File& file);

    /** Checks each of the files in the list, removing any that don't exist.

        You might want to call this after reloading a list of files, or before putting them
        on a menu.
    */
    void removeNonExistentFiles();

    /** Tells the OS to add a file to the OS-managed list of recent documents for this app.
        Not all OSes maintain a list of recent files for an application, so this
        function will have no effect on some OSes. Currently it's just implemented for OSX.
    */
    static void registerRecentFileNatively (const File& file);

    //==============================================================================
    /** Adds entries to a menu, representing each of the files in the list.

        This is handy for creating an "open recent file..." menu in your app. The
        menu items are numbered consecutively starting with the baseItemId value,
        and can either be added as complete pathnames, or just the last part of the
        filename.

        If dontAddNonExistentFiles is true, then each file will be checked and only those
        that exist will be added.

        If filesToAvoid is not a nullptr, then it is considered to be a zero-terminated array
        of pointers to file objects. Any files that appear in this list will not be added to
        the menu - the reason for this is that you might have a number of files already open,
        so might not want these to be shown in the menu.

        It returns the number of items that were added.
    */
    int createPopupMenuItems (PopupMenu& menuToAddItemsTo,
                              int baseItemId,
                              bool showFullPaths,
                              bool dontAddNonExistentFiles,
                              const File** filesToAvoid = nullptr);

    //==============================================================================
    /** Returns a string that encapsulates all the files in the list.

        The string that is returned can later be passed into restoreFromString() in
        order to recreate the list. This is handy for persisting your list, e.g. in
        a PropertiesFile object.

        @see restoreFromString
    */
    String toString() const;

    /** Restores the list from a previously stringified version of the list.

        Pass in a stringified version created with toString() in order to persist/restore
        your list.

        @see toString
    */
    void restoreFromString (const String& stringifiedVersion);


private:
    //==============================================================================
    StringArray files;
    int maxNumberOfItems;

    JUCE_LEAK_DETECTOR (RecentlyOpenedFilesList)
};

} // namespace juce
