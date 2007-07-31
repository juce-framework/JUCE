/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

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
