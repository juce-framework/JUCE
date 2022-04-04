/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

static StringArray readDeadMansPedalFile (const File& file)
{
    StringArray lines;
    file.readLines (lines);
    lines.removeEmptyStrings();
    return lines;
}

PluginDirectoryScanner::PluginDirectoryScanner (KnownPluginList& listToAddTo,
                                                AudioPluginFormat& formatToLookFor,
                                                FileSearchPath directoriesToSearch,
                                                const bool recursive,
                                                const File& deadMansPedal,
                                                bool allowPluginsWhichRequireAsynchronousInstantiation)
    : list (listToAddTo),
      format (formatToLookFor),
      deadMansPedalFile (deadMansPedal),
      allowAsync (allowPluginsWhichRequireAsynchronousInstantiation)
{
    directoriesToSearch.removeRedundantPaths();
    setFilesOrIdentifiersToScan (format.searchPathsForPlugins (directoriesToSearch, recursive, allowAsync));
}

PluginDirectoryScanner::~PluginDirectoryScanner()
{
    list.scanFinished();
}

//==============================================================================
void PluginDirectoryScanner::setFilesOrIdentifiersToScan (const StringArray& filesOrIdentifiers)
{
    filesOrIdentifiersToScan = filesOrIdentifiers;

    // If any plugins have crashed recently when being loaded, move them to the
    // end of the list to give the others a chance to load correctly..
    for (auto& crashed : readDeadMansPedalFile (deadMansPedalFile))
        for (int j = filesOrIdentifiersToScan.size(); --j >= 0;)
            if (crashed == filesOrIdentifiersToScan[j])
                filesOrIdentifiersToScan.move (j, -1);

    applyBlacklistingsFromDeadMansPedal (list, deadMansPedalFile);
    nextIndex.set (filesOrIdentifiersToScan.size());
}

String PluginDirectoryScanner::getNextPluginFileThatWillBeScanned() const
{
    return format.getNameOfPluginFromIdentifier (filesOrIdentifiersToScan [nextIndex.get() - 1]);
}

void PluginDirectoryScanner::updateProgress()
{
    progress = (1.0f - (float) nextIndex.get() / (float) filesOrIdentifiersToScan.size());
}

bool PluginDirectoryScanner::scanNextFile (bool dontRescanIfAlreadyInList,
                                           String& nameOfPluginBeingScanned)
{
    const int index = --nextIndex;

    if (index >= 0)
    {
        auto file = filesOrIdentifiersToScan [index];

        if (file.isNotEmpty() && ! (dontRescanIfAlreadyInList && list.isListingUpToDate (file, format)))
        {
            nameOfPluginBeingScanned = format.getNameOfPluginFromIdentifier (file);

            OwnedArray<PluginDescription> typesFound;

            // Add this plugin to the end of the dead-man's pedal list in case it crashes...
            auto crashedPlugins = readDeadMansPedalFile (deadMansPedalFile);
            crashedPlugins.removeString (file);
            crashedPlugins.add (file);
            setDeadMansPedalFile (crashedPlugins);

            list.scanAndAddFile (file, dontRescanIfAlreadyInList, typesFound, format);

            // Managed to load without crashing, so remove it from the dead-man's-pedal..
            crashedPlugins.removeString (file);
            setDeadMansPedalFile (crashedPlugins);

            if (typesFound.size() == 0 && ! list.getBlacklistedFiles().contains (file))
                failedFiles.add (file);
        }
    }

    updateProgress();
    return index > 0;
}

bool PluginDirectoryScanner::skipNextFile()
{
    updateProgress();
    return --nextIndex > 0;
}

void PluginDirectoryScanner::setDeadMansPedalFile (const StringArray& newContents)
{
    if (deadMansPedalFile.getFullPathName().isNotEmpty())
        deadMansPedalFile.replaceWithText (newContents.joinIntoString ("\n"), true, true);
}

void PluginDirectoryScanner::applyBlacklistingsFromDeadMansPedal (KnownPluginList& list, const File& file)
{
    // If any plugins have crashed recently when being loaded, move them to the
    // end of the list to give the others a chance to load correctly..
    for (auto& crashedPlugin : readDeadMansPedalFile (file))
        list.addToBlacklist (crashedPlugin);
}

} // namespace juce
