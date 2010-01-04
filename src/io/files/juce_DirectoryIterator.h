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

#ifndef __JUCE_DIRECTORYITERATOR_JUCEHEADER__
#define __JUCE_DIRECTORYITERATOR_JUCEHEADER__

#include "juce_File.h"
#include "../../containers/juce_ScopedPointer.h"


//==============================================================================
/**
    Searches through a the files in a directory, returning each file that is found.

    A DirectoryIterator will search through a directory and its subdirectories using
    a wildcard filepattern match.

    If you may be finding a large number of files, this is better than
    using File::findChildFiles() because it doesn't block while it finds them
    all, and this is more memory-efficient.

    It can also guess how far it's got using a wildly inaccurate algorithm.
*/
class JUCE_API  DirectoryIterator
{
public:
    //==============================================================================
    /** Creates a DirectoryIterator for a given directory.

        After creating one of these, call its next() method to get the
        first file - e.g. @code

        DirectoryIterator iter (File ("/animals/mooses"), true, "*.moose");

        while (iter.next())
        {
            File theFileItFound (iter.getFile());

            ... etc
        }
        @endcode

        @param directory    the directory to search in
        @param isRecursive  whether all the subdirectories should also be searched
        @param wildCard     the file pattern to match
        @param whatToLookFor    a value from the File::TypesOfFileToFind enum, specifying
                                whether to look for files, directories, or both.
    */
    DirectoryIterator (const File& directory,
                       bool isRecursive,
                       const String& wildCard = JUCE_T("*"),
                       const int whatToLookFor = File::findFiles);

    /** Destructor. */
    ~DirectoryIterator();

    /** Call this to move the iterator along to the next file.

        @returns    true if a file was found (you can then use getFile() to see what it was) - or
                    false if there are no more matching files.
    */
    bool next();

    /** Returns the file that the iterator is currently pointing at.

        The result of this call is only valid after a call to next() has returned true.
    */
    const File getFile() const;

    /** Returns a guess of how far through the search the iterator has got.

        @returns    a value 0.0 to 1.0 to show the progress, although this won't be
                    very accurate.
    */
    float getEstimatedProgress() const;


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <File> filesFound;
    OwnedArray <File> dirsFound;
    String wildCard;
    int index;
    const int whatToLookFor;
    ScopedPointer <DirectoryIterator> subIterator;

    DirectoryIterator (const DirectoryIterator&);
    const DirectoryIterator& operator= (const DirectoryIterator&);
};

#endif   // __JUCE_DIRECTORYITERATOR_JUCEHEADER__
