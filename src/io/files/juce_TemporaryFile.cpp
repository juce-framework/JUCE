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

#include "juce_TemporaryFile.h"
#include "../../core/juce_Random.h"
#include "../../threads/juce_Thread.h"


//==============================================================================
TemporaryFile::TemporaryFile (const String& suffix, const int optionFlags)
{
    createTempFile (File::getSpecialLocation (File::tempDirectory),
                    T("temp_") + String (Random::getSystemRandom().nextInt()),
                    suffix,
                    optionFlags);
}

TemporaryFile::TemporaryFile (const File& targetFile_, const int optionFlags)
    : targetFile (targetFile_)
{
    // If you use this constructor, you need to give it a valid target file!
    jassert (targetFile != File::nonexistent);

    createTempFile (targetFile.getParentDirectory(),
                    targetFile.getFileNameWithoutExtension() + T("_temp") + String (Random::getSystemRandom().nextInt()),
                    targetFile.getFileExtension(),
                    optionFlags);
}

void TemporaryFile::createTempFile (const File& parentDirectory, String name,
                                    const String& suffix, const int optionFlags)
{
    if ((optionFlags & useHiddenFile) != 0)
        name = T(".") + name;

    temporaryFile = parentDirectory.getNonexistentChildFile (name, suffix, (optionFlags & putNumbersInBrackets) != 0);
}

TemporaryFile::~TemporaryFile()
{
    // Have a few attempts at deleting the file before giving up..
    for (int i = 5; --i >= 0;)
    {
        if (temporaryFile.deleteFile())
            return;

        Thread::sleep (50);
    }

    // Failed to delete our temporary file! Check that you've deleted all the
    // file output streams that were using it!
    jassertfalse;
}

//==============================================================================
bool TemporaryFile::overwriteTargetFileWithTemporary() const
{
    // This method only works if you created this object with the constructor
    // that takes a target file!
    jassert (targetFile != File::nonexistent);

    if (temporaryFile.exists())
    {
        // Have a few attempts at overwriting the file before giving up..
        for (int i = 5; --i >= 0;)
        {
            if (temporaryFile.moveFileTo (targetFile))
                return true;

            Thread::sleep (100);
        }

        // Failed to overwrite the new file! Make sure you've not left any
        // file streams hanging around when you call this method!
        jassertfalse
    }
    else
    {
        // There's no temporary file to use. If your write failed, you should
        // probably check, and not bother calling this method.
        jassertfalse
    }

    return false;
}


END_JUCE_NAMESPACE
