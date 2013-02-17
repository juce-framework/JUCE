/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_MEMORYMAPPEDFILE_JUCEHEADER__
#define __JUCE_MEMORYMAPPEDFILE_JUCEHEADER__

#include "juce_File.h"

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
    */
    MemoryMappedFile (const File& file, AccessMode mode);

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
                      AccessMode mode);

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

    void openInternal (const File&, AccessMode);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedFile)
};


#endif   // __JUCE_MEMORYMAPPEDFILE_JUCEHEADER__
