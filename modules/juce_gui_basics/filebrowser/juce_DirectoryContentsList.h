/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A class to asynchronously scan for details about the files in a directory.

    This keeps a list of files and some information about them, using a background
    thread to scan for more files. As files are found, it broadcasts change messages
    to tell any listeners.

    @see FileListComponent, FileBrowserComponent

    @tags{GUI}
*/
class JUCE_API  DirectoryContentsList   : public ChangeBroadcaster,
                                          private TimeSliceClient
{
public:
    //==============================================================================
    /** Creates a directory list.

        To set the directory it should point to, use setDirectory(), which will
        also start it scanning for files on the background thread.

        When the background thread finds and adds new files to this list, the
        ChangeBroadcaster class will send a change message, so you can register
        listeners and update them when the list changes.

        @param fileFilter       an optional filter to select which files are
                                included in the list. If this is nullptr, then all files
                                and directories are included. Make sure that the filter
                                doesn't get deleted during the lifetime of this object
        @param threadToUse      a thread object that this list can use
                                to scan for files as a background task. Make sure
                                that the thread you give it has been started, or you
                                won't get any files!
    */
    DirectoryContentsList (const FileFilter* fileFilter,
                           TimeSliceThread& threadToUse);

    /** Destructor. */
    ~DirectoryContentsList() override;


    //==============================================================================
    /** Returns the directory that's currently being used. */
    const File& getDirectory() const noexcept               { return root; }

    /** Sets the directory to look in for files.

        If the directory that's passed in is different to the current one, this will
        also start the background thread scanning it for files.
    */
    void setDirectory (const File& directory,
                       bool includeDirectories,
                       bool includeFiles);

    /** Returns true if this list contains directories.
        @see setDirectory
    */
    bool isFindingDirectories() const noexcept              { return (fileTypeFlags & File::findDirectories) != 0; }

    /** Returns true if this list contains files.
        @see setDirectory
    */
    bool isFindingFiles() const noexcept                    { return (fileTypeFlags & File::findFiles) != 0; }

    /** Clears the list, and stops the thread scanning for files. */
    void clear();

    /** Clears the list and restarts scanning the directory for files. */
    void refresh();

    /** True if the background thread hasn't yet finished scanning for files. */
    bool isStillLoading() const;

    /** Tells the list whether or not to ignore hidden files.
        By default these are ignored.
    */
    void setIgnoresHiddenFiles (bool shouldIgnoreHiddenFiles);

    /** Returns true if hidden files are ignored.
        @see setIgnoresHiddenFiles
    */
    bool ignoresHiddenFiles() const;

    /** Replaces the current FileFilter.
        This can be nullptr to have no filter. The DirectoryContentList does not take
        ownership of this object - it just keeps a pointer to it, so you must manage its
        lifetime.
        Note that this only replaces the filter, it doesn't refresh the list - you'll
        probably want to call refresh() after calling this.
    */
    void setFileFilter (const FileFilter* newFileFilter);

    //==============================================================================
    /** Contains cached information about one of the files in a DirectoryContentsList.
    */
    struct FileInfo
    {
        //==============================================================================
        /** The filename.

            This isn't a full pathname, it's just the last part of the path, same as you'd
            get from File::getFileName().

            To get the full pathname, use DirectoryContentsList::getDirectory().getChildFile (filename).
        */
        String filename;

        /** File size in bytes. */
        int64 fileSize;

        /** File modification time.
            As supplied by File::getLastModificationTime().
        */
        Time modificationTime;

        /** File creation time.
            As supplied by File::getCreationTime().
        */
        Time creationTime;

        /** True if the file is a directory. */
        bool isDirectory;

        /** True if the file is read-only. */
        bool isReadOnly;
    };

    //==============================================================================
    /** Returns the number of files currently available in the list.

        The info about one of these files can be retrieved with getFileInfo() or getFile().

        Obviously as the background thread runs and scans the directory for files, this
        number will change.

        @see getFileInfo, getFile
    */
    int getNumFiles() const noexcept;

    /** Returns the cached information about one of the files in the list.

        If the index is in-range, this will return true and will copy the file's details
        to the structure that is passed-in.

        If it returns false, then the index wasn't in range, and the structure won't
        be affected.

        @see getNumFiles, getFile
    */
    bool getFileInfo (int index, FileInfo& resultInfo) const;

    /** Returns one of the files in the list.

        @param index    should be less than getNumFiles(). If this is out-of-range, the
                        return value will be a default File() object
        @see getNumFiles, getFileInfo
    */
    File getFile (int index) const;

    /** Returns the file filter being used.
        The filter is specified in the constructor.
    */
    const FileFilter* getFilter() const noexcept            { return fileFilter; }

    /** Returns true if the list contains the specified file. */
    bool contains (const File&) const;

    //==============================================================================
    /** @internal */
    TimeSliceThread& getTimeSliceThread() const noexcept    { return thread; }

private:
    File root;
    const FileFilter* fileFilter = nullptr;
    TimeSliceThread& thread;
    int fileTypeFlags = File::ignoreHiddenFiles | File::findFiles;

    CriticalSection fileListLock;
    OwnedArray<FileInfo> files;

    std::unique_ptr<RangedDirectoryIterator> fileFindHandle;
    std::atomic<bool> shouldStop { true }, isSearching { false };

    bool wasEmpty = true;

    int useTimeSlice() override;
    void stopSearching();
    void changed();
    bool checkNextFile (bool& hasChanged);
    bool addFile (const File&, bool isDir, int64 fileSize, Time modTime,
                  Time creationTime, bool isReadOnly);
    void setTypeFlags (int);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectoryContentsList)
};

} // namespace juce
