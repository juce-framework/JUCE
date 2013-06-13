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

WildcardFileFilter::WildcardFileFilter (const String& fileWildcardPatterns,
                                        const String& directoryWildcardPatterns,
                                        const String& desc)
    : FileFilter (desc.isEmpty() ? fileWildcardPatterns
                                 : (desc + " (" + fileWildcardPatterns + ")"))
{
    parse (fileWildcardPatterns, fileWildcards);
    parse (directoryWildcardPatterns, directoryWildcards);
}

WildcardFileFilter::~WildcardFileFilter()
{
}

bool WildcardFileFilter::isFileSuitable (const File& file) const
{
    return match (file, fileWildcards);
}

bool WildcardFileFilter::isDirectorySuitable (const File& file) const
{
    return match (file, directoryWildcards);
}

//==============================================================================
void WildcardFileFilter::parse (const String& pattern, StringArray& result)
{
    result.addTokens (pattern.toLowerCase(), ";,", "\"'");

    result.trim();
    result.removeEmptyStrings();

    // special case for *.*, because people use it to mean "any file", but it
    // would actually ignore files with no extension.
    for (int i = result.size(); --i >= 0;)
        if (result[i] == "*.*")
            result.set (i, "*");
}

bool WildcardFileFilter::match (const File& file, const StringArray& wildcards)
{
    const String filename (file.getFileName());

    for (int i = wildcards.size(); --i >= 0;)
        if (filename.matchesWildcard (wildcards[i], true))
            return true;

    return false;
}
