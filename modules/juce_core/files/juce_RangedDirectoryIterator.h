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

//==============================================================================
JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

/**
    Describes the attributes of a file or folder.

    @tags{Core}
*/
class JUCE_API DirectoryEntry final
{
public:
    /** The path to a file or folder. */
    File getFile()              const { return file; }

    /** The time at which the item was last modified. */
    Time getModificationTime()  const { return modTime; }

    /** The time at which the item was created. */
    Time getCreationTime()      const { return creationTime; }

    /** The size of the item. */
    int64 getFileSize()         const { return fileSize; }

    /** True if the item is a directory, false otherwise. */
    bool isDirectory()          const { return directory; }

    /** True if the item is hidden, false otherwise. */
    bool isHidden()             const { return hidden; }

    /** True if the item is read-only, false otherwise. */
    bool isReadOnly()           const { return readOnly; }

    /** The estimated proportion of the range that has been visited
        by the iterator, from 0.0 to 1.0.
    */
    float getEstimatedProgress() const;

private:
    std::weak_ptr<DirectoryIterator> iterator;
    File file;
    Time modTime;
    Time creationTime;
    int64 fileSize  = 0;
    bool directory  = false;
    bool hidden     = false;
    bool readOnly   = false;

    friend class RangedDirectoryIterator;
};

/** A convenience operator so that the expression `*it++` works correctly when
    `it` is an instance of RangedDirectoryIterator.
*/
inline const DirectoryEntry& operator* (const DirectoryEntry& e) noexcept { return e; }

//==============================================================================
/**
    Allows iterating over files and folders using C++11 range-for syntax.

    In the following example, we recursively find all hidden files in a
    specific directory.

    @code
    std::vector<File> hiddenFiles;

    for (DirectoryEntry entry : RangedDirectoryIterator (File ("/path/to/folder"), isRecursive))
        if (entry.isHidden())
            hiddenFiles.push_back (entry.getFile());
    @endcode

    @tags{Core}
*/
class JUCE_API RangedDirectoryIterator final
{
public:
    using difference_type   = std::ptrdiff_t;
    using value_type        = DirectoryEntry;
    using reference         = DirectoryEntry;
    using pointer           = void;
    using iterator_category = std::input_iterator_tag;

    /** The default-constructed iterator acts as the 'end' sentinel. */
    RangedDirectoryIterator() = default;

    /** Creates a RangedDirectoryIterator for a given directory.

        The resulting iterator can be used directly in a 'range-for' expression.

        @param directory        the directory to search in
        @param isRecursive      whether all the subdirectories should also be searched
        @param wildCard         the file pattern to match. This may contain multiple patterns
                                separated by a semi-colon or comma, e.g. "*.jpg;*.png"
        @param whatToLookFor    a value from the File::TypesOfFileToFind enum, specifying
                                whether to look for files, directories, or both.
        @param followSymlinks   the policy to use when symlinks are encountered
    */
    RangedDirectoryIterator (const File& directory,
                             bool isRecursive,
                             const String& wildCard = "*",
                             int whatToLookFor = File::findFiles,
                             File::FollowSymlinks followSymlinks = File::FollowSymlinks::yes);

    /** Returns true if both iterators are in their end/sentinel state,
        otherwise returns false.
    */
    bool operator== (const RangedDirectoryIterator& other) const noexcept
    {
        return iterator == nullptr && other.iterator == nullptr;
    }

    /** Returns the inverse of operator== */
    bool operator!= (const RangedDirectoryIterator& other) const noexcept
    {
        return ! operator== (other);
    }

    /** Return an object containing metadata about the file or folder to
        which the iterator is currently pointing.
    */
    const DirectoryEntry& operator* () const noexcept { return  entry; }
    const DirectoryEntry* operator->() const noexcept { return &entry; }

    /** Moves the iterator along to the next file. */
    RangedDirectoryIterator& operator++()
    {
        increment();
        return *this;
    }

    /** Moves the iterator along to the next file.

        @returns an object containing metadata about the file or folder to
                 to which the iterator was previously pointing.
    */
    DirectoryEntry operator++ (int)
    {
        auto result = *(*this);
        ++(*this);
        return result;
    }

private:
    bool next();
    void increment();

    std::shared_ptr<DirectoryIterator> iterator;
    DirectoryEntry entry;
};

/** Returns the iterator that was passed in.
    Provided for range-for compatibility.
*/
inline RangedDirectoryIterator begin (const RangedDirectoryIterator& it) { return it; }

/** Returns a default-constructed sentinel value.
    Provided for range-for compatibility.
*/
inline RangedDirectoryIterator end   (const RangedDirectoryIterator&)    { return {}; }


JUCE_END_IGNORE_DEPRECATION_WARNINGS

} // namespace juce
