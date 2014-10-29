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

#ifndef JUCE_ABSTRACTFIFO_H_INCLUDED
#define JUCE_ABSTRACTFIFO_H_INCLUDED


//==============================================================================
/**
    Encapsulates the logic required to implement a lock-free FIFO.

    This class handles the logic needed when building a single-reader, single-writer FIFO.

    It doesn't actually hold any data itself, but your FIFO class can use one of these to manage
    its position and status when reading or writing to it.

    To use it, you can call prepareToWrite() to determine the position within your own buffer that
    an incoming block of data should be stored, and prepareToRead() to find out when the next
    outgoing block should be read from.

    e.g.
    @code
    class MyFifo
    {
    public:
        MyFifo()  : abstractFifo (1024)
        {
        }

        void addToFifo (const int* someData, int numItems)
        {
            int start1, size1, start2, size2;
            abstractFifo.prepareToWrite (numItems, start1, size1, start2, size2);

            if (size1 > 0)
                copySomeData (myBuffer + start1, someData, size1);

            if (size2 > 0)
                copySomeData (myBuffer + start2, someData + size1, size2);

            abstractFifo.finishedWrite (size1 + size2);
        }

        void readFromFifo (int* someData, int numItems)
        {
            int start1, size1, start2, size2;
            abstractFifo.prepareToRead (numSamples, start1, size1, start2, size2);

            if (size1 > 0)
                copySomeData (someData, myBuffer + start1, size1);

            if (size2 > 0)
                copySomeData (someData + size1, myBuffer + start2, size2);

            abstractFifo.finishedRead (size1 + size2);
        }

    private:
        AbstractFifo abstractFifo;
        int myBuffer [1024];
    };
    @endcode
*/
class JUCE_API  AbstractFifo
{
public:
    //==============================================================================
    /** Creates a FIFO to manage a buffer with the specified capacity. */
    AbstractFifo (int capacity) noexcept;

    /** Destructor */
    ~AbstractFifo();

    //==============================================================================
    /** Returns the total size of the buffer being managed. */
    int getTotalSize() const noexcept;

    /** Returns the number of items that can currently be added to the buffer without it overflowing. */
    int getFreeSpace() const noexcept;

    /** Returns the number of items that can currently be read from the buffer. */
    int getNumReady() const noexcept;

    /** Clears the buffer positions, so that it appears empty. */
    void reset() noexcept;

    /** Changes the buffer's total size.
        Note that this isn't thread-safe, so don't call it if there's any danger that it
        might overlap with a call to any other method in this class!
    */
    void setTotalSize (int newSize) noexcept;

    //==============================================================================
    /** Returns the location within the buffer at which an incoming block of data should be written.

        Because the section of data that you want to add to the buffer may overlap the end
        and wrap around to the start, two blocks within your buffer are returned, and you
        should copy your data into the first one, with any remaining data spilling over into
        the second.

        If the number of items you ask for is too large to fit within the buffer's free space, then
        blockSize1 + blockSize2 may add up to a lower value than numToWrite. If this happens, you
        may decide to keep waiting and re-trying the method until there's enough space available.

        After calling this method, if you choose to write your data into the blocks returned, you
        must call finishedWrite() to tell the FIFO how much data you actually added.

        e.g.
        @code
        void addToFifo (const int* someData, int numItems)
        {
            int start1, size1, start2, size2;
            prepareToWrite (numItems, start1, size1, start2, size2);

            if (size1 > 0)
                copySomeData (myBuffer + start1, someData, size1);

            if (size2 > 0)
                copySomeData (myBuffer + start2, someData + size1, size2);

            finishedWrite (size1 + size2);
        }
        @endcode

        @param numToWrite       indicates how many items you'd like to add to the buffer
        @param startIndex1      on exit, this will contain the start index in your buffer at which your data should be written
        @param blockSize1       on exit, this indicates how many items can be written to the block starting at startIndex1
        @param startIndex2      on exit, this will contain the start index in your buffer at which any data that didn't fit into
                                the first block should be written
        @param blockSize2       on exit, this indicates how many items can be written to the block starting at startIndex2
        @see finishedWrite
    */
    void prepareToWrite (int numToWrite, int& startIndex1, int& blockSize1, int& startIndex2, int& blockSize2) const noexcept;

    /** Called after writing from the FIFO, to indicate that this many items have been added.
        @see prepareToWrite
    */
    void finishedWrite (int numWritten) noexcept;

    /** Returns the location within the buffer from which the next block of data should be read.

        Because the section of data that you want to read from the buffer may overlap the end
        and wrap around to the start, two blocks within your buffer are returned, and you
        should read from both of them.

        If the number of items you ask for is greater than the amount of data available, then
        blockSize1 + blockSize2 may add up to a lower value than numWanted. If this happens, you
        may decide to keep waiting and re-trying the method until there's enough data available.

        After calling this method, if you choose to read the data, you must call finishedRead() to
        tell the FIFO how much data you have consumed.

        e.g.
        @code
        void readFromFifo (int* someData, int numItems)
        {
            int start1, size1, start2, size2;
            prepareToRead (numSamples, start1, size1, start2, size2);

            if (size1 > 0)
                copySomeData (someData, myBuffer + start1, size1);

            if (size2 > 0)
                copySomeData (someData + size1, myBuffer + start2, size2);

            finishedRead (size1 + size2);
        }
        @endcode

        @param numWanted        indicates how many items you'd like to add to the buffer
        @param startIndex1      on exit, this will contain the start index in your buffer at which your data should be written
        @param blockSize1       on exit, this indicates how many items can be written to the block starting at startIndex1
        @param startIndex2      on exit, this will contain the start index in your buffer at which any data that didn't fit into
                                the first block should be written
        @param blockSize2       on exit, this indicates how many items can be written to the block starting at startIndex2
        @see finishedRead
    */
    void prepareToRead (int numWanted, int& startIndex1, int& blockSize1, int& startIndex2, int& blockSize2) const noexcept;

    /** Called after reading from the FIFO, to indicate that this many items have now been consumed.
        @see prepareToRead
    */
    void finishedRead (int numRead) noexcept;


private:
    //==============================================================================
    int bufferSize;
    Atomic <int> validStart, validEnd;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AbstractFifo)
};


#endif   // JUCE_ABSTRACTFIFO_H_INCLUDED
