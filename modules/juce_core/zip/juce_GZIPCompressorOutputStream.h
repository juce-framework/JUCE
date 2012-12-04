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

#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
#define __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__

#include "../streams/juce_OutputStream.h"
#include "../memory/juce_OptionalScopedPointer.h"
#include "../memory/juce_HeapBlock.h"


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

    int64 getPosition();
    bool setPosition (int64 newPosition);
    bool write (const void* destBuffer, int howMany);

    /** These are preset values that can be used for the constructor's windowBits paramter.
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
    friend class ScopedPointer <GZIPCompressorHelper>;
    ScopedPointer <GZIPCompressorHelper> helper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GZIPCompressorOutputStream)
};

#endif   // __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
