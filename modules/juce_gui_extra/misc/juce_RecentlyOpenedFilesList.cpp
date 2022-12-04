/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

RecentlyOpenedFilesList::RecentlyOpenedFilesList()
    : maxNumberOfItems (10)
{
}

//==============================================================================
void RecentlyOpenedFilesList::setMaxNumberOfItems (const int newMaxNumber)
{
    maxNumberOfItems = jmax (1, newMaxNumber);

    files.removeRange (maxNumberOfItems, getNumFiles());
}

int RecentlyOpenedFilesList::getNumFiles() const
{
    return files.size();
}

File RecentlyOpenedFilesList::getFile (const int index) const
{
    return File (files [index]);
}

void RecentlyOpenedFilesList::clear()
{
    files.clear();
}

void RecentlyOpenedFilesList::addFile (const File& file)
{
    removeFile (file);
    files.insert (0, file.getFullPathName());

    setMaxNumberOfItems (maxNumberOfItems);
}

void RecentlyOpenedFilesList::removeFile (const File& file)
{
    files.removeString (file.getFullPathName());
}

void RecentlyOpenedFilesList::removeNonExistentFiles()
{
    for (int i = getNumFiles(); --i >= 0;)
        if (! getFile(i).exists())
            files.remove (i);
}

//==============================================================================
int RecentlyOpenedFilesList::createPopupMenuItems (PopupMenu& menuToAddTo,
                                                   const int baseItemId,
                                                   const bool showFullPaths,
                                                   const bool dontAddNonExistentFiles,
                                                   const File** filesToAvoid)
{
    int num = 0;

    for (int i = 0; i < getNumFiles(); ++i)
    {
        const File f (getFile(i));

        if ((! dontAddNonExistentFiles) || f.exists())
        {
            bool needsAvoiding = false;

            if (filesToAvoid != nullptr)
            {
                for (const File** avoid = filesToAvoid; *avoid != nullptr; ++avoid)
                {
                    if (f == **avoid)
                    {
                        needsAvoiding = true;
                        break;
                    }
                }
            }

            if (! needsAvoiding)
            {
                menuToAddTo.addItem (baseItemId + i,
                                     showFullPaths ? f.getFullPathName()
                                                   : f.getFileName());
                ++num;
            }
        }
    }

    return num;
}

//==============================================================================
String RecentlyOpenedFilesList::toString() const
{
    return files.joinIntoString ("\n");
}

void RecentlyOpenedFilesList::restoreFromString (const String& stringifiedVersion)
{
    clear();
    files.addLines (stringifiedVersion);

    setMaxNumberOfItems (maxNumberOfItems);
}


//==============================================================================
void RecentlyOpenedFilesList::registerRecentFileNatively ([[maybe_unused]] const File& file)
{
   #if JUCE_MAC
    JUCE_AUTORELEASEPOOL
    {
        [[NSDocumentController sharedDocumentController] noteNewRecentDocumentURL: createNSURLFromFile (file)];
    }
   #endif
}

void RecentlyOpenedFilesList::forgetRecentFileNatively ([[maybe_unused]] const File& file)
{
   #if JUCE_MAC
    JUCE_AUTORELEASEPOOL
    {
        // for some reason, OSX doesn't provide a method to just remove a single file
        // from the recent list, so we clear them all and add them back excluding
        // the specified file

        auto sharedDocController = [NSDocumentController sharedDocumentController];
        auto recentDocumentURLs  = [sharedDocController recentDocumentURLs];

        [sharedDocController clearRecentDocuments: nil];

        auto* nsFile = createNSURLFromFile (file);

        auto reverseEnumerator = [recentDocumentURLs reverseObjectEnumerator];

        for (NSURL* url : reverseEnumerator)
            if (! [url isEqual:nsFile])
                [sharedDocController noteNewRecentDocumentURL:url];
    }
   #endif
}

void RecentlyOpenedFilesList::clearRecentFilesNatively()
{
   #if JUCE_MAC
    JUCE_AUTORELEASEPOOL
    {
        [[NSDocumentController sharedDocumentController] clearRecentDocuments: nil];
    }
   #endif
}

} // namespace juce
