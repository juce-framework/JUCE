/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Encapsulates the logic for a single-threaded FIFO.

    This might be useful for building buffers which can be written and read in
    blocks of different sizes. For example, in an audio effect we might wish to
    run some processing on fixed-size blocks of audio input, but the host may
    provide input blocks of varying sizes. In this situation, we might want to
    store the previous input in a buffer, and extract a fixed-size block
    whenever there are enough samples available. The SingleThreadedAbstractFifo
    implements logic suitable for this use-case.

    This class is quite similar to AbstractFifo, in that it only keeps track of
    the current read/write locations. The user is responsible for providing the
    actual buffer that will be read/written.

    The intended usage of this class is as follows:
    - Create some backing storage in a vector, AudioBuffer etc.
    - Construct a SingleThreadedAbstractFifo to manage the buffer, passing the
      number of items in the buffer.
    - Each time new input is ready, call write(), passing the number of items
      you wish to write into the buffer. This function returns a pair of ranges
      describing which indices in the backing storage should be written.
    - Call getNumReadable() to find out how many items are ready to read from
      the buffer.
    - If there are enough items ready to read, call read(), passing the number
      of items you require. This function returns a pair of ranges describing
      which indices in the backing storage may be read.

    Unlike AbstractFifo, the SingleThreadedAbstractFifo is intended for use
    from a single thread. It is not safe to call any non-const member function
    of SingleThreadedAbstractFifo concurrently with any other member function.

    @see AbstractFifo

    @tags{Core}
*/
class SingleThreadedAbstractFifo
{
public:
    /** Creates a SingleThreadedAbstractFifo with no size. */
    SingleThreadedAbstractFifo() = default;

    /** Creates a SingleThreadedAbstractFifo that can manage a buffer of the specified size. */
    explicit SingleThreadedAbstractFifo (int sizeIn)
        : size (sizeIn)
    {
        // This class only works properly when the size is a power of two.
        // Use nextPowerOfTwo() to find a good size, and ensure that your
        // backing storage is the same size.
        jassert (isPowerOfTwo (sizeIn));
    }

    /** Returns the number of unused elements present in the buffer. */
    int getRemainingSpace() const   { return size - numReadable; }

    /** Returns the number of pending elements present in the buffer. */
    int getNumReadable() const      { return numReadable; }

    /** Returns the size of the managed buffer. */
    int getSize() const             { return size; }

    /** Returns two blocks in the buffer where new items may be written.

        Note that if the buffer is running low on free space, the sum of the lengths of
        the returned ranges may be less than num!
    */
    std::array<Range<int>, 2> write (int num)
    {
        const auto startPos = (readPos + numReadable) & (size - 1);
        const auto maxToWrite = jmin (getRemainingSpace(), num);
        const auto firstBlockSize = jmin (maxToWrite, size - startPos);

        numReadable += maxToWrite;

        return { { { startPos, startPos + firstBlockSize }, { 0, maxToWrite - firstBlockSize } } };
    }

    /** Returns two blocks in the buffer from which new items may be read.

        Note that if the buffer doesn't have the requested number of items available,
        the sum of the lengths of the returned ranges may be less than num!
    */
    std::array<Range<int>, 2> read (int num)
    {
        const auto startPos = readPos;
        const auto maxToRead = jmin (numReadable, num);
        const auto firstBlockSize = jmin (maxToRead, size - startPos);

        readPos = (startPos + maxToRead) & (size - 1);
        numReadable -= maxToRead;

        return { { { startPos, startPos + firstBlockSize }, { 0, maxToRead - firstBlockSize } } };
    }

private:
    int size = 0, readPos = 0, numReadable = 0;
};


} // namespace juce
