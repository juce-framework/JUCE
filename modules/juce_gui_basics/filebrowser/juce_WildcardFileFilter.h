/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_WILDCARDFILEFILTER_H_INCLUDED
#define JUCE_WILDCARDFILEFILTER_H_INCLUDED


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



#endif   // JUCE_WILDCARDFILEFILTER_H_INCLUDED
