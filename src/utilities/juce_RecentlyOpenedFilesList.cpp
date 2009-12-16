/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_RecentlyOpenedFilesList.h"


//==============================================================================
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

    while (getNumFiles() > maxNumberOfItems)
        files.remove (getNumFiles() - 1);
}

int RecentlyOpenedFilesList::getNumFiles() const
{
    return files.size();
}

const File RecentlyOpenedFilesList::getFile (const int index) const
{
    return File (files [index]);
}

void RecentlyOpenedFilesList::clear()
{
    files.clear();
}

void RecentlyOpenedFilesList::addFile (const File& file)
{
    const String path (file.getFullPathName());

    files.removeString (path, true);
    files.insert (0, path);

    setMaxNumberOfItems (maxNumberOfItems);
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

            if (filesToAvoid != 0)
            {
                const File** avoid = filesToAvoid;

                while (*avoid != 0)
                {
                    if (f == **avoid)
                    {
                        needsAvoiding = true;
                        break;
                    }

                    ++avoid;
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
const String RecentlyOpenedFilesList::toString() const
{
    return files.joinIntoString (T("\n"));
}

void RecentlyOpenedFilesList::restoreFromString (const String& stringifiedVersion)
{
    clear();
    files.addLines (stringifiedVersion);

    setMaxNumberOfItems (maxNumberOfItems);
}


END_JUCE_NAMESPACE
