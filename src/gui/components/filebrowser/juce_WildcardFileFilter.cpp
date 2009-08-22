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
WildcardFileFilter::WildcardFileFilter (const String& wildcardPatterns,
                                        const String& description)
    : FileFilter (description.isEmpty() ? wildcardPatterns
                                        : (description + T(" (") + wildcardPatterns + T(")")))
{
    wildcards.addTokens (wildcardPatterns.toLowerCase(), T(";,"), T("\"'"));

    wildcards.trim();
    wildcards.removeEmptyStrings();

    // special case for *.*, because people use it to mean "any file", but it
    // would actually ignore files with no extension.
    for (int i = wildcards.size(); --i >= 0;)
        if (wildcards[i] == T("*.*"))
            wildcards.set (i, T("*"));
}

WildcardFileFilter::~WildcardFileFilter()
{
}


//==============================================================================
bool WildcardFileFilter::isFileSuitable (const File& file) const
{
    const String filename (file.getFileName());

    for (int i = wildcards.size(); --i >= 0;)
        if (filename.matchesWildcard (wildcards[i], true))
            return true;

    return false;
}

bool WildcardFileFilter::isDirectorySuitable (const File&) const
{
    return true;
}


END_JUCE_NAMESPACE
