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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_DirectoryIterator.h"

void* juce_findFileStart (const String& directory, const String& wildCard, String& firstResultFile,
                          bool* isDirectory, bool* isHidden, int64* fileSize,
                          Time* modTime, Time* creationTime, bool* isReadOnly);
bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDirectory, bool* isHidden, int64* fileSize,
                        Time* modTime, Time* creationTime, bool* isReadOnly);
void juce_findFileClose (void* handle);


//==============================================================================
DirectoryIterator::DirectoryIterator (const File& directory,
                                      bool isRecursive,
                                      const String& wc,
                                      const int whatToLookFor_)
  : wildCard (wc),
    index (-1),
    whatToLookFor (whatToLookFor_)
{
    // you have to specify the type of files you're looking for!
    jassert ((whatToLookFor_ & (File::findFiles | File::findDirectories)) != 0);
    jassert (whatToLookFor_ > 0 && whatToLookFor_ <= 7);

    String path (directory.getFullPathName());
    if (! path.endsWithChar (File::separator))
        path += File::separator;

    String filename;
    bool isDirectory, isHidden;

    void* const handle = juce_findFileStart (path,
                                             isRecursive ? T("*") : wc,
                                             filename, &isDirectory, &isHidden, 0, 0, 0, 0);

    if (handle != 0)
    {
        do
        {
            if (! filename.containsOnly (T(".")))
            {
                bool addToList = false;

                if (isDirectory)
                {
                    if (isRecursive
                         && ((whatToLookFor_ & File::ignoreHiddenFiles) == 0
                              || ! isHidden))
                    {
                        dirsFound.add (new File (path + filename, 0));
                    }

                    addToList = (whatToLookFor_ & File::findDirectories) != 0;
                }
                else
                {
                    addToList = (whatToLookFor_ & File::findFiles) != 0;
                }

                // if it's recursive, we're not relying on the OS iterator
                // to do the wildcard match, so do it now..
                if (isRecursive && addToList)
                    addToList = filename.matchesWildcard (wc, true);

                if (addToList && (whatToLookFor_ & File::ignoreHiddenFiles) != 0)
                    addToList = ! isHidden;

                if (addToList)
                    filesFound.add (new File (path + filename, 0));
            }

        } while (juce_findFileNext (handle, filename, &isDirectory, &isHidden, 0, 0, 0, 0));

        juce_findFileClose (handle);
    }
}

DirectoryIterator::~DirectoryIterator()
{
}

bool DirectoryIterator::next()
{
    if (subIterator != 0)
    {
        if (subIterator->next())
            return true;

        subIterator = 0;
    }

    if (index >= filesFound.size() + dirsFound.size() - 1)
        return false;

    ++index;

    if (index >= filesFound.size())
    {
        subIterator = new DirectoryIterator (*(dirsFound [index - filesFound.size()]),
                                             true, wildCard, whatToLookFor);
        return next();
    }

    return true;
}

const File DirectoryIterator::getFile() const
{
    if (subIterator != 0)
        return subIterator->getFile();

    const File* const f = filesFound [index];

    return (f != 0) ? *f
                    : File::nonexistent;
}

float DirectoryIterator::getEstimatedProgress() const
{
    if (filesFound.size() + dirsFound.size() == 0)
    {
        return 0.0f;
    }
    else
    {
        const float detailedIndex = (subIterator != 0) ? index + subIterator->getEstimatedProgress()
                                                       : (float) index;

        return detailedIndex / (filesFound.size() + dirsFound.size());
    }
}

END_JUCE_NAMESPACE
