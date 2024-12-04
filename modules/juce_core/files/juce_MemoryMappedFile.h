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
/**
    Maps a file into virtual memory for easy reading and/or writing.

    @tags{Core}
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

        NOTE: The start of the actual range used may be rounded-down to a multiple of the OS's page-size,
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
    void* address = nullptr;
    Range<int64> range;

   #if JUCE_WINDOWS
    void* fileHandle = nullptr;
   #else
    int fileHandle = 0;
   #endif

    void openInternal (const File&, AccessMode, bool);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedFile)
};

} // namespace juce
