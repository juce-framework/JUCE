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

    if (includeDirectories) newFlags |=  File::findDirectories;
    else                    newFlags &= ~File::findDirectories;

    if (includeFiles)       newFlags |=  File::findFiles;
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
    isSearching = false;
}

void DirectoryContentsList::clear()
{
    stopSearching();

    if (! files.isEmpty())
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
        fileFindHandle = std::make_unique<RangedDirectoryIterator> (root, false, "*", fileTypeFlags);
        shouldStop = false;
        isSearching = true;
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
        if (root.getChildFile (files.getUnchecked (i)->filename) == targetFile)
            return true;

    return false;
}

bool DirectoryContentsList::isStillLoading() const
{
    return isSearching;
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
    if (fileFindHandle == nullptr)
        return false;

    if (*fileFindHandle == RangedDirectoryIterator())
    {
        fileFindHandle = nullptr;
        isSearching = false;
        hasChanged = true;
        return false;
    }

    const auto entry = *(*fileFindHandle)++;

    hasChanged |= addFile (entry.getFile(),
                           entry.isDirectory(),
                           entry.getFileSize(),
                           entry.getModificationTime(),
                           entry.getCreationTime(),
                           entry.isReadOnly());

    return true;
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
            if (files.getUnchecked (i)->filename == info->filename)
                return false;

        files.add (std::move (info));

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
