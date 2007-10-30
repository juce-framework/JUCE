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

#include "../../../../juce.h"
#include "juce_PluginDirectoryScanner.h"
#include "juce_AudioPluginFormat.h"


//==============================================================================
PluginDirectoryScanner::PluginDirectoryScanner (KnownPluginList& listToAddTo,
                                                FileSearchPath directoriesToSearch,
                                                const bool recursive,
                                                const File& deadMansPedalFile_)
    : list (listToAddTo),
      deadMansPedalFile (deadMansPedalFile_),
      nextIndex (0),
      progress (0)
{
    directoriesToSearch.removeRedundantPaths();

    for (int j = 0; j < directoriesToSearch.getNumPaths(); ++j)
        recursiveFileSearch (directoriesToSearch [j], recursive);

    // If any plugins have crashed recently when being loaded, move them to the
    // end of the list to give the others a chance to load correctly..
    const StringArray crashedPlugins (getDeadMansPedalFile());

    for (int i = 0; i < crashedPlugins.size(); ++i)
    {
        const File f (crashedPlugins[i]);

        for (int j = filesToScan.size(); --j >= 0;)
            if (f == *filesToScan.getUnchecked (j))
                filesToScan.move (j, -1);
    }
}

void PluginDirectoryScanner::recursiveFileSearch (const File& dir, const bool recursive)
{
    // avoid allowing the dir iterator to be recursive, because we want to avoid letting it delve inside
    // .component or .vst directories.
    DirectoryIterator iter (dir, false, "*", File::findFilesAndDirectories);

    while (iter.next())
    {
        const File f (iter.getFile());
        bool isPlugin = false;

        for (int i = 0; i < AudioPluginFormatManager::getInstance()->getNumFormats(); ++i)
        {
            AudioPluginFormat* const format = AudioPluginFormatManager::getInstance()->getFormat (i);

            if (format->fileMightContainThisPluginType (f))
            {
                isPlugin = true;
                filesToScan.add (new File (f));
                break;
            }
        }

        if (recursive && (! isPlugin) && f.isDirectory())
            recursiveFileSearch (f, true);
    }
}

PluginDirectoryScanner::~PluginDirectoryScanner()
{
}

//==============================================================================
const File PluginDirectoryScanner::getNextPluginFileThatWillBeScanned() const throw()
{
    File* const file = filesToScan [nextIndex];

    if (file != 0)
        return *file;

    return File::nonexistent;
}

bool PluginDirectoryScanner::scanNextFile (const bool dontRescanIfAlreadyInList)
{
    File* const file = filesToScan [nextIndex];

    if (file != 0)
    {
        if (! list.isListingUpToDate (*file))
        {
            OwnedArray <PluginDescription> typesFound;

            // Add this plugin to the end of the dead-man's pedal list in case it crashes...
            StringArray crashedPlugins (getDeadMansPedalFile());
            crashedPlugins.removeString (file->getFullPathName());
            crashedPlugins.add (file->getFullPathName());
            setDeadMansPedalFile (crashedPlugins);

            list.scanAndAddFile (*file,
                                 dontRescanIfAlreadyInList,
                                 typesFound);

            // Managed to load without crashing, so remove it from the dead-man's-pedal..
            crashedPlugins.removeString (file->getFullPathName());
            setDeadMansPedalFile (crashedPlugins);

            if (typesFound.size() == 0)
                failedFiles.add (file->getFullPathName());
        }

        ++nextIndex;
        progress = nextIndex / (float) filesToScan.size();
    }

    return nextIndex < filesToScan.size();
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
