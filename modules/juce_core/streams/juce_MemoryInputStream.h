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

#ifndef __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__
#define __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__

#include "juce_InputStream.h"
#include "../memory/juce_HeapBlock.h"


//==============================================================================
/**
    Allows a block of data and to be accessed as a stream.

    This can either be used to refer to a shared block of memory, or can make its
    own internal copy of the data when the MemoryInputStream is created.
*/
class JUCE_API  MemoryInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a MemoryInputStream.

        @param sourceData               the block of data to use as the stream's source
        @param sourceDataSize           the number of bytes in the source data block
        @param keepInternalCopyOfData   if false, the stream will just keep a pointer to
                                        the source data, so this data shouldn't be changed
                                        for the lifetime of the stream; if this parameter is
                                        true, the stream will make its own copy of the
                                        data and use that.
    */
    MemoryInputStream (const void* sourceData,
                       size_t sourceDataSize,
                       bool keepInternalCopyOfData);

    /** Creates a MemoryInputStream.

        @param data                     a block of data to use as the stream's source
        @param keepInternalCopyOfData   if false, the stream will just keep a reference to
                                        the source data, so this data shouldn't be changed
                                        for the lifetime of the stream; if this parameter is
                                        true, the stream will make its own copy of the
                                        data and use that.
    */
    MemoryInputStream (const MemoryBlock& data,
                       bool keepInternalCopyOfData);

    /** Destructor. */
    ~MemoryInputStream();

    /** Returns a pointer to the source data block from which this stream is reading. */
    const void* getData() const noexcept        { return data; }

    /** Returns the number of bytes of source data in the block from which this stream is reading. */
    size_t getDataSize() const noexcept         { return dataSize; }

    //==============================================================================
    int64 getPosition();
    bool setPosition (int64 pos);
    int64 getTotalLength();
    bool isExhausted();
    int read (void* destBuffer, int maxBytesToRead);

private:
    //==============================================================================
    const void* data;
    size_t dataSize, position;
    HeapBlock<char> internalCopy;

    void createInternalCopy();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryInputStream)
};

#endif   // __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__
