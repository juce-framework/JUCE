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

#ifndef JUCE_BUFFEREDINPUTSTREAM_H_INCLUDED
#define JUCE_BUFFEREDINPUTSTREAM_H_INCLUDED


//==============================================================================
/** Wraps another input stream, and reads from it using an intermediate buffer

    If you're using an input stream such as a file input stream, and making lots of
    small read accesses to it, it's probably sensible to wrap it in one of these,
    so that the source stream gets accessed in larger chunk sizes, meaning less
    work for the underlying stream.
*/
class JUCE_API  BufferedInputStream  : public InputStream
{
public:
    //==============================================================================
    /** Creates a BufferedInputStream from an input source.

        @param sourceStream                 the source stream to read from
        @param bufferSize                   the size of reservoir to use to buffer the source
        @param deleteSourceWhenDestroyed    whether the sourceStream that is passed in should be
                                            deleted by this object when it is itself deleted.
    */
    BufferedInputStream (InputStream* sourceStream,
                         int bufferSize,
                         bool deleteSourceWhenDestroyed);

    /** Creates a BufferedInputStream from an input source.

        @param sourceStream     the source stream to read from - the source stream  must not
                                be deleted until this object has been destroyed.
        @param bufferSize       the size of reservoir to use to buffer the source
    */
    BufferedInputStream (InputStream& sourceStream, int bufferSize);

    /** Destructor.

        This may also delete the source stream, if that option was chosen when the
        buffered stream was created.
    */
    ~BufferedInputStream();


    //==============================================================================
    int64 getTotalLength() override;
    int64 getPosition() override;
    bool setPosition (int64 newPosition) override;
    int read (void* destBuffer, int maxBytesToRead) override;
    String readString() override;
    bool isExhausted() override;


private:
    //==============================================================================
    OptionalScopedPointer<InputStream> source;
    int bufferSize;
    int64 position, lastReadPos, bufferStart, bufferOverlap;
    HeapBlock<char> buffer;
    void ensureBuffered();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BufferedInputStream)
};

#endif   // JUCE_BUFFEREDINPUTSTREAM_H_INCLUDED
