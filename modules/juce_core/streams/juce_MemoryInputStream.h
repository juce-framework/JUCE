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

#ifndef JUCE_MEMORYINPUTSTREAM_H_INCLUDED
#define JUCE_MEMORYINPUTSTREAM_H_INCLUDED


//==============================================================================
/**
    Allows a block of data to be accessed as a stream.

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
    int64 getPosition() override;
    bool setPosition (int64 pos) override;
    int64 getTotalLength() override;
    bool isExhausted() override;
    int read (void* destBuffer, int maxBytesToRead) override;

private:
    //==============================================================================
    const void* data;
    size_t dataSize, position;
    HeapBlock<char> internalCopy;

    void createInternalCopy();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryInputStream)
};

#endif   // JUCE_MEMORYINPUTSTREAM_H_INCLUDED
