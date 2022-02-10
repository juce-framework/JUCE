/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

StringArray DirectoryIterator::parseWildcards (const String& pattern)
{
    StringArray s;
    s.addTokens (pattern, ";,", "\"'");
    s.trim();
    s.removeEmptyStrings();
    return s;
}

bool DirectoryIterator::fileMatches (const StringArray& wildcards, const String& filename)
{
    for (auto& w : wildcards)
        if (filename.matchesWildcard (w, ! File::areFileNamesCaseSensitive()))
            return true;

    return false;
}

bool DirectoryIterator::next()
{
    return next (nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)

bool DirectoryIterator::next (bool* isDirResult, bool* isHiddenResult, int64* fileSize,
                              Time* modTime, Time* creationTime, bool* isReadOnly)
{
    for (;;)
    {
        hasBeenAdvanced = true;

        if (subIterator != nullptr)
        {
            if (subIterator->next (isDirResult, isHiddenResult, fileSize, modTime, creationTime, isReadOnly))
                return true;

            subIterator.reset();
        }

        String filename;
        bool isDirectory, isHidden = false, shouldContinue = false;

        while (fileFinder.next (filename, &isDirectory,
                                (isHiddenResult != nullptr || (whatToLookFor & File::ignoreHiddenFiles) != 0) ? &isHidden : nullptr,
                                fileSize, modTime, creationTime, isReadOnly))
        {
            ++index;

            if (! filename.containsOnly ("."))
            {
                const auto fullPath = File::createFileWithoutCheckingPath (path + filename);
                bool matches = false;

                if (isDirectory)
                {
                    const auto mayRecurseIntoPossibleHiddenDir = [this, &isHidden]
                    {
                        return (whatToLookFor & File::ignoreHiddenFiles) == 0 || ! isHidden;
                    };

                    const auto mayRecurseIntoPossibleSymlink = [this, &fullPath]
                    {
                        return followSymlinks == File::FollowSymlinks::yes
                            || ! fullPath.isSymbolicLink()
                            || (followSymlinks == File::FollowSymlinks::noCycles
                                && knownPaths->find (fullPath.getLinkedTarget()) == knownPaths->end());
                    };

                    if (isRecursive && mayRecurseIntoPossibleHiddenDir() && mayRecurseIntoPossibleSymlink())
                        subIterator.reset (new DirectoryIterator (fullPath, true, wildCard, whatToLookFor, followSymlinks, knownPaths));

                    matches = (whatToLookFor & File::findDirectories) != 0;
                }
                else
                {
                    matches = (whatToLookFor & File::findFiles) != 0;
                }

                // if we're not relying on the OS iterator to do the wildcard match, do it now..
                if (matches && (isRecursive || wildCards.size() > 1))
                    matches = fileMatches (wildCards, filename);

                if (matches && (whatToLookFor & File::ignoreHiddenFiles) != 0)
                    matches = ! isHidden;

                if (matches)
                {
                    currentFile = fullPath;
                    if (isHiddenResult != nullptr)     *isHiddenResult = isHidden;
                    if (isDirResult != nullptr)        *isDirResult = isDirectory;

                    return true;
                }

                if (subIterator != nullptr)
                {
                    shouldContinue = true;
                    break;
                }
            }
        }

        if (! shouldContinue)
            return false;
    }
}

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

const File& DirectoryIterator::getFile() const
{
    if (subIterator != nullptr && subIterator->hasBeenAdvanced)
        return subIterator->getFile();

    // You need to call DirectoryIterator::next() before asking it for the file that it found!
    jassert (hasBeenAdvanced);

    return currentFile;
}

float DirectoryIterator::getEstimatedProgress() const
{
    if (totalNumFiles < 0)
        totalNumFiles = File (path).getNumberOfChildFiles (File::findFilesAndDirectories);

    if (totalNumFiles <= 0)
        return 0.0f;

    auto detailedIndex = (subIterator != nullptr) ? (float) index + subIterator->getEstimatedProgress()
                                                  : (float) index;

    return jlimit (0.0f, 1.0f, detailedIndex / (float) totalNumFiles);
}

} // namespace juce
