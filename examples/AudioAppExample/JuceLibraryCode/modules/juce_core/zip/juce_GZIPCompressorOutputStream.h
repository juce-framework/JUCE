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

#ifndef JUCE_GZIPCOMPRESSOROUTPUTSTREAM_H_INCLUDED
#define JUCE_GZIPCOMPRESSOROUTPUTSTREAM_H_INCLUDED


//==============================================================================
/**
    A stream which uses zlib to compress the data written into it.

    Important note: When you call flush() on a GZIPCompressorOutputStream,
    the gzip data is closed - this means that no more data can be written to
    it, and any subsequent attempts to call write() will cause an assertion.

    @see GZIPDecompressorInputStream
*/
class JUCE_API  GZIPCompressorOutputStream  : public OutputStream
{
public:
    //==============================================================================
    /** Creates a compression stream.

        @param destStream                       the stream into which the compressed data should
                                                be written
        @param compressionLevel                 how much to compress the data, between 1 and 9, where
                                                1 is the fastest/lowest compression, and 9 is the
                                                slowest/highest compression. Any value outside this range
                                                indicates that a default compression level should be used.
        @param deleteDestStreamWhenDestroyed    whether or not to delete the destStream object when
                                                this stream is destroyed
        @param windowBits                       this is used internally to change the window size used
                                                by zlib - leave it as 0 unless you specifically need to set
                                                its value for some reason
    */
    GZIPCompressorOutputStream (OutputStream* destStream,
                                int compressionLevel = 0,
                                bool deleteDestStreamWhenDestroyed = false,
                                int windowBits = 0);

    /** Destructor. */
    ~GZIPCompressorOutputStream();

    //==============================================================================
    /** Flushes and closes the stream.
        Note that unlike most streams, when you call flush() on a GZIPCompressorOutputStream,
        the stream is closed - this means that no more data can be written to it, and any
        subsequent attempts to call write() will cause an assertion.
    */
    void flush();

    int64 getPosition() override;
    bool setPosition (int64) override;
    bool write (const void*, size_t) override;

    /** These are preset values that can be used for the constructor's windowBits parameter.
        For more info about this, see the zlib documentation for its windowBits parameter.
    */
    enum WindowBitsValues
    {
        windowBitsRaw = -15,
        windowBitsGZIP = 15 + 16
    };

private:
    //==============================================================================
    OptionalScopedPointer<OutputStream> destStream;

    class GZIPCompressorHelper;
    friend struct ContainerDeletePolicy<GZIPCompressorHelper>;
    ScopedPointer<GZIPCompressorHelper> helper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GZIPCompressorOutputStream)
};

#endif   // JUCE_GZIPCOMPRESSOROUTPUTSTREAM_H_INCLUDED
