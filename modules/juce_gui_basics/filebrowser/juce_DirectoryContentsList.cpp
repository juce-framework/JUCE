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

DirectoryContentsList::DirectoryContentsList (const FileFilter* f, TimeSliceThread& t)
   : fileFilter (f), thread (t)
{
}

DirectoryContentsList::~DirectoryContentsList()
{
    stopSearching();
}

void DirectoryContentsList::setIgnoresHiddenFiles (const bool shouldIgnoreHiddenFiles)
{
    setTypeFlags (shouldIgnoreHiddenFiles ? (fileTypeFlags | File::ignoreHiddenFiles)
                                          : (fileTypeFlags & ~File::ignoreHiddenFiles));
}

bool DirectoryContentsList::ignoresHiddenFiles() const
{
    return (fileTypeFlags & File::ignoreHiddenFiles) != 0;
}

//==============================================================================
void DirectoryContentsList::setDirectory (const File& directory,
                                          const bool includeDirectories,
                                          const bool includeFiles)
{
    jassert (includeDirectories || includeFiles); // you have to specify at least one of these!

    if (directory != root)
    {
        clear();
        root = directory;
        changed();

        // (this forces a refresh when setTypeFlags() is called, rather than triggering two refreshes)
        fileTypeFlags &= ~(File::findDirectories | File::findFiles);
    }

    auto newFlags = fileTypeFlags;

    if (includeDirectories) newFlags |= File::findDirectories;
    else                    newFlags &= ~File::findDirectories;

    if (includeFiles)       newFlags |= File::findFiles;
    else                    newFlags &= ~File::findFiles;

    setTypeFlags (newFlags);
}

void DirectoryContentsList::setTypeFlags (const int newFlags)
{
    if (fileTypeFlags != newFlags)
    {
        fileTypeFlags = newFlags;
        refresh();
    }
}

void DirectoryContentsList::stopSearching()
{
    shouldStop = true;
    thread.removeTimeSliceClient (this);
    fileFindHandle.reset();
}

void DirectoryContentsList::clear()
{
    stopSearching();

    if (files.size() > 0)
    {
        files.clear();
        changed();
    }
}

void DirectoryContentsList::refresh()
{
    stopSearching();
    wasEmpty = files.isEmpty();
    files.clear();

    if (root.isDirectory())
    {
        fileFindHandle.reset (new DirectoryIterator (root, false, "*", fileTypeFlags));
        shouldStop = false;
        thread.addTimeSliceClient (this);
    }
}

void DirectoryContentsList::setFileFilter (const FileFilter* newFileFilter)
{
    const ScopedLock sl (fileListLock);
    fileFilter = newFileFilter;
}

//==============================================================================
int DirectoryContentsList::getNumFiles() const noexcept
{
    const ScopedLock sl (fileListLock);
    return files.size();
}

bool DirectoryContentsList::getFileInfo (const int index, FileInfo& result) const
{
    const ScopedLock sl (fileListLock);

    if (auto* info = files [index])
    {
        result = *info;
        return true;
    }

    return false;
}

File DirectoryContentsList::getFile (const int index) const
{
    const ScopedLock sl (fileListLock);

    if (auto* info = files [index])
        return root.getChildFile (info->filename);

    return {};
}

bool DirectoryContentsList::contains (const File& targetFile) const
{
    const ScopedLock sl (fileListLock);

    for (int i = files.size(); --i >= 0;)
        if (root.getChildFile (files.getUnchecked(i)->filename) == targetFile)
            return true;

    return false;
}

bool DirectoryContentsList::isStillLoading() const
{
    return fileFindHandle != nullptr;
}

void DirectoryContentsList::changed()
{
    sendChangeMessage();
}

//==============================================================================
int DirectoryContentsList::useTimeSlice()
{
    auto startTime = Time::getApproximateMillisecondCounter();
    bool hasChanged = false;

    for (int i = 100; --i >= 0;)
    {
        if (! checkNextFile (hasChanged))
        {
            if (hasChanged)
                changed();

            return 500;
        }

        if (shouldStop || (Time::getApproximateMillisecondCounter() > startTime + 150))
            break;
    }

    if (hasChanged)
        changed();

    return 0;
}

bool DirectoryContentsList::checkNextFile (bool& hasChanged)
{
    if (fileFindHandle != nullptr)
    {
        bool fileFoundIsDir, isHidden, isReadOnly;
        int64 fileSize;
        Time modTime, creationTime;

        if (fileFindHandle->next (&fileFoundIsDir, &isHidden, &fileSize,
                                  &modTime, &creationTime, &isReadOnly))
        {
            if (addFile (fileFindHandle->getFile(), fileFoundIsDir,
                         fileSize, modTime, creationTime, isReadOnly))
            {
                hasChanged = true;
            }

            return true;
        }

        fileFindHandle.reset();

        if (! wasEmpty && files.isEmpty())
            hasChanged = true;
    }

    return false;
}

bool DirectoryContentsList::addFile (const File& file, const bool isDir,
                                     const int64 fileSize,
                                     Time modTime, Time creationTime,
                                     const bool isReadOnly)
{
    const ScopedLock sl (fileListLock);

    if (fileFilter == nullptr
         || ((! isDir) && fileFilter->isFileSuitable (file))
         || (isDir && fileFilter->isDirectorySuitable (file)))
    {
        auto info = std::make_unique<FileInfo>();

        info->filename         = file.getFileName();
        info->fileSize         = fileSize;
        info->modificationTime = modTime;
        info->creationTime     = creationTime;
        info->isDirectory      = isDir;
        info->isReadOnly       = isReadOnly;

        for (int i = files.size(); --i >= 0;)
            if (files.getUnchecked(i)->filename == info->filename)
                return false;

        files.add (info.release());

        std::sort (files.begin(), files.end(), [] (const FileInfo* a, const FileInfo* b)
        {
           #if JUCE_WINDOWS
            if (a->isDirectory != b->isDirectory)
                return a->isDirectory;
           #endif

            return a->filename.compareNatural (b->filename) < 0;
        });

        return true;
    }

    return false;
}

} // namespace juce
