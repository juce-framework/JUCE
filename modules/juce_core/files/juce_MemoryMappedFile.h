/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_MEMORYMAPPEDFILE_H_INCLUDED
#define JUCE_MEMORYMAPPEDFILE_H_INCLUDED


//==============================================================================
/**
    Maps a file into virtual memory for easy reading and/or writing.
*/
class JUCE_API  MemoryMappedFile
{
public:
    /** The read/write flags used when opening a memory mapped file. */
    enum AccessMode
    {
        readOnly,   /**< Indicates that the memory can only be read. */
        readWrite   /**< Indicates that the memory can be read and written to - changes that are
                         made will be flushed back to disk at the whim of the OS. */
    };

    /** Opens a file and maps it to an area of virtual memory.

        The file should already exist, and should already be the size that you want to work with
        when you call this. If the file is resized after being opened, the behaviour is undefined.

        If the file exists and the operation succeeds, the getData() and getSize() methods will
        return the location and size of the data that can be read or written. Note that the entire
        file is not read into memory immediately - the OS simply creates a virtual mapping, which
        will lazily pull the data into memory when blocks are accessed.

        If the file can't be opened for some reason, the getData() method will return a null pointer.

        If exclusive is false then other apps can also open the same memory mapped file and use this
        mapping as an effective way of communicating. If exclusive is true then the mapped file will
        be opened exclusively - preventing other apps to access the file which may improve the
        performance of accessing the file.
    */
    MemoryMappedFile (const File& file, AccessMode mode, bool exclusive = false);

    /** Opens a section of a file and maps it to an area of virtual memory.

        The file should already exist, and should already be the size that you want to work with
        when you call this. If the file is resized after being opened, the behaviour is undefined.

        If the file exists and the operation succeeds, the getData() and getSize() methods will
        return the location and size of the data that can be read or written. Note that the entire
        file is not read into memory immediately - the OS simply creates a virtual mapping, which
        will lazily pull the data into memory when blocks are accessed.

        If the file can't be opened for some reason, the getData() method will return a null pointer.

        NOTE: the start of the actual range used may be rounded-down to a multiple of the OS's page-size,
        so do not assume that the mapped memory will begin at exactly the position you requested - always
        use getRange() to check the actual range that is being used.
    */
    MemoryMappedFile (const File& file,
                      const Range<int64>& fileRange,
                      AccessMode mode,
                      bool exclusive = false);

    /** Destructor. */
    ~MemoryMappedFile();

    /** Returns the address at which this file has been mapped, or a null pointer if
        the file couldn't be successfully mapped.
    */
    void* getData() const noexcept              { return address; }

    /** Returns the number of bytes of data that are available for reading or writing.
        This will normally be the size of the file.
    */
    size_t getSize() const noexcept             { return (size_t) range.getLength(); }

    /** Returns the section of the file at which the mapped memory represents. */
    Range<int64> getRange() const noexcept      { return range; }

private:
    //==============================================================================
    void* address;
    Range<int64> range;

   #if JUCE_WINDOWS
    void* fileHandle;
   #else
    int fileHandle;
   #endif

    void openInternal (const File&, AccessMode, bool);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedFile)
};


#endif   // JUCE_MEMORYMAPPEDFILE_H_INCLUDED
