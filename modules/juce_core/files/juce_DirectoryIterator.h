/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** @cond */
//==============================================================================
/**
    This class is now deprecated in favour of RangedDirectoryIterator.

    Searches through the files in a directory, returning each file that is found.

    A DirectoryIterator will search through a directory and its subdirectories using
    a wildcard filepattern match.

    The iterator keeps track of directories that it has previously traversed, and will
    skip any previously-seen directories in the case of cycles caused by symbolic links.
    It is also possible to avoid following symbolic links altogether.

    If you may be scanning a large number of files, it's usually smarter to use this
    class than File::findChildFiles() because it allows you to stop at any time, rather
    than having to wait for the entire scan to finish before getting the results.

    Please note that the order in which files are returned is completely undefined!
    They'll arrive in whatever order the underlying OS calls provide them, which will
    depend on the filesystem and other factors. If you need a sorted list, you'll need
    to manually sort them using your preferred comparator after collecting the list.

    It also provides an estimate of its progress, using a (highly inaccurate!) algorithm.

    @tags{Core}
    @see RangedDirectoryIterator
*/
class JUCE_API  DirectoryIterator  final
{
public:
    //==============================================================================
    /** Creates a DirectoryIterator for a given directory.

        After creating one of these, call its next() method to get the
        first file - e.g. @code

        DirectoryIterator iter (File ("/animals/mooses"), true, "*.moose");

        while (iter.next())
        {
            File theFileItFound (iter.getFile());

            ... etc
        }
        @endcode

        @see RangedDirectoryIterator
    */
    [[deprecated ("This class is now deprecated in favour of RangedDirectoryIterator.")]]
    DirectoryIterator (const File& directory,
                       bool recursive,
                       const String& pattern = "*",
                       int type = File::findFiles,
                       File::FollowSymlinks follow = File::FollowSymlinks::yes)
        : DirectoryIterator (directory, recursive, pattern, type, follow, nullptr)
    {
    }

    /** Moves the iterator along to the next file.

        @returns    true if a file was found (you can then use getFile() to see what it was) - or
                    false if there are no more matching files.
    */
    bool next();

    /** Moves the iterator along to the next file, and returns various properties of that file.

        If you need to find out details about the file, it's more efficient to call this method than
        to call the normal next() method and then find out the details afterwards.

        All the parameters are optional, so pass null pointers for any items that you're not
        interested in.

        @returns    true if a file was found (you can then use getFile() to see what it was) - or
                    false if there are no more matching files. If it returns false, then none of the
                    parameters will be filled-in.
    */
    bool next (bool* isDirectory,
               bool* isHidden,
               int64* fileSize,
               Time* modTime,
               Time* creationTime,
               bool* isReadOnly);

    /** Returns the file that the iterator is currently pointing at.

        The result of this call is only valid after a call to next() has returned true.
    */
    const File& getFile() const;

    /** Returns a guess of how far through the search the iterator has got.

        @returns    a value 0.0 to 1.0 to show the progress, although this won't be
                    very accurate.
    */
    float getEstimatedProgress() const;

private:
    using KnownPaths = std::set<File>;

    DirectoryIterator (const File& directory,
                       bool recursive,
                       const String& pattern,
                       int type,
                       File::FollowSymlinks follow,
                       KnownPaths* seenPaths)
            : wildCards (parseWildcards (pattern)),
              fileFinder (directory, (recursive || wildCards.size() > 1) ? "*" : pattern),
              wildCard (pattern),
              path (File::addTrailingSeparator (directory.getFullPathName())),
              whatToLookFor (type),
              isRecursive (recursive),
              followSymlinks (follow),
              knownPaths (seenPaths)
    {
        // you have to specify the type of files you're looking for!
        jassert ((whatToLookFor & (File::findFiles | File::findDirectories)) != 0);
        jassert (whatToLookFor > 0 && whatToLookFor <= 7);

        if (followSymlinks == File::FollowSymlinks::noCycles)
        {
            if (knownPaths == nullptr)
            {
                heapKnownPaths = std::make_unique<KnownPaths>();
                knownPaths = heapKnownPaths.get();
            }

            knownPaths->insert (directory);
        }
    }

    //==============================================================================
    struct NativeIterator
    {
        NativeIterator (const File& directory, const String& wildCard);
        ~NativeIterator();

        bool next (String& filenameFound,
                   bool* isDirectory, bool* isHidden, int64* fileSize,
                   Time* modTime, Time* creationTime, bool* isReadOnly);

        class Pimpl;
        std::unique_ptr<Pimpl> pimpl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeIterator)
    };

    StringArray wildCards;
    NativeIterator fileFinder;
    String wildCard, path;
    int index = -1;
    mutable int totalNumFiles = -1;
    const int whatToLookFor;
    const bool isRecursive;
    bool hasBeenAdvanced = false;
    std::unique_ptr<DirectoryIterator> subIterator;
    File currentFile;
    File::FollowSymlinks followSymlinks = File::FollowSymlinks::yes;
    KnownPaths* knownPaths = nullptr;
    std::unique_ptr<KnownPaths> heapKnownPaths;

    static StringArray parseWildcards (const String& pattern);
    static bool fileMatches (const StringArray& wildCards, const String& filename);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectoryIterator)
};
/** @endcond */

} // namespace juce
