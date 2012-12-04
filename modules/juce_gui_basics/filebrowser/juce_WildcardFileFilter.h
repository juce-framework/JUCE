/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_WILDCARDFILEFILTER_JUCEHEADER__
#define __JUCE_WILDCARDFILEFILTER_JUCEHEADER__

#include "juce_FileFilter.h"


//==============================================================================
/**
    A type of FileFilter that works by wildcard pattern matching.

    This filter only allows files that match one of the specified patterns, but
    allows all directories through.

    @see FileFilter, DirectoryContentsList, FileListComponent, FileBrowserComponent
*/
class JUCE_API  WildcardFileFilter  : public FileFilter
{
public:
    //==============================================================================
    /**
        Creates a wildcard filter for one or more patterns.

        The wildcardPatterns parameter is a comma or semicolon-delimited set of
        patterns, e.g. "*.wav;*.aiff" would look for files ending in either .wav
        or .aiff.

        Passing an empty string as a pattern will fail to match anything, so by leaving
        either the file or directory pattern parameter empty means you can control
        whether files or directories are found.

        The description is a name to show the user in a list of possible patterns, so
        for the wav/aiff example, your description might be "audio files".
    */
    WildcardFileFilter (const String& fileWildcardPatterns,
                        const String& directoryWildcardPatterns,
                        const String& description);

    /** Destructor. */
    ~WildcardFileFilter();

    //==============================================================================
    /** Returns true if the filename matches one of the patterns specified. */
    bool isFileSuitable (const File& file) const;

    /** This always returns true. */
    bool isDirectorySuitable (const File& file) const;

private:
    //==============================================================================
    StringArray fileWildcards, directoryWildcards;

    static void parse (const String& pattern, StringArray& result);
    static bool match (const File& file, const StringArray& wildcards);

    JUCE_LEAK_DETECTOR (WildcardFileFilter)
};



#endif   // __JUCE_WILDCARDFILEFILTER_JUCEHEADER__
