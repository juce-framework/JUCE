/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

DirectoryIterator::DirectoryIterator (const File& directory, bool recursive,
                                      const String& pattern, const int type)
  : wildCards (parseWildcards (pattern)),
    fileFinder (directory, (recursive || wildCards.size() > 1) ? "*" : pattern),
    wildCard (pattern),
    path (File::addTrailingSeparator (directory.getFullPathName())),
    index (-1),
    totalNumFiles (-1),
    whatToLookFor (type),
    isRecursive (recursive),
    hasBeenAdvanced (false)
{
    // you have to specify the type of files you're looking for!
    jassert ((type & (File::findFiles | File::findDirectories)) != 0);
    jassert (type > 0 && type <= 7);
}

DirectoryIterator::~DirectoryIterator()
{
}

StringArray DirectoryIterator::parseWildcards (const String& pattern)
{
    StringArray s;
    s.addTokens (pattern, ";,", "\"'");
    s.trim();
    s.removeEmptyStrings();
    return s;
}

bool DirectoryIterator::fileMatches (const StringArray& wildCards, const String& filename)
{
    for (int i = 0; i < wildCards.size(); ++i)
        if (filename.matchesWildcard (wildCards[i], ! File::areFileNamesCaseSensitive()))
            return true;

    return false;
}

bool DirectoryIterator::next()
{
    return next (nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
}

bool DirectoryIterator::next (bool* const isDirResult, bool* const isHiddenResult, int64* const fileSize,
                              Time* const modTime, Time* const creationTime, bool* const isReadOnly)
{
    for (;;)
    {
        hasBeenAdvanced = true;

        if (subIterator != nullptr)
        {
            if (subIterator->next (isDirResult, isHiddenResult, fileSize, modTime, creationTime, isReadOnly))
                return true;

            subIterator = nullptr;
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
                bool matches = false;

                if (isDirectory)
                {
                    if (isRecursive && ((whatToLookFor & File::ignoreHiddenFiles) == 0 || ! isHidden))
                        subIterator = new DirectoryIterator (File::createFileWithoutCheckingPath (path + filename),
                                                             true, wildCard, whatToLookFor);

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
                    currentFile = File::createFileWithoutCheckingPath (path + filename);
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

    const float detailedIndex = (subIterator != nullptr) ? index + subIterator->getEstimatedProgress()
                                                         : (float) index;

    return detailedIndex / totalNumFiles;
}
