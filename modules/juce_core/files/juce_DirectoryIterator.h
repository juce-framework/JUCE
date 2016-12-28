/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_DIRECTORYITERATOR_H_INCLUDED
#define JUCE_DIRECTORYITERATOR_H_INCLUDED


//==============================================================================
/**
    Searches through the files in a directory, returning each file that is found.

    A DirectoryIterator will search through a directory and its subdirectories using
    a wildcard filepattern match.

    If you may be scanning a large number of files, it's usually smarter to use this
    class than File::findChildFiles() because it allows you to stop at any time, rather
    than having to wait for the entire scan to finish before getting the results.

    It also provides an estimate of its progress, using a (highly inaccurate!) algorithm.
*/
class JUCE_API  DirectoryIterator
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

        @param directory    the directory to search in
        @param isRecursive  whether all the subdirectories should also be searched
        @param wildCard     the file pattern to match. This may contain multiple patterns
                            separated by a semi-colon or comma, e.g. "*.jpg;*.png"
        @param whatToLookFor    a value from the File::TypesOfFileToFind enum, specifying
                                whether to look for files, directories, or both.
    */
    DirectoryIterator (const File& directory,
                       bool isRecursive,
                       const String& wildCard = "*",
                       int whatToLookFor = File::findFiles);

    /** Destructor. */
    ~DirectoryIterator();

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
    //==============================================================================
    class NativeIterator
    {
    public:
        NativeIterator (const File& directory, const String& wildCard);
        ~NativeIterator();

        bool next (String& filenameFound,
                   bool* isDirectory, bool* isHidden, int64* fileSize,
                   Time* modTime, Time* creationTime, bool* isReadOnly);

        class Pimpl;

    private:
        friend class DirectoryIterator;
        friend struct ContainerDeletePolicy<Pimpl>;
        ScopedPointer<Pimpl> pimpl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeIterator)
    };

    friend struct ContainerDeletePolicy<NativeIterator::Pimpl>;
    StringArray wildCards;
    NativeIterator fileFinder;
    String wildCard, path;
    int index;
    mutable int totalNumFiles;
    const int whatToLookFor;
    const bool isRecursive;
    bool hasBeenAdvanced;
    ScopedPointer<DirectoryIterator> subIterator;
    File currentFile;

    static StringArray parseWildcards (const String& pattern);
    static bool fileMatches (const StringArray& wildCards, const String& filename);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectoryIterator)
};

#endif   // JUCE_DIRECTORYITERATOR_H_INCLUDED
