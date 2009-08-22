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

#include "juce_PluginDirectoryScanner.h"
#include "juce_AudioPluginFormat.h"
#include "../../io/files/juce_DirectoryIterator.h"


//==============================================================================
PluginDirectoryScanner::PluginDirectoryScanner (KnownPluginList& listToAddTo,
                                                AudioPluginFormat& formatToLookFor,
                                                FileSearchPath directoriesToSearch,
                                                const bool recursive,
                                                const File& deadMansPedalFile_)
    : list (listToAddTo),
      format (formatToLookFor),
      deadMansPedalFile (deadMansPedalFile_),
      nextIndex (0),
      progress (0)
{
    directoriesToSearch.removeRedundantPaths();

    filesOrIdentifiersToScan = format.searchPathsForPlugins (directoriesToSearch, recursive);

    // If any plugins have crashed recently when being loaded, move them to the
    // end of the list to give the others a chance to load correctly..
    const StringArray crashedPlugins (getDeadMansPedalFile());

    for (int i = 0; i < crashedPlugins.size(); ++i)
    {
        const String f = crashedPlugins[i];

        for (int j = filesOrIdentifiersToScan.size(); --j >= 0;)
            if (f == filesOrIdentifiersToScan[j])
                filesOrIdentifiersToScan.move (j, -1);
    }
}

PluginDirectoryScanner::~PluginDirectoryScanner()
{
}

//==============================================================================
const String PluginDirectoryScanner::getNextPluginFileThatWillBeScanned() const throw()
{
    return format.getNameOfPluginFromIdentifier (filesOrIdentifiersToScan [nextIndex]);
}

bool PluginDirectoryScanner::scanNextFile (const bool dontRescanIfAlreadyInList)
{
    String file (filesOrIdentifiersToScan [nextIndex]);

    if (file.isNotEmpty())
    {
        if (! list.isListingUpToDate (file))
        {
            OwnedArray <PluginDescription> typesFound;

            // Add this plugin to the end of the dead-man's pedal list in case it crashes...
            StringArray crashedPlugins (getDeadMansPedalFile());
            crashedPlugins.removeString (file);
            crashedPlugins.add (file);
            setDeadMansPedalFile (crashedPlugins);

            list.scanAndAddFile (file,
                                 dontRescanIfAlreadyInList,
                                 typesFound,
                                 format);

            // Managed to load without crashing, so remove it from the dead-man's-pedal..
            crashedPlugins.removeString (file);
            setDeadMansPedalFile (crashedPlugins);

            if (typesFound.size() == 0)
                failedFiles.add (file);
        }

        ++nextIndex;
        progress = nextIndex / (float) filesOrIdentifiersToScan.size();
    }

    return nextIndex < filesOrIdentifiersToScan.size();
}

const StringArray PluginDirectoryScanner::getDeadMansPedalFile() throw()
{
    StringArray lines;

    if (deadMansPedalFile != File::nonexistent)
    {
        lines.addLines (deadMansPedalFile.loadFileAsString());
        lines.removeEmptyStrings();
    }

    return lines;
}

void PluginDirectoryScanner::setDeadMansPedalFile (const StringArray& newContents) throw()
{
    if (deadMansPedalFile != File::nonexistent)
        deadMansPedalFile.replaceWithText (newContents.joinIntoString ("\n"), true, true);
}


END_JUCE_NAMESPACE
