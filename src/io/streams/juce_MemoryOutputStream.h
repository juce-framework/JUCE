/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__
#define __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__

#include "juce_OutputStream.h"
#include "../../containers/juce_MemoryBlock.h"
#include "../../containers/juce_ScopedPointer.h"


//==============================================================================
/** Writes data to an internal memory buffer, which grows as required.

    The data that was written into the stream can then be accessed later as
    a contiguous block of memory.
*/
class JUCE_API  MemoryOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates a memory stream ready for writing into.

        @param initialSize  the intial amount of space to allocate for writing into
        @param granularity  the increments by which the internal storage will be increased
        @param memoryBlockToWriteTo if this is non-zero, then this block will be used as the
                                    place that the data gets stored. If it's zero, the stream
                                    will allocate its own storage internally, which you can
                                    access using getData() and getDataSize()
    */
    MemoryOutputStream (const size_t initialSize = 256,
                        const size_t granularity = 256,
                        MemoryBlock* const memoryBlockToWriteTo = 0) throw();

    /** Destructor.

        This will free any data that was written to it.
    */
    ~MemoryOutputStream() throw();

    //==============================================================================
    /** Returns a pointer to the data that has been written to the stream.

        @see getDataSize
    */
    const char* getData() throw();

    /** Returns the number of bytes of data that have been written to the stream.

        @see getData
    */
    size_t getDataSize() const throw();

    /** Resets the stream, clearing any data that has been written to it so far. */
    void reset() throw();

    //==============================================================================
    void flush();
    bool write (const void* buffer, int howMany);
    int64 getPosition();
    bool setPosition (int64 newPosition);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    MemoryBlock* data;
    ScopedPointer <MemoryBlock> dataToDelete;
    size_t position, size, blockSize;
};

#endif   // __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__
