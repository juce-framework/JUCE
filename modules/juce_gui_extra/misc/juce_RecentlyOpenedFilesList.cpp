/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

RecentlyOpenedFilesList::RecentlyOpenedFilesList()
    : maxNumberOfItems (10)
{
}

RecentlyOpenedFilesList::~RecentlyOpenedFilesList()
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
void RecentlyOpenedFilesList::registerRecentFileNatively (const File& file)
{
   #if JUCE_MAC
    JUCE_AUTORELEASEPOOL
    {
        [[NSDocumentController sharedDocumentController]
            noteNewRecentDocumentURL: [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())]];
    }
   #else
    ignoreUnused (file);
   #endif
}
