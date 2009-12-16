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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_WildcardFileFilter.h"


//==============================================================================
WildcardFileFilter::WildcardFileFilter (const String& fileWildcardPatterns,
                                        const String& directoryWildcardPatterns,
                                        const String& description_)
    : FileFilter (description_.isEmpty() ? fileWildcardPatterns
                                         : (description_ + T(" (") + fileWildcardPatterns + T(")")))
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
void WildcardFileFilter::parse (const String& pattern, StringArray& result) throw()
{
    result.addTokens (pattern.toLowerCase(), T(";,"), T("\"'"));

    result.trim();
    result.removeEmptyStrings();

    // special case for *.*, because people use it to mean "any file", but it
    // would actually ignore files with no extension.
    for (int i = result.size(); --i >= 0;)
        if (result[i] == T("*.*"))
            result.set (i, T("*"));
}

bool WildcardFileFilter::match (const File& file, const StringArray& wildcards) throw()
{
    const String filename (file.getFileName());

    for (int i = wildcards.size(); --i >= 0;)
        if (filename.matchesWildcard (wildcards[i], true))
            return true;

    return false;
}


END_JUCE_NAMESPACE
